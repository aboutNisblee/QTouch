/**
 * \file course.cpp
 *
 * \date 02.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "course.hpp"

#include <QByteArray>
#include <QCryptographicHash>
#include <QDebug>

namespace qtouch
{

CourseLessonBase::CourseLessonBase() :
	mBuiltin(false)
{
}

CourseLessonBase::~CourseLessonBase()
{
}

inline const QUuid& CourseLessonBase::getId() const
{
	return mId;
}

inline bool CourseLessonBase::setId(const QUuid& id)
{
	if (id.isNull())
	{
		mId = QUuid::createUuid();
		return false;
	}
	else
	{
		mId = id;
		return true;
	}
}

inline QString CourseLessonBase::getTitle() const
{
	return mTitle;
}

inline void CourseLessonBase::setTitle(const QString& title)
{
	mTitle = title;
}

inline bool CourseLessonBase::isBuiltin() const
{
	return mBuiltin;
}

inline void CourseLessonBase::setBuiltin(bool builtin)
{
	mBuiltin = builtin;
}

Lesson::~Lesson()
{
}

const QString& Lesson::getNewChars() const
{
	return mNewChars;
}

void Lesson::setNewChars(const QString& newChars)
{
	mNewChars = newChars;
}

const QString& Lesson::getText() const
{
	return mText;
}

void Lesson::setText(const QString& text)
{
	mText = text;
}

/**
 * Get the Course this Lesson belongs to.
 * @note When the Course was deleted or the Lesson never belonged to a Course
 * this function returns a smart pointer to NULL.
 * @return A Course or NULL.
 */
CoursePtr Lesson::getCourse() const
{
	/* Get a strong reference to the parent.
	 * This may fail, when Lessons was never added to a Course. */
	CoursePtr c = mCourse.lock();
	return c;
}

/* Store the back pointer to the owning object. */
inline void Lesson::setCourse(const CoursePtr& parent)
{
	mCourse = parent;
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
CoursePtr Course::create()
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
	 * Here we can use a weak pointer (here called "mParent").
	 * BUT: The all parent pointers stored inside the Lessons must be initialized
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
	CoursePtr p(new Course);
	p->initWeakThis(p);
	return p;
}

CoursePtr Course::clone(const ConstCoursePtr& org)
{
	/* Create a new course by coping the original one.
	 * After the protected constructor made a shallow copy of
	 * the object, we have to replace the lessons. */
	CoursePtr p(new Course(org));
	p->initWeakThis(p);
	p->replace(org->mLessons);
	return p;
}

/* Protected: see create() */
Course::Course()
{
}

Course::Course(const ConstCoursePtr& org) :
	mDescription(org->mDescription)
{
}

Course::~Course()
{
}

inline const QString& Course::getDescription() const
{
	return mDescription;
}

void Course::setDescription(const QString& description)
{
	mDescription = description;
}

/**
 * Replace the Lesson container, drop the current one.
 * @param lessons
 */
void Course::replace(ConstLessonList lessons)
{
	mLessons.clear();
	for (const_iterator it = lessons.begin(); it != lessons.end(); ++it)
	{
		/* Make a "deep" copy of the given lesson
		 * to not alter the parent ptr of the passed one.
		 * Its not really deep because its members are implicitly shared. */
		LessonPtr l(new Lesson(**it));

		CoursePtr thiz(sharedFromWeakThis().staticCast<Course>());
		l->setCourse(thiz);

		mLessons.append(l);
	}
}

/**
 * Append a Lesson.
 * @note Despite the Course takes ownership of the given Lesson,
 * the Lesson is not copied. It can still be manipulated from outside.
 * @param lesson
 */
void Course::append(const LessonPtr& lesson)
{
	CoursePtr thiz(sharedFromWeakThis().staticCast<Course>());
	lesson->setCourse(thiz);

	mLessons.append(lesson);
}

int Course::size() const
{
	return mLessons.size();
}

ConstLessonPtr Course::at(int i) const
{
	return mLessons.at(i);
}

bool Course::contains(const QUuid& id) const
{
	for(const_iterator it = mLessons.begin(); it != mLessons.end(); ++it)
	{
		if((*it)->getId() == id)
			return true;
	}
	return false;
}

ConstLessonPtr Course::get(const QUuid& id) const
{
	for(const_iterator it = mLessons.begin(); it != mLessons.end(); ++it)
	{
		if((*it)->getId() == id)
			return *it;
	}
	return ConstLessonPtr();
}

int Course::indexOf(const LessonPtr& lesson) const
{
	return mLessons.indexOf(lesson);
}

Course::const_iterator Course::begin() const
{
	return mLessons.begin();
}

Course::const_iterator Course::end() const
{
	return mLessons.end();
}

QDataStream& Course::serialize(QDataStream& out) const
{
	out << getId() << getTitle() << getDescription() << isBuiltin();
	for (Course::const_iterator it = begin(); it != end(); ++it)
	{
		out << (*it);
	}
	return out;
}

/**
 * Calculate the MD5 hash of a given Course.
 * @param course A Course.
 * @return The MD5 hash.
 */
QByteArray Course::hash(const ConstCoursePtr& course)
{
	QByteArray buffer;
	QDataStream stream(&buffer, QIODevice::WriteOnly);
	stream << course;
	return QCryptographicHash::hash(buffer, QCryptographicHash::Md5);
}

/**
 * Calculate the MD5 hash of a given list of Courses.
 * @param courses A list of constant Courses.
 * @return The MD5 hash.
 */
QByteArray Course::hash(const ConstCourseList& courses)
{
	QByteArray buffer;
	for (ConstCourseList::const_iterator it = courses.begin(); it != courses.end(); ++it)
	{
		QDataStream stream(&buffer, QIODevice::WriteOnly);
		stream << (*it);
	}
	return QCryptographicHash::hash(buffer, QCryptographicHash::Md5);
}

/**
 * Calculate the MD5 hash of a given list of Courses.
 * @param courses A list of Courses.
 * @return The MD5 hash.
 */
QByteArray Course::hash(const CourseList& courses)
{
	QByteArray buffer;
	for (CourseList::const_iterator it = courses.begin(); it != courses.end(); ++it)
	{
		QDataStream stream(&buffer, QIODevice::WriteOnly);
		stream << (*it);
	}
	return QCryptographicHash::hash(buffer, QCryptographicHash::Md5);
}

}/* namespace qtouch */

