/**
 * \file datamodel.hpp
 *
 * \date 16.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef DATAMODEL_HPP_
#define DATAMODEL_HPP_

#include <map>
#include <QObject>

#include "db/db.hpp"

namespace qtouch
{

class DataModel: public QObject
{
	Q_OBJECT

public:
	explicit DataModel(QObject* parent = 0);
	virtual ~DataModel();

	void init() throw (Exception);

	// Course

	int getCourseCount() const { return mCourses.size(); }
	inline bool isValidCourseIndex(int index) const
	{
		if (index >= 0 && index < static_cast<int>(mCourses.size()))
			return true;
		else
			return false;
	}

	inline QUuid getCourseId(int index) const
	{
		if (!isValidCourseIndex(index))	return QUuid();
		else return mCourses.at(index)->getId();
	}

	inline bool isValidCourse(const QUuid& courseId) const
	{
		if (mCourseMap.count(courseId)) return true;
		else return false;
	}

	QString getCourseTitle(const QUuid& courseId) const;
	QString getCourseDescription(const QUuid& courseId) const;
	bool isCourseBuiltin(const QUuid& courseId) const;

	std::shared_ptr<Course> getCourseCopy(const QUuid& courseId) const;

	// Lesson

	int getLessonCount(const QUuid& courseId) const;
	inline bool isValidLessonIndex(const QUuid& courseId, int index) const
	{
		if (isValidCourse(courseId) && index >= 0 && index < mCourseMap.at(courseId)->size()) return true;
		else return false;
	}

	inline QUuid getLessonId(const QUuid& courseId, int index) const
	{
		if (isValidLessonIndex(courseId, index)) return mCourseMap.at(courseId)->at(index)->getId();
		else return QUuid();
	}

	inline bool isValidLesson(const QUuid& courseId, const QUuid& lessonId) const
	{
		if (isValidCourse(courseId) && mCourseMap.at(courseId)->contains(lessonId))	return true;
		else return false;
	}

	QString getLessonTitle(const QUuid& courseId, const QUuid& lessonId) const;
	QString getLessonNewChars(const QUuid& courseId, const QUuid& lessonId) const;
	bool isLessonBuiltin(const QUuid& courseId, const QUuid& lessonId) const;
	QString getLessonText(const QUuid& courseId, const QUuid& lessonId) const;

private:
	std::unique_ptr<Db> db;

	std::vector<std::shared_ptr<Course>> mCourses;
	std::map<QUuid, std::shared_ptr<Course>> mCourseMap;
};

} /* namespace qtouch */

#endif /* DATAMODEL_HPP_ */
