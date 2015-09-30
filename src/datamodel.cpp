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
 * \file datamodel.cpp
 *
 * \date 16.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "datamodel.hpp"

#include <map>

#include <QDir>
#include <QDebug>

#include "xml/parser.hpp"
#include "db/dbv1.hpp"
#include "db/dbhelper.hpp"

namespace qtouch
{

namespace
{

std::map<QUuid, std::shared_ptr<Course>> make_map(const std::vector<std::shared_ptr<Course>>& list)
{
	std::map<QUuid, std::shared_ptr<Course>> map;
	for (const auto& c : list)
	{
		map.insert(std::make_pair(c->getId(), c));
	}
	return map;
}

} /* namespace anonymous */

DataModel::DataModel(QObject* parent) :
	QObject(parent)
{
}

DataModel::~DataModel()
{
}

/**
 * Initialize the data model.
 * @throw qtouch::Exception
 */
void DataModel::init()
{
	// Get all course files from the resources
	QDir coursepath(QStringLiteral(":/courses"), "*.xml", QDir::Name | QDir::IgnoreCase, QDir::Files);
	QStringList sourceFileList;
	for (const auto& s : coursepath.entryList())
		sourceFileList.append(coursepath.filePath(s));

	// Parse courses
	std::vector<std::shared_ptr<Course>> parsedCourses;

	xml::ParseResult result;
	QString message;

	// Create a validator
	auto validator = xml::createValidator(QStringLiteral(":/courses/course.xsd"));

	// Parse Course files
	QStringListIterator it(sourceFileList);
	while (it.hasNext())
	{
		auto course = xml::parseCourse(it.next(), *validator, &result, &message);
		if (result != xml::Ok)
		{
			qWarning() << "XML parser result:" << result << " " << message.toLatin1().data();
		}
		parsedCourses.push_back(course);
	}

	// Sorting
	std::sort(parsedCourses.begin(), parsedCourses.end(), CourseListAscTitle());

	// Calculate a hash
	auto parsedHash = hash(parsedCourses.begin(), parsedCourses.end());

	// Initialize the database
	/* XXX: Use QStandardPaths::DataLocation when < 5.4
	 * else QStandardPaths::AppDataLocation */

	mDb = DbV1::create();
	mDbHelper = std::unique_ptr<DbHelper>(new DbHelper(mDb, QStringLiteral("QTouch.sqlite")));

	// Check database schema version
	if(mDbHelper->getShemaVersion() != 1)
	{
		// FIXME: Drop and recreate for now
		mDb->dropSchema();
		mDb->createSchema();
	}

	// Read the hash of the build-in courses from the database
	QByteArray dbHash = mDbHelper->getCourseHash();

	// Compare (On match, loading from Db is redundant)
	if (parsedHash != dbHash)
	{
		qDebug() << "Built-in courses in database differ from courses files: Update needed.";

		mDbHelper->updateBuiltinCourses(parsedCourses.begin(), parsedCourses.end());

		if (parsedHash != mDbHelper->getCourseHash())
			qCritical() << "Hash mismatch after database update! Using course files";
	}

	mCourses = parsedCourses;

	// Create the course map
	mCourseMap = make_map(mCourses);

	mDbHelper->getProfiles(std::inserter(mProfiles, mProfiles.begin()));
}

QString DataModel::getCourseTitle(const QUuid& courseId) const
{
	if (isValidCourse(courseId))
		return mCourseMap.at(courseId)->getTitle();
	else
		return QString();
}

QString DataModel::getCourseDescription(const QUuid& courseId) const
{
	if (isValidCourse(courseId))
		return mCourseMap.at(courseId)->getDescription();
	else
		return QString();
}

bool DataModel::isCourseBuiltin(const QUuid& courseId) const
{
	if (isValidCourse(courseId))
		return mCourseMap.at(courseId)->isBuiltin();
	else
		return false;
}

std::shared_ptr<Course> DataModel::getCourseCopy(const QUuid& courseId) const
{
	std::shared_ptr<Course> copy;
	if (isValidCourse(courseId))
	{
		copy = Course::clone(*mCourseMap.at(courseId));
	}

	return copy;
}

int DataModel::getLessonCount(const QUuid& courseId) const
{
	if (isValidCourse(courseId))
		return mCourseMap.at(courseId)->size();
	else
		return 0;
}

QString DataModel::getLessonTitle(const QUuid& courseId, const QUuid& lessonId) const
{
	if (isValidLesson(courseId, lessonId))
		return mCourseMap.at(courseId)->get(lessonId)->getTitle();
	else
		return QString();
}

QString DataModel::getLessonNewChars(const QUuid& courseId, const QUuid& lessonId) const
{
	if (isValidLesson(courseId, lessonId))
		return mCourseMap.at(courseId)->get(lessonId)->getNewChars();
	else
		return QString();
}

bool DataModel::isLessonBuiltin(const QUuid& courseId, const QUuid& lessonId) const
{
	if (isValidLesson(courseId, lessonId))
		return mCourseMap.at(courseId)->get(lessonId)->isBuiltin();
	else
		return false;
}

QString DataModel::getLessonText(const QUuid& courseId, const QUuid& lessonId) const
{
	if (isValidLesson(courseId, lessonId))
		return mCourseMap.at(courseId)->get(lessonId)->getText();
	else
		return QString();
}

bool DataModel::isValidProfile(const QString& name) const
{
	return std::count_if(mProfiles.begin(), mProfiles.end(), [&](const Profile& p){
		return p.getName() == name;
	});
}


} /* namespace qtouch */
