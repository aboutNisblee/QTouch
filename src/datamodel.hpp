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

	inline int getCourseCount() const { return mCourses.size(); }
	bool isValidCourseIndex(int index) const;
	std::shared_ptr<Course> getCourse(int index) const;

	// Lesson

	int getLessonCount(int courseIndex) const;
	bool isValidLessonIndex(int courseIndex, int lessonIndex) const;
	std::shared_ptr<const Lesson> getLesson(int courseIndex, int lessonIndex) const;

	// Profile

	inline int getProfileCount() const { return mProfiles.size(); }
	bool isValidProfileIndex(int index) const;
	bool isValidProfile(const QString& name) const;
	Profile getProfile(int index, bool selectStats = false);
	bool insertProfile(const Profile& profile);

private:
	std::shared_ptr<DbInterface> mDb;
	std::unique_ptr<DbHelper> mDbHelper;

	std::vector<std::shared_ptr<Course>> mCourses;
	std::map<QUuid, std::shared_ptr<Course>> mCourseMap;
	std::vector<Profile> mProfiles;
};

} /* namespace qtouch */

#endif /* DATAMODEL_HPP_ */
