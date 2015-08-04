/**
 * \file dbhelper.hpp
 *
 * \date 31.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef DBHELPER_HPP_
#define DBHELPER_HPP_

#include <memory>
#include <QDebug>

#include "dbinterface.hpp"
#include "utils/utils.hpp"

namespace qtouch
{

class DbHelper
{
public:
	DbHelper(std::shared_ptr<DbInterface> db, const QString& path);
	virtual ~DbHelper();

	inline const QString& getPath() const { return mPath; }
	inline void setPath(const QString& path) { mPath = path; }

	inline std::shared_ptr<DbInterface> getDb() const { return mDb; }
	inline void setDb(const std::shared_ptr<DbInterface>& db) { mDb = db;	}

	int getShemaVersion();
	QByteArray getCourseHash();

	template<typename OutputIter>
	bool getProfiles(OutputIter out);
	template<typename OutputIter>
	bool getStats(const QString& profileName, OutputIter out);
	template<typename OutputIter>
	bool getCourses(Db::CourseType type, OutputIter out, bool includeLessons = false);
	std::shared_ptr<Course> getCourse(const QUuid& courseId, bool includeLessons = false);
	std::unique_ptr<Lesson> getLesson(const QUuid& lessonId);
	bool loadLessons(Course& course);

	template<typename T>
	bool insert(const T& object);
	bool insert(const Course& course);
	template<typename Iter>
	bool insert(Iter first, Iter last);

	template<typename T>
	bool update(const T& object);
	bool update(const Course& c);

	template<typename Iter>
	bool updateBuiltinCourses(Iter first, Iter last);

	bool deleteProfile(const QString& profileName);
	bool deleteStats(const QString& profileName);
	bool deleteCourse(const QUuid& courseId);
	bool deleteLesson(const QUuid& lessonId);

private:
	void insertCourseHelper(const Course& course);
	void updateCourseHelper(const Course& course);

	QString mPath;
	std::shared_ptr<DbInterface> mDb;
};


template<typename OutputIter>
inline bool qtouch::DbHelper::getProfiles(OutputIter out)
{
	try
	{
		if (!mDb->isOpen())
			mDb->open(mPath);

		// pkProfileName, cSkillLevel
		auto query = mDb->selectProfiles();

		while (query.next())
		{
			Profile profile(query.value("pkProfileName").toString(),
			                static_cast<Profile::SkillLevel>(query.value("cSkillLevel").toInt()));
			*out = profile;
			++out;
		}
	}
	catch (const DbException& e)
	{
		qCritical() << e.message();
		return false;
	}

	return true;
}

template<typename OutputIter>
inline bool qtouch::DbHelper::getStats(const QString& profileName, OutputIter out)
{
	try
	{
		if (!mDb->isOpen())
			mDb->open(mPath);

		// pkfkLessonUuid, cStartDateTime, cEndDateTime, cCharCount, cErrorCount
		auto query = mDb->selectStats(profileName);

		while (query.next())
		{
			Stats stats(query.value("pkfkLessonUuid").toUuid(), profileName, query.value("cStartDateTime").toDateTime());
			stats.setEnd(query.value("cEndDateTime").toDateTime());
			stats.setCharCount(query.value("cCharCount").toUInt());
			stats.setErrorCount(query.value("cErrorCount").toUInt());

			*out = stats;
			++out;
		}
	}
	catch (const DbException& e)
	{
		qCritical() << e.message();
		return false;
	}

	return true;
}

template<typename OutputIter>
bool DbHelper::getCourses(Db::CourseType type, OutputIter out, bool includeLessons)
{
	try
	{
		if (!mDb->isOpen())
			mDb->open(mPath);

		// pkCourseUuid, cCourseTitle, cDescription, cCourseBuiltin
		auto query = mDb->selectCourses(type);

		while (query.next())
		{
			auto course = Course::create();
			course->setId(QUuid(query.value("pkCourseUuid").toString()));
			course->setTitle(query.value("cCourseTitle").toString());
			course->setDescription(query.value("cDescription").toString());
			course->setBuiltin(query.value("cCourseBuiltin").toBool());

			if (includeLessons)
			{
				// TODO: What about failures?
				loadLessons(*course);
			}

			*out = course;
			++out;
		}
	}
	catch (const DbException& e)
	{
		qCritical() << e.message();
		return false;
	}

	return true;
}

template<typename T>
bool DbHelper::insert(const T& object)
{
	static_assert(!(std::is_convertible<T, std::shared_ptr<const Course>>::value
	                || std::is_convertible<T, std::unique_ptr<const Course>>::value),
	              "T convertible to smart pointer to Course type. Use \"void insert(const Course& course)\" instead!");

	try
	{
		if (!mDb->isOpen())
			mDb->open(mPath);

		mDb->insert(value(object));
	}
	catch (const DbException& e)
	{
		qCritical() << e.message();
		return false;
	}
	return true;
}

template<typename Iter>
bool DbHelper::insert(Iter first, Iter last)
{
	try
	{
		if (!mDb->isOpen())
			mDb->open(mPath);

		mDb->begin_transaction();
		for (; first != last; ++first)
		{
			insert(value(*first));
		}
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

template<typename T>
bool DbHelper::update(const T& object)
{
	static_assert(!(std::is_convertible<T, std::shared_ptr<const Course>>::value
	                || std::is_convertible<T, std::unique_ptr<const Course>>::value),
	              "T convertible to smart pointer to Course type. Use \"void insert(const Course& course)\" instead!");

	try
	{
		if (!mDb->isOpen())
			mDb->open(mPath);

		mDb->update(value(object));
	}
	catch (const DbException& e)
	{
		qCritical() << e.message();
		return false;
	}
	return true;
}

template<typename Iter>
bool DbHelper::updateBuiltinCourses(Iter first, Iter last)
{
	try
	{
		if (!mDb->isOpen())
			mDb->open(mPath);

		mDb->begin_transaction();

		std::vector<std::shared_ptr<Course>> dbCourses;

		// Read all built-in courses from the database
		if (!getCourses(Db::BuiltIn, std::inserter(dbCourses, dbCourses.begin()), true))
			throw DbException("Unable to read built-in Courses");

		// Sorting
		std::sort(dbCourses.begin(), dbCourses.end(), CourseListAscTitle());

		// Match the Courses from the XML files against the courses in the database

		// Make a working copy
		auto dbCp = dbCourses;

		// Iterate over the source courses
		for (; first != last; ++first)
		{
			bool action = false;

			for (auto dIter = dbCp.begin(); dIter != dbCp.end(); ++dIter)
			{
				// Compare IDs
				if ((*first)->getId() == (*dIter)->getId())
				{
					// if ID are equal -> compare values
					if ((**first) != (**dIter))
					{
						// if unequal -> update of destination needed
						qDebug() << "Updating Course:" << (*first)->getId().toString() << " (" << (*first)->getTitle() << ")";
						updateCourseHelper(**first);
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
				qDebug() << "Inserting new Course:" << (*first)->getId().toString() << " (" << (*first)->getTitle() << ")";
				insertCourseHelper(**first);
			}
		}

		// the entries left in the destination copy must be redundant -> remove
		for (const auto& d : dbCp)
		{
			qDebug() << "Deleting old Course:" << d->getId().toString() << " (" << d->getTitle() << ")";
			if (!deleteCourse(d->getId()))
				throw DbException("Unable to delete Courses");
		}

		// Read updated  courses from the database
		dbCourses.clear();
		if (!getCourses(Db::BuiltIn, std::inserter(dbCourses, dbCourses.begin()), true))
			throw DbException("Unable to read built-in Courses");

		// Sorting
		std::sort(dbCourses.begin(), dbCourses.end(), CourseListAscTitle());

		auto newHash = hash(dbCourses.begin(), dbCourses.end());
		qDebug() << "New Course hash:" << newHash.toHex();

		// If everything went fine, update the hash in the database
		mDb->setMeta(Db::metaCourseHashKey, newHash);

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

} /* namespace qtouch */

#endif /* DBHELPER_HPP_ */
