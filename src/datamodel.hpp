/* Copyright (C) 2015  Moritz Nisblé <moritz.nisble@gmx.de>
 *
 * This file is part of QTouch.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

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

#include "entities/course.hpp"
#include "entities/profile.hpp"
#include "entities/stats.hpp"

namespace qtouch
{

struct DbInterface;
class DbHelper;

class DataModel: public QObject
{
	Q_OBJECT

public:
	explicit DataModel(QObject* parent = 0);
	virtual ~DataModel();

	void init();

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

	// Profile
	int getProfileCount() const { return mProfiles.size(); }
	inline bool isValidProfileIndex(int index) const
	{
		if (index >= 0 && index < static_cast<int>(mProfiles.size()))
			return true;
		else
			return false;
	}
	inline QString getProfileName(int index) const
	{
		if (!isValidProfileIndex(index)) return QString();
		else return mProfiles.at(index).getName();
	}
	bool isValidProfile(const QString& name) const;
	inline Profile::SkillLevel getProfileSkill(int index) const
	{
		if (!isValidProfileIndex(index)) return Profile::Beginner;
		else return mProfiles.at(index).getSkillLevel();
	}

private:
	std::shared_ptr<DbInterface> mDb;
	std::unique_ptr<DbHelper> mDbHelper;

	std::vector<std::shared_ptr<Course>> mCourses;
	std::map<QUuid, std::shared_ptr<Course>> mCourseMap;
	std::vector<Profile> mProfiles;
};

} /* namespace qtouch */

#endif /* DATAMODEL_HPP_ */
