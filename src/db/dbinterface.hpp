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
 * \file dbinterface.hpp
 *
 * \date 31.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef DBINTERFACE_HPP_
#define DBINTERFACE_HPP_

#include <QSqlQuery>

#include "entities/profile.hpp"
#include "entities/stats.hpp"
#include "entities/course.hpp"
#include "dbexception.hpp"

namespace qtouch
{

namespace Db
{
enum CourseType { All, BuiltIn, Custom };

const QString metaSchemaVersionKey = QStringLiteral("SchemaVersion");
const QString metaCourseHashKey = QStringLiteral("BuiltInCourseHash");
} /* namespace Db */

struct DbInterface
{
	/* Connection handling */
	virtual void open(QString const& path) = 0;
	virtual void close() = 0;
	virtual bool isOpen() = 0;

	/* Schema */
	virtual void createSchema() = 0;
	virtual void dropSchema() = 0;

	/* MetaTable */
	virtual void setMeta(const QString& key, const QVariant& value) = 0;
	virtual QVariant getMeta(const QString& key) = 0;

	/* TRANSACTION */
	virtual void begin_transaction() = 0;
	virtual void end_transaction() = 0;
	virtual void rollback() = 0;

	/* INSERT */
	virtual void insert(const Profile& profile) = 0;
	virtual void insert(const Stats& stats) = 0;
	virtual void insert(const Course& course) = 0;
	virtual void insert(const Lesson& lesson) = 0;
	virtual int insert(const QUuid& courseId, const QUuid& lessonId, int parentId = 0) = 0;

	/* UPDATE */
	virtual void update(const Profile& profile) = 0;
	virtual void update(const Stats& stats) = 0;
	virtual void update(const Course& course) = 0;
	virtual void update(const Lesson& lesson) = 0;

	/* SELECT */
	virtual QSqlQuery selectProfiles() = 0;
	virtual QSqlQuery selectStats(const QString& profileName) = 0;
	virtual QSqlQuery selectCourses(Db::CourseType type) = 0;
	virtual QSqlQuery selectCourse(const QUuid& courseId) = 0;
	virtual QSqlQuery selectLesson(const QUuid& lessonId) = 0;
	virtual QSqlQuery selectLessonList(const QUuid& courseId) = 0;
	virtual QSqlQuery selectDanglingLesson() = 0;

	/* DELETE */
	virtual void deleteProfile(const QString& profileName) = 0;
	virtual void deleteStats(const QString& profileName) = 0;
	virtual void deleteCourse(const QUuid& courseId) = 0;
	virtual void deleteLesson(const QUuid& lessonId) = 0;
	virtual void deleteLessonList(const QUuid& courseId) = 0;

	virtual ~DbInterface() {}
};

} /* namespace qtouch */

#endif /* DBINTERFACE_HPP_ */
