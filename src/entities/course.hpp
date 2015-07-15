/**
 * \file course.hpp
 *
 * \date 02.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef COURSE_HPP_
#define COURSE_HPP_

#include <QString>
#include <QUuid>

#include <QList>
#include <QMap>
#include <QDataStream>

#include "utils/sharedthis.hpp"

namespace qtouch
{

class Lesson;
typedef QSharedPointer<Lesson> LessonPtr;
typedef QSharedPointer<const Lesson> ConstLessonPtr;

typedef QList<ConstLessonPtr> LessonList;
typedef QMap<QUuid, ConstLessonPtr> LessonMap;

class Course;
typedef QSharedPointer<Course> CoursePtr;
typedef QSharedPointer<const Course> ConstCoursePtr;

typedef QList<ConstCoursePtr> ConstCourseList;
typedef QList<CoursePtr> CourseList;
typedef QMap<QUuid, ConstCoursePtr> ConstCourseMap;

class CourseLessonBase
{
public:
	virtual ~CourseLessonBase();

	virtual const QUuid& getId() const;
	virtual bool setId(const QUuid& id);

	virtual QString getTitle() const;
	virtual void setTitle(const QString& title);

	virtual bool isBuiltin() const;
	virtual void setBuiltin(bool builtin);

	virtual QDataStream& serialize(QDataStream& out) const = 0;

protected:
	CourseLessonBase();

	QUuid mId;
	QString mTitle;
	bool mBuiltin;
};

class Lesson: public CourseLessonBase
{
	friend class Course;
public:
	virtual ~Lesson();

	const QString& getNewChars() const;
	void setNewChars(const QString& newChars);

	const QString& getText() const;
	void setText(const QString& text);

	CoursePtr getCourse() const;

	virtual QDataStream& serialize(QDataStream& out) const;

private:
	void setCourse(const CoursePtr& parent);

	QWeakPointer<Course> mCourse;

	QString mNewChars;
	QString mText;
};

/**
 * Class that is able to manage a Course and its Lessons.
 * @note Since Lessons are managed in a list of pointers it is possible
 * to manipulate internal data by using the the iterators. Use the clone()
 * method before manipulating lessons.
 */
class Course: public SharedThis, public CourseLessonBase
{
public:
	typedef LessonList::ConstIterator const_iterator;

	static CoursePtr create();
	static CoursePtr clone(const ConstCoursePtr& org);

	virtual ~Course();

	const QString& getDescription() const;
	void setDescription(const QString& description);

	void replace(LessonList lessons);
	void append(const LessonPtr& lesson);

	int size() const;

	ConstLessonPtr at(int i) const;

	bool contains(const QUuid& id) const;
	ConstLessonPtr get(const QUuid& id) const;

	int indexOf(const LessonPtr& lesson) const;

	const_iterator begin() const;
	const_iterator end() const;

	static QByteArray hash(const ConstCoursePtr& course);
	static QByteArray hash(const CourseList& courses);
	static QByteArray hash(const ConstCourseList& courses);

	virtual QDataStream& serialize(QDataStream& out) const;

private:
	Course();
	Course(const ConstCoursePtr& org);

	QString mDescription;
	LessonList mLessons;
	LessonMap mLessonMap;

	Q_DISABLE_COPY(Course)
};

/**
 * Serialize all members.
 * @param out An output stream.
 * @param object A Course or Lesson.
 * @return The output stream.
 */
template<class T>
QDataStream& operator<<(QDataStream& out, const T& object)
{
	return object->serialize(out);
}

struct CourseListAscTitle
{
	bool operator()(const ConstCoursePtr& lhs, const ConstCoursePtr& rhs) { return (lhs->getTitle() < rhs->getTitle()); }
};

} /* namespace qtouch */


#endif /* COURSE_HPP_ */
