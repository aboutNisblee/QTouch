/**
 * \file course.cpp
 *
 * \date 02.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "course.hpp"

#include <QDebug>

namespace qtouch
{

bool CourseLessonBase::setId(const QUuid& id)
{
	if (id.isNull())
	{
		mId = QUuid::createUuid();
		return false;
	}
	else
	{
		if (!mId.isNull())
			qWarning() << this << "Primary key change detected";

		mId = id;
		return true;
	}
}

/**
 * Calculate the MD5 hash.
 * @return The MD5 hash.
 */
QByteArray CourseLessonBase::hash() const
{
	QByteArray buffer;
	QDataStream stream(&buffer, QIODevice::WriteOnly);
	stream << *this;
	return QCryptographicHash::hash(buffer, QCryptographicHash::Md5);
}

/**
 * Get the Course this Lesson belongs to.
 * @note When the Course was deleted or the Lesson never belonged to a Course
 * this function returns a smart pointer to NULL.
 * @return A Course or NULL.
 */
std::shared_ptr<Course> Lesson::getCourse() const
{
	/* Get a strong reference to the parent.
	 * This may fail, when Lessons was never added to a Course. */
	auto c = mCourse.lock();
	return c;
}


QDataStream& Lesson::serialize(QDataStream& out) const
{
	out << getId() << getTitle() << getNewChars() << isBuiltin() << getText();
	return out;
}

/**
 * Create a Course instance managed by a shared pointer.
 * @return A share pointer to the Course instance.
 */
std::shared_ptr<Course> Course::create()
{
	/* What's going on here? ... Let me explain :)
	 * A course manages a list of lessons. Both classes inherit from Resource.
	 * To be able to later ask any Lesson for the Course it corresponds to,
	 * the Lesson must hold a reference back to its Course.
	 * Managing this is a bit tricky because we must ensure two things.
	 * 1. When the course is deleted it is not allowed to delete a Lesson that
	 * was passed out to a client. Here the solution is quite simple:
	 * Use smart pointers to store the Lessons. The Lesson is then deleted but
	 * the Lesson passed out stays valid, as long as the client holds a reference
	 * to it.
	 * 2. After the Course is deleted, the Lesson must not return the reference
	 * to the Course it belonged to, because the memory is already freed.
	 * Here we can use a weak pointer (here called "mCourse").
	 * BUT: The parent pointers stored inside the Lessons must be initialized
	 * by the same shared pointer to the Course, because they must share the same
	 * reference counter.
	 * (Rule: Never store the same address in multiple smart pointers!)
	 * So there must be a single smart pointer to the Course that can be used to
	 * initialize the parent pointers.
	 * (Note: In this case, it would be possible to use the same variable for
	 * both, weak this and weak parent pointer, because only one of them is
	 * used at the same time. But then we would use one variable for two
	 * different purposes.)
	 * This factory method ensures that there is always a shared pointer (and
	 * with a reference count of min. 1) that lives as long as the Course lives.
	 * This pointer is used to initialize the weak pointer in the base class.
	 * And that weak pointer is then used to initialize the parent pointers of
	 * the Lessons.
	 * How tricky ...
	 */
	std::shared_ptr<Course> p(new Course);
	return p;
}

std::shared_ptr<Course> Course::clone(const Course& org)
{
	/* Create a new course by coping the original one.
	 * After the copy constructor made a shallow copy of
	 * the object, we have to replace the lessons. */
	std::shared_ptr<Course> p(new Course(org));
	p->insert(p->begin(), org.mLessons.begin(), org.mLessons.end());
	return p;
}

Course::Course(const Course& org) :
	enable_shared_from_this(), CourseLessonBase(org), mDescription(org.mDescription)
{
}

void Course::push_back(const Lesson& lesson)
{
	auto thiz = shared_from_this();

	auto copy = std::make_shared<Lesson>(lesson);
	copy->setCourse(thiz);

	mLessons.push_back(copy);
}

bool Course::contains(const QUuid& id) const
{
	for (const auto& it : mLessons)
	{
		if (it->getId() == id)
			return true;
	}
	return false;
}

std::shared_ptr<const Lesson> Course::get(const QUuid& lessonId) const
{
	for (const auto& it : mLessons)
	{
		if (it->getId() == lessonId)
			return it;
	}
	return std::shared_ptr<const Lesson>();
}

QDataStream& Course::serialize(QDataStream& out) const
{
	out << getId() << getTitle() << getDescription() << isBuiltin();
	for (const auto& it : mLessons)
	{
		out << (*it);
	}
	return out;
}

}/* namespace qtouch */

