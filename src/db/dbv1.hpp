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
 * \file dbv1.hpp
 *
 * \date 31.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef DBV1_HPP_
#define DBV1_HPP_

#include <memory>
#include "dbinterface.hpp"

class QSqlDatabase;

namespace qtouch
{

class DbV1: public DbInterface
{
public:
	const int VERSION = 1;

	static std::unique_ptr<DbV1> create();
	virtual ~DbV1();

	/* Connection handling */
	void open(QString const& path) Q_DECL_OVERRIDE;
	void close() Q_DECL_OVERRIDE;
	inline bool isOpen() Q_DECL_OVERRIDE { return (db) ? db->isOpen() : false; }

	/* Schema */
	void createSchema() Q_DECL_OVERRIDE;
	void dropSchema() Q_DECL_OVERRIDE;

	/* MetaTable */
	void setMeta(const QString& key, const QVariant& value) Q_DECL_OVERRIDE;
	QVariant getMeta(const QString& key) Q_DECL_OVERRIDE;

	/* TRANSACTION */
	void begin_transaction() Q_DECL_OVERRIDE;
	void end_transaction() Q_DECL_OVERRIDE;
	void rollback() Q_DECL_OVERRIDE;

	/* INSERT */
	void insert(const Profile& profile) Q_DECL_OVERRIDE;
	void insert(const Stats& stats) Q_DECL_OVERRIDE;
	void insert(const Course& course) Q_DECL_OVERRIDE;
	void insert(const Lesson& lesson) Q_DECL_OVERRIDE;
	int insert(const QUuid& courseId, const QUuid& lessonId, int parentId = 0) Q_DECL_OVERRIDE;

	/* UPDATE */
	void update(const Profile& profile) Q_DECL_OVERRIDE;
	void update(const Stats& stats) Q_DECL_OVERRIDE;
	void update(const Course& course) Q_DECL_OVERRIDE;
	void update(const Lesson& lesson) Q_DECL_OVERRIDE;

	/* SELECT */
	QSqlQuery selectProfiles() Q_DECL_OVERRIDE;
	QSqlQuery selectStats(const QString& profileName) Q_DECL_OVERRIDE;
	QSqlQuery selectCourses(Db::CourseType type) Q_DECL_OVERRIDE;
	QSqlQuery selectCourse(const QUuid& courseId) Q_DECL_OVERRIDE;
	QSqlQuery selectLesson(const QUuid& lessonId) Q_DECL_OVERRIDE;
	QSqlQuery selectLessonList(const QUuid& courseId) Q_DECL_OVERRIDE;
	QSqlQuery selectDanglingLesson() Q_DECL_OVERRIDE;

	/* DELETE */
	void deleteProfile(const QString& profileName) Q_DECL_OVERRIDE;
	void deleteStats(const QString& profileName) Q_DECL_OVERRIDE;
	void deleteCourse(const QUuid& courseId) Q_DECL_OVERRIDE;
	void deleteLesson(const QUuid& lessonId) Q_DECL_OVERRIDE;
	void deleteLessonList(const QUuid& courseId) Q_DECL_OVERRIDE;

private:
	DbV1() {}
	Q_DISABLE_COPY(DbV1)

	inline void checkOpen() { if (!isOpen()) throw DbException("Database not open"); }

	std::unique_ptr<QSqlDatabase> db;
};

} /* namespace qtouch */

#endif /* DBV1_HPP_ */
