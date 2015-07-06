/**
 * \file course.cpp
 *
 * \date 02.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "course.hpp"

#include <QDataStream>
#include <QByteArray>
#include <QCryptographicHash>
#include <QDebug>

namespace qtouch
{

Resource::Resource() :
	mBuiltin(false)
{
}

Resource::~Resource()
{
}

inline const QUuid& Resource::getId() const
{
	return mId;
}

inline void Resource::setId(const QUuid& id, bool correction)
{
	if (correction && id.isNull())
	{
		mId = QUuid::createUuid();
		qWarning() << "Invalid Resource UUID";
		qWarning() << "    Title: " << (getTitle().isEmpty() ? "\"Unknown\"" : getTitle());
		qWarning() << "    Generated: " << mId.toString();
	}
	else
	{
		mId = id;
	}
}

inline QString Resource::getTitle() const
{
	return mTitle;
}

inline void Resource::setTitle(const QString& title)
{
	mTitle = title;
}

inline bool Resource::isBuiltin() const
{
	return mBuiltin;
}

inline void Resource::setBuiltin(bool builtin)
{
	mBuiltin = builtin;
}

void Resource::initWeakThis(const ResourcePtr& thiz)
{
	mWeakThis = thiz;
}

ResourcePtr Resource::sharedFromWeakThis()
{
	ResourcePtr strong = mWeakThis.lock();
	return strong;
}

/* Store the back pointer to the owning object. */
inline void Resource::setParent(const ResourcePtr& parent)
{
	mParent = parent;
}

Lesson::~Lesson()
{
}

void Lesson::setId(const QUuid& id, bool correction)
{
	if (correction && id.isNull())
	{
		mId = QUuid::createUuid();
		qWarning() << "Invalid Lesson UUID";
		qWarning() << "    Course: " << (getCourse() ? getCourse()->getTitle() : "Unknown");
		qWarning() << "    Lesson: " << getTitle();
		qWarning() << "    Generated: " << mId.toString();
	}
	else
	{
		mId = id;
	}
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
	ResourcePtr p = mParent.lock();
	CoursePtr c;

	// But when we have a parent the cast cannot fail
	if (p)
		c = p.staticCast<CoursePtr::Type>();

	return c;
}

/**
 * Serialize all members.
 * @param out An output stream.
 * @param lesson A Lesson.
 * @return The output stream.
 */
QDataStream& operator<<(QDataStream& out, const LessonPtr& lesson)
{
	out << lesson->getId() << lesson->getTitle() << lesson->getNewChars() << lesson->isBuiltin() << lesson->getText();
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

CoursePtr Course::clone(const CoursePtr& org)
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

Course::Course(const CoursePtr& org) :
	mDescription(org->mDescription)
{
}

Course::~Course()
{
}

void Course::setId(const QUuid& id, bool correction)
{
	if (correction && id.isNull())
	{
		mId = QUuid::createUuid();
		qWarning() << "Invalid Course UUID";
		qWarning() << "    Course: " << getTitle();
		qWarning() << "    Generated: " << mId.toString();
	}
	else
	{
		mId = id;
	}
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
void Course::replace(const LessonList& lessons)
{
	mLessons.clear();
	for (const_iterator it = lessons.begin(); it != lessons.end(); ++it)
	{
		/* Make a "deep" copy of the given lesson
		 * to not alter the parent ptr of the passed one.
		 * Its not really deep because its members are implicitly shared. */
		LessonPtr l(new Lesson(**it));

		ResourcePtr thiz(sharedFromWeakThis());
		l->setParent(thiz);

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
	ResourcePtr thiz(sharedFromWeakThis());
	lesson->setParent(thiz);

	mLessons.append(lesson);
}

int Course::lessonCount() const
{
	return mLessons.size();
}

Course::iterator Course::begin()
{
	return mLessons.begin();
}

Course::iterator Course::end()
{
	return mLessons.end();
}

Course::const_iterator Course::begin() const
{
	return mLessons.begin();
}

Course::const_iterator Course::end() const
{
	return mLessons.end();
}

/**
 * Calculate the MD5 hash of a given Course.
 * @param course A Course.
 * @return The MD5 hash.
 */
QByteArray Course::hash(const CoursePtr& course)
{
	QByteArray buffer;
	QDataStream stream(&buffer, QIODevice::WriteOnly);
	stream << course;
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

/**
 * Serialize all members.
 * @param out An output stream.
 * @param lesson A Course.
 * @return The output stream.
 */
QDataStream& operator<<(QDataStream& out, const CoursePtr& course)
{
	out << course->getId() << course->getTitle() << course->getDescription() << course->isBuiltin();
	for (Course::const_iterator it = course->begin(); it != course->end(); ++it)
	{
		out << (*it);
	}
	return out;
}

}/* namespace qtouch */

