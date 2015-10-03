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
	if (mDbHelper->getShemaVersion() != 1)
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

	// Read profiles from Db; Stats are loaded on demand
	mDbHelper->getProfiles(std::inserter(mProfiles, mProfiles.begin()));
}

bool DataModel::isValidCourseIndex(int index) const
{
	return (index >= 0 && index < static_cast<int>(mCourses.size())) ? true : false;
}

std::shared_ptr<Course> DataModel::getCourse(int index) const
{
	return isValidCourseIndex(index) ? mCourses.at(index) : std::shared_ptr<Course>();
}

int DataModel::getLessonCount(int courseIndex) const
{
	return isValidCourseIndex(courseIndex) ? mCourses.at(courseIndex)->size() : 0;
}

bool DataModel::isValidLessonIndex(int courseIndex, int lessonIndex) const
{
	return (isValidCourseIndex(courseIndex) && lessonIndex >= 0
	        && lessonIndex < mCourses.at(courseIndex)->size()) ? true : false;
}

std::shared_ptr<const Lesson> DataModel::getLesson(int courseIndex, int lessonIndex) const
{
	return isValidLessonIndex(courseIndex,
	                          lessonIndex) ? mCourses.at(courseIndex)->at(lessonIndex) : std::shared_ptr<const Lesson>();
}

bool DataModel::isValidProfileIndex(int index) const
{
	return (index >= 0 && index < static_cast<int>(mProfiles.size())) ? true : false;
}

bool DataModel::isValidProfile(const QString& name) const
{
	if (name.isEmpty())
		return false;

	return std::count_if(mProfiles.begin(), mProfiles.end(), [&](const Profile & p)
	{
		return p.getName() == name;
	});
}

bool DataModel::insertProfile(const Profile& profile)
{
	bool result = false;
	if (!profile.getName().isEmpty() && !isValidProfile(profile.getName()) && mDbHelper->insert(profile))
	{
		result = true;
		mProfiles.push_back(profile);
	}
	return result;
}

Profile DataModel::getProfile(int index, bool selectStats)
{
	if (isValidProfileIndex(index))
	{
		// Lazy load stats
		if (selectStats)
		{
			mProfiles.at(index).clear();
			mDbHelper->getStats(mProfiles.at(index).getName(),
			                    std::inserter(mProfiles.at(index), mProfiles.at(index).begin()));
		}
		return mProfiles.at(index);
	}
	else
		return Profile(QString());
}

} /* namespace qtouch */
