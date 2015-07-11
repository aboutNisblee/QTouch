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

#include <QtGlobal>

#include "sharedthis.hpp"

namespace qtouch
{

class CourseLessonBase;
typedef QSharedPointer<CourseLessonBase> CourseLessonBasePtr;

class Lesson;
typedef QSharedPointer<Lesson> LessonPtr;
typedef QList<LessonPtr> LessonList;

class Course;
typedef QSharedPointer<Course> CoursePtr;
typedef QList<CoursePtr> CourseList;

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
 */
class Course: public SharedThis, public CourseLessonBase
{
public:
	typedef LessonList::Iterator iterator;
	typedef LessonList::ConstIterator const_iterator;

	static CoursePtr create();
	static CoursePtr clone(const CoursePtr& org);

	virtual ~Course();

	const QString& getDescription() const;
	void setDescription(const QString& description);

	void replace(const LessonList& lessons);
	void append(const LessonPtr& lesson);
	int lessonCount() const;

	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;

	static QByteArray hash(const CoursePtr& course);
	static QByteArray hash(const CourseList& courses);

	virtual QDataStream& serialize(QDataStream& out) const;

private:
	Course();
	Course(const CoursePtr& org);

	QString mDescription;
	LessonList mLessons;

	Q_DISABLE_COPY(Course)
};

QDataStream& operator<<(QDataStream& out, const CoursePtr& course);

struct CourseListAscTitle
{
	bool operator()(const CoursePtr& lhs, const CoursePtr& rhs) { return (lhs->getTitle() < rhs->getTitle()); }
};

} /* namespace qtouch */

#endif /* COURSE_HPP_ */
