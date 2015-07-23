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

void DataModel::init() throw (Exception)
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
	db.reset(new Db(QString("QTouch.sqlite")));

	// Read the hash of the build-in courses from the database
	QByteArray dbHash;
	// XXX: DEBUGGING: Force recreation
//	db->open(QString(), true);
	db->open(QString());

	dbHash = db->getCourseHash();

	// Compare (On match, loading from Db is redundant)
	if (parsedHash != dbHash)
	{
		qWarning() << "Built-in courses in database differ from courses files: Update needed.";

		std::vector<std::shared_ptr<Course>> dbCourses;

		// Read all built-in courses from the database
		dbCourses = db->courses(Db::BuiltIn);

		// Get the lessons
		for (auto& pC : dbCourses)
			db->replaceLessons(*pC);

		// Sorting
		std::sort(dbCourses.begin(), dbCourses.end(), CourseListAscTitle());

		// Match the Courses from the XML files against the courses in the database

		// Make a working copy
		auto dbCp = dbCourses;

		// Iterate over the source courses
		for (auto pIter = parsedCourses.begin(); pIter != parsedCourses.end(); ++pIter)
		{
			bool action = false;

			for (auto dIter = dbCp.begin(); dIter != dbCp.end(); ++dIter)
			{
				// Compare IDs
				if ((*pIter)->getId() == (*dIter)->getId())
				{
					// if ID are equal -> compare values
					if ((**pIter) != (**dIter))
					{
						// if unequal -> update of destination needed
						qDebug() << "Updating Course:" << (*pIter)->getId().toString() << " (" << (*pIter)->getTitle() << ")";
						db->update(**pIter);
					}

					// remove from working copy
					dbCp.erase(dIter);
					action = true;
					break;
				}
			}

			// if course not found in database -> insert
			if (!action)
			{
				qDebug() << "Inserting new Course:" << (*pIter)->getId().toString() << " (" << (*pIter)->getTitle() << ")";
				db->insert(**pIter);
			}
		}

		// the entries left in the destination copy must be redundant -> remove
		for (const auto& d : dbCp)
		{
			qDebug() << "Deleting old Course:" << d->getId().toString() << " (" << d->getTitle() << ")";
			db->deleteCourse(d->getId());
		}

		// Read all built-in courses from the database
		auto written = db->courses(Db::BuiltIn);

		// Get the lessons
		for (auto& pC : written)
			db->replaceLessons(*pC);

		std::sort(written.begin(), written.end(), CourseListAscTitle());

		auto writtenHash = hash(written.begin(), written.end());
		if (writtenHash != parsedHash)
		{
			throw Exception("Hash mismatch after database update");
		}

		// If everything went fine, update the hash in the database
		db->setCourseHash(writtenHash);
	}

	mCourses = parsedCourses;

	// Create the course map
	mCourseMap = make_map(mCourses);
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

} /* namespace qtouch */
