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

#include <QSharedPointer>
#include <QWeakPointer>

namespace qtouch
{

class Resource;
typedef QSharedPointer<Resource> ResourcePtr;

class Lesson;
typedef QSharedPointer<Lesson> LessonPtr;
typedef QList<LessonPtr> LessonList;

class Course;
typedef QSharedPointer<Course> CoursePtr;
typedef QList<CoursePtr> CourseList;

class Resource
{
public:
	virtual ~Resource();

	virtual const QUuid& getId() const;
	virtual void setId(const QUuid& id, bool correction = true);

	virtual QString getTitle() const;
	virtual void setTitle(const QString& title);

	virtual bool isBuiltin() const;
	virtual void setBuiltin(bool builtin);

protected:
	Resource();

	/* XXX: Since 5.4 there is a base class for this purpose
	 * called QEnableSharedFromThis. */
	QWeakPointer<Resource> mWeakThis;

	QWeakPointer<Resource> mParent;

	QUuid mId;
	QString mTitle;
	bool mBuiltin;

	void initWeakThis(const ResourcePtr& thiz);
	ResourcePtr sharedFromWeakThis();

	void setParent(const ResourcePtr& parent);
};

class Lesson: public Resource
{
	/* For Course to be able to access protected setParent() in Resource. */
	friend class Course;

public:
	virtual ~Lesson();

	virtual void setId(const QUuid& id, bool correction = true);

	const QString& getNewChars() const;
	void setNewChars(const QString& newChars);

	const QString& getText() const;
	void setText(const QString& text);

	CoursePtr getCourse() const;

private:
	QString mNewChars;
	QString mText;
};

QDataStream& operator<<(QDataStream& out, const LessonPtr& lesson);

/**
 * Class that is able to manage a Course and its Lessons.
 */
class Course: public Resource
{
public:
	typedef LessonList::Iterator iterator;
	typedef LessonList::ConstIterator const_iterator;

	static CoursePtr create();
	static CoursePtr clone(const CoursePtr& org);

	virtual ~Course();

	virtual void setId(const QUuid& id, bool correction = true);

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

protected:
	Course();
	Course(const CoursePtr& org);

private:
	QString mDescription;

	LessonList mLessons;

	Q_DISABLE_COPY(Course)
};

QDataStream& operator<<(QDataStream& out, const CoursePtr& course);

} /* namespace qtouch */

#endif /* COURSE_HPP_ */
