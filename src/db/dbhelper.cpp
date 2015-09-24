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
 * \file dbhelper.cpp
 *
 * \date 31.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "dbhelper.hpp"

#include <sqlite3.h>

namespace qtouch
{

DbHelper::DbHelper(std::shared_ptr<DbInterface> db, const QString& path) :
	mPath(path), mDb(db)
{
}

DbHelper::~DbHelper()
{
	if (mDb->isOpen())
		mDb->close();
}

/**
 * Returns the schema version.
 * @return The schema version of the Db, -1 on error.
 */
int DbHelper::getShemaVersion()
{
	int version = -1;
	try
	{
		if (!mDb->isOpen())
			mDb->open(mPath);

		version = mDb->getMeta(Db::metaSchemaVersionKey).toInt();
	}
	catch (const DbException& e)
	{
		if (QSqlError::StatementError != e.sqlError().type())
			qCritical() << e.message();
		else
			qDebug() << "Unable to fetch schema version. No database present?";
	}
	return version;
}

QByteArray DbHelper::getCourseHash()
{
	QByteArray hash;
	try
	{
		if (!mDb->isOpen())
			mDb->open(mPath);

		hash = mDb->getMeta(Db::metaCourseHashKey).toByteArray();
	}
	catch (const DbException& e)
	{
		qCritical() << e.message();
	}
	return hash;
}

/**
 * Load a specific Course from the Database.
 * @param courseId A CourseId.
 * @param includeLessons When true, the Lessons are also loaded.
 * @return
 */
std::shared_ptr<Course> DbHelper::getCourse(const QUuid& courseId, bool includeLessons)
{
	std::shared_ptr<Course> course;

	try
	{
		if (!mDb->isOpen())
			mDb->open(mPath);

		// pkCourseUuid, cCourseTitle, cDescription, cCourseBuiltin
		auto query = mDb->selectCourse(courseId);
		if (query.next())
		{
			course = Course::create();
			course->setId(QUuid(query.value("pkCourseUuid").toString()));
			course->setTitle(query.value("cCourseTitle").toString());
			course->setDescription(query.value("cDescription").toString());
			course->setBuiltin(query.value("cCourseBuiltin").toBool());

			if (includeLessons)
			{
				if (!loadLessons(*course))
					course.reset();
			}
		}
	}
	catch (const DbException& e)
	{
		qCritical() << e.message();
		course.reset();
	}
	return course;
}

/**
 * Load a specific Lesson from the database.
 * @param lessonId A LessonId.
 * @return On success a smart pointer to the Lesson else an empty smart pointer.
 */
std::unique_ptr<Lesson> DbHelper::getLesson(const QUuid& lessonId)
{
	std::unique_ptr<Lesson> lesson;
	try
	{
		// pkLessonUuid, cLessonTitle, cNewChars, cLessonBuiltin, cText
		auto query = mDb->selectLesson(lessonId);
		if (query.next())
		{
			lesson.reset(new Lesson());
			lesson->setId(QUuid(query.value("pkLessonUuid").toString()));
			lesson->setTitle(query.value("cLessonTitle").toString());
			lesson->setNewChars(query.value("cNewChars").toString());
			lesson->setBuiltin(query.value("cLessonBuiltin").toBool());
			lesson->setText(query.value("cText").toString());
		}
	}
	catch (const DbException& e)
	{
		qCritical() << e.message();
		lesson.reset();
	}
	return lesson;
}

/**
 * Load the Lessons of the given Course and replace the LessonList
 * of the Course with the loaded one.
 * @param course A Course.
 * @return true on success else false.
 */
bool DbHelper::loadLessons(Course& course)
{
	course.clear();

	try
	{
		// pkLessonUuid, cLessonTitle, cNewChars, cLessonBuiltin, cText
		auto query = mDb->selectLessonList(course.getId());
		while (query.next())
		{
			Lesson lesson;
			lesson.setId(QUuid(query.value("pkLessonUuid").toString()));
			lesson.setTitle(query.value("cLessonTitle").toString());
			lesson.setNewChars(query.value("cNewChars").toString());
			lesson.setBuiltin(query.value("cLessonBuiltin").toBool());
			lesson.setText(query.value("cText").toString());

			course.push_back(lesson);
		}
	}
	catch (const DbException& e)
	{
		qCritical() << e.message();
		return false;
	}
	return true;
}

bool DbHelper::insert(const Course& course)
{
	try
	{
		if (!mDb->isOpen())
			mDb->open(mPath);

		mDb->begin_transaction();

		insertCourseHelper(course);

		mDb->end_transaction();
	}
	catch (const DbException& e)
	{
		mDb->rollback();
		qCritical() << e.message();
		return false;
	}
	return true;
}

bool DbHelper::update(const Course& c)
{
	try
	{
		// Try to find the course
		auto cEntry = getCourse(c.getId(), true);
		if (!cEntry)
			return false;

		// Check if update is needed
		if (*cEntry == c)
		{
			qDebug() << "No update needed";
			return true;
		}

		mDb->begin_transaction();

		updateCourseHelper(c);

		mDb->end_transaction();
	}
	catch (const DbException& e)
	{
		mDb->rollback();
		qCritical() << e.message();
		return false;
	}

	return true;
}

bool DbHelper::deleteProfile(const QString& profileName)
{
	try
	{
		if (!mDb->isOpen())
			mDb->open(mPath);
		mDb->deleteProfile(profileName);
	}
	catch (const DbException& e)
	{
		qCritical() << e.message();
		return false;
	}
	return true;
}

bool DbHelper::deleteStats(const QString& profileName)
{
	try
	{
		if (!mDb->isOpen())
			mDb->open(mPath);
		mDb->deleteStats(profileName);
	}
	catch (const DbException& e)
	{
		qCritical() << e.message();
		return false;
	}
	return true;
}

bool DbHelper::deleteCourse(const QUuid& courseId)
{
	try
	{
		if (!mDb->isOpen())
			mDb->open(mPath);
		mDb->deleteCourse(courseId);
	}
	catch (const DbException& e)
	{
		qCritical() << e.message();
		return false;
	}
	return true;
}

bool DbHelper::deleteLesson(const QUuid& lessonId)
{
	try
	{
		if (!mDb->isOpen())
			mDb->open(mPath);
		mDb->deleteLesson(lessonId);
	}
	catch (const DbException& e)
	{
		qCritical() << e.message();
		return false;
	}
	return true;
}

/* Private Course manipulation helper.
 * This functions should be called from inside transactions. */
void DbHelper::insertCourseHelper(const Course& course)
{
	// Insert the Course
	mDb->insert(course);

	int parentId = 0;
	for (const auto& lesson : course)
	{
		try
		{
			// Insert its Lessons
			mDb->insert(*lesson);
		}
		catch (const DbException& e)
		{
			if (SQLITE_CONSTRAINT == e.sqlErrorCode())
				qDebug() << "INFO: Lesson already present:" << e.databaseMessage();
			else
				throw;
		}

		// Create LessonList entry
		parentId = mDb->insert(course.getId(), lesson->getId(), parentId);
	}
}

void DbHelper::updateCourseHelper(const Course& course)
{
	// Update course
	mDb->update(course);

	// Delete current LessonList
	mDb->deleteLessonList(course.getId());

	int parentId = -1;
	for (const auto& lesson : course)
	{
		// Try to find the lesson
		auto lEntry = getLesson(lesson->getId());
		if (lEntry)
			mDb->update(*lesson);
		else
			mDb->insert(*lesson);

		// Create list entries
		if (*course.begin() == lesson)
		{
			// Store the rowId of the last insert on the LessonList for next round
			parentId = mDb->insert(course.getId(), lesson->getId());
		}
		else
		{
			parentId = mDb->insert(course.getId(), lesson->getId(), parentId);
		}
	}
}

} /* namespace qtouch */
