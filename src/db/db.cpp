/**
 * \file db.cpp
 *
 * \date 06.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "db.hpp"

#ifdef QTOUCH_CONF_DB_TARGET_VERSION
#define QTOUCH_DB_TARGET_VERSION QTOUCH_CONF_DB_TARGET_VERSION
#else
#define QTOUCH_DB_TARGET_VERSION 1
#endif

#include <QFileInfo>
#include <QDir>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

//#include <QtSql/QSqlDriver>
//#include <QtSql/5.4.2/QtSql/private/qsql_sqlite_p.h>
#include <sqlite3.h>

#include <QDebug>

#include "db_help.hpp"

namespace qtouch
{

using namespace db::internal;

namespace
{

void insertCourseHelper(QSqlDatabase& db, const Course& course)
{
	insertCourse(db, course);

	int parentId = -1;
	for (const auto& it : course)
	{
		/* Do not abort when the Lesson insert fails due to PK constraint failed!
		 * This could be a custom Course that contains built-in Lessons.
		 * Try insert and catch the exception! */
		try
		{
			insertLesson(db, *it);
		}
		catch (const DbException& e)
		{
			if (SQLITE_CONSTRAINT == e.sqlErrorCode())
				qDebug() << "INFO: Lesson already present:" << e.databaseMessage();
			else
				throw;
		}

		/* Create list head or node and
		 * store the rowId of the last insert on the LessonList for next round */
		if (*course.begin() == it)
			parentId = insertLessonListHead(db, *it);
		else
			parentId = insertLessonListNode(db, *it, parentId);
	}
}

/* Get the LessonListId for the given Lesson Course combination
 * Note: The database ensures that there can only be one entry! */
int lessonListId(QSqlDatabase& db, const QUuid& courseId, const QUuid& lessonId)
{
	QSqlQuery q(db);
	q.setForwardOnly(true);

	q.prepare(
	    QStringLiteral("SELECT pkLessonListId FROM vLessons WHERE pkCourseUuid = :course_id AND pkLessonUuid = :lesson_id"));
	q.bindValue(":course_id", courseId);
	q.bindValue(":lesson_id", lessonId);

	exec_query(q);

	// NOTE: Remember that size() does not work with SQLite !!
	//	if (q.size() != 1)
	if (!q.next())
		throw DbException(QStringLiteral("Unable to find LessonListId for Course: ") % courseId.toString() %
		                  " Lesson: " % lessonId.toString(), q.lastError());

	return q.value(0).toInt();
}

} /* namespace */

/**
 * Constructor.
 * A call to open is optional. The connection is opened when needed,
 * using the given path.
 * @param path Path to the database file.
 */
Db::Db(const QString& path) :
	db(new QSqlDatabase(QSqlDatabase::addDatabase(QStringLiteral("QSQLITE")))), mPath(path)
{
}

/**
 * Destructor.
 * Closes the connection to the database and free the resources.
 */
Db::~Db()
{
	close();
}

/**
 * Explicitly open a Db file. If path is empty the path passed to constructor is used.
 * @param path Path to the database file.
 * @param force_creation If true, the old database is dropped.
 * @return true on success.
 */
void Db::open(const QString& path, bool force_creation) throw (DbException)
{
	if (force_creation)
		close();

	if (!db)
		db.reset(new QSqlDatabase(QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"))));

	// Shortcut
	if (db->isOpen())
		return;

	// Check for valid driver
	if (!db->isValid())
		throw DbException(QStringLiteral("Cannot add database driver"), db->lastError());

	// If the given path is empty, take the stored one
	QString tmp = path.isEmpty() ? mPath : path;

	// If that was also empty, throw an exception
	if (tmp.isEmpty())
		throw DbException(QStringLiteral("No database path passed"), db->lastError());

	// Check existence of the directory to the database file
	if (!QDir(QFileInfo(tmp).absoluteDir()).exists())
	{
		throw
		//		DbException(QStringLiteral("Invalid path to database file: ") % tmp, db->lastError());
		DbException(QStringLiteral("Invalid path to database file: ") % tmp);
	}

	// Else open or create the database file
	db->setDatabaseName(tmp);
	if (!db->open())
	{
		throw DbException(QStringLiteral("Unable to open database at ") % path, db->lastError());
	}
	else
	{
		qDebug() << "Opened database at" << QFileInfo(tmp).absolutePath();

		// Store the path for further actions
		mPath = tmp;
	}

	int version = getUserVersion();
	qDebug() << "Found database version" << version;

	/*
	 * If the user version equals target version,
	 * -> return
	 * If user version is 0, we have just created a new database
	 * -> Simply fill in the tables.
	 * If the user version is smaller than the target version,
	 * -> make a backup and later implement altering of tables.
	 * If user version is bigger than the target version (downgrade),
	 * -> make a backup and create a new database (inform the user!)
	 */

	if (0 == version || force_creation)
	{
		if (force_creation)
			qDebug() << "Forcing recreation of database";

		dropTables(*db);

		configure(*db);

		createScheme(*db);
	}
	else if (version == QTOUCH_DB_TARGET_VERSION)
	{
	}
	else if (version < QTOUCH_DB_TARGET_VERSION)
	{
		throw DbException(QStringLiteral("Upgrade of database not supported yet! Call your local developer ..."));
	}
	else
	{
		/* TODO: At least in this case, we must ask the user!
		 * So use a delegate or an enum as return value and an make the create frunction accessable! */
		throw DbException(
		    QStringLiteral("Seems that you downgraded the application. Backup of old database was made ..."));
	}
}

/**
 * Explicitly close the current database.
 * A call to this function is normally not needed. It is called from destructor.
 */
void Db::close()
{
	if (db)
	{
		if (db->isOpen())
			db->close();

		// Note: You have to release the Db before you can remove the database connection
		db.reset();

		QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);
	}
}

/**
 * Returns the configured user version.
 * @return The user version of the Db, 0 if Db just created or -1 on error.
 */
int Db::getUserVersion() throw (DbException)
{
	open();

	int version = -1;
	QSqlQuery q(*db);
	q.setForwardOnly(true);

	exec_query_string(q, QStringLiteral("PRAGMA user_version"));

	if (q.next())
		version = q.value(0).toInt();

	return version;
}

void Db::setCourseHash(const QByteArray& hash) throw (DbException)
{
	open();

	updateMeta(*db, meta_courseHash, hash);
}

QByteArray Db::getCourseHash() throw (DbException)
{
	open();

	return getMeta(*db, meta_courseHash).toByteArray();
}

/**
 * Get all Profiles.
 * @return A list with all Profiles.
 */
std::vector<std::shared_ptr<const Profile>> Db::profiles() throw (DbException)
{
	open();

	std::vector<std::shared_ptr<const Profile>> result;

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	exec_query_string(q, QStringLiteral("SELECT pkProfileName,cSkillLevel FROM tblProfile"));

	while (q.next())
	{
		auto profile = std::make_shared<Profile>(q.value(0).toString(), (Profile::SkillLevel) q.value(1).toInt());
		result.push_back(profile);
	}

	return result;
}

std::vector<std::shared_ptr<const Stats>> Db::stats(const QString& profileName) throw (DbException)
{
	open();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(
	    QStringLiteral("SELECT pkfkLessonUuid,cStartDateTime,cEndDateTime,cCharCount,cErrorCount FROM tblStats WHERE pkfkProfileName = :profileName"));
	q.bindValue(":profileName", profileName);

	exec_query(q);

	std::vector<std::shared_ptr<const Stats>> result;
	while (q.next())
	{
		auto stats = std::make_shared<Stats>(q.value("pkfkLessonUuid").toUuid(), profileName,
		                                     q.value("cStartDateTime").toDateTime());
		stats->setEnd(q.value("cEndDateTime").toDateTime());
		stats->setCharCount(q.value("cCharCount").toUInt());
		stats->setErrorCount(q.value("cErrorCount").toUInt());

		result.push_back(stats);
	}
	return result;
}

void Db::replaceStats(Profile& profile) throw (DbException)
{
	auto statsList = stats(profile.getName());
	profile.replace(statsList);
}

/**
 * Get a specific lesson.
 * @param lessonId
 * @return
 */
std::unique_ptr<Lesson> Db::lesson(const QUuid& lessonId) throw (DbException)
{
	open();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(
	    QStringLiteral("SELECT pkLessonUuid,cLessonTitle,cNewChars,cLessonBuiltin,cText FROM tblLesson WHERE pkLessonUuid = :lesson_id"));
	q.bindValue(":lesson_id", lessonId);

	exec_query(q);

	if (!q.next())
	{
		return std::unique_ptr<Lesson>();
	}

	std::unique_ptr<Lesson> result(new Lesson());
	result->setId(QUuid(q.value(0).toString()));
	result->setTitle(q.value(1).toString());
	result->setNewChars(q.value(2).toString());
	result->setBuiltin(q.value(3).toBool());
	result->setText(q.value(3).toString());

	return result;
}

/**
 * Get all Lessons for a given CourseId.
 * @param course
 * @return
 */
std::vector<std::shared_ptr<Lesson>> Db::lessons(const QUuid& courseId) throw (DbException)
{
	open();

	std::vector<std::shared_ptr<Lesson>> result;

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(
	    QStringLiteral("SELECT pkLessonUuid,cLessonTitle,cNewChars,cLessonBuiltin,cText FROM vLessons WHERE pkCourseUuid = :course_id"));
	q.bindValue(":course_id", courseId);

	exec_query(q);

	while (q.next())
	{
		auto lesson = std::make_shared<Lesson>();
		lesson->setId(QUuid(q.value(0).toString()));
		lesson->setTitle(q.value(1).toString());
		lesson->setNewChars(q.value(2).toString());
		lesson->setBuiltin(q.value(3).toBool());
		lesson->setText(q.value(4).toString());

		result.push_back(lesson);
	}

	return result;
}

std::shared_ptr<Course> Db::course(const QUuid& courseId) throw (DbException)
{
	open();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(
	    QStringLiteral("SELECT pkCourseUuid,cCourseTitle,cDescription,cCourseBuiltin FROM tblCourse WHERE pkCourseUuid = :course_id"));
	q.bindValue(":course_id", courseId);

	exec_query(q);

	if (!q.next())
	{
		return std::shared_ptr<Course>();
	}

	std::shared_ptr<Course> result = Course::create();

	result->setId(QUuid(q.value(0).toString()));
	result->setTitle(q.value(1).toString());
	result->setDescription(q.value(2).toString());
	result->setBuiltin(q.value(3).toBool());

	return result;
}

/**
 * Get all Courses.
 * @note Only the courses are selected NOT the lessons. Use lessons()!
 * @param type Select a category.
 * @return A list of Courses.
 */
std::vector<std::shared_ptr<Course>> Db::courses(CourseType type) throw (DbException)
{
	open();

	std::vector<std::shared_ptr<Course>> result;

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	QString stmt = QStringLiteral("SELECT pkCourseUuid,cCourseTitle,cDescription,cCourseBuiltin FROM tblCourse");
	if (All != type)
		stmt.append(" WHERE cCourseBuiltin = :builtin");

	q.prepare(stmt);

	if (All != type)
		q.bindValue(":builtin", (BuiltIn == type ? "1" : "0"));

	exec_query(q);

	//	qDebug() << lastQuery(q);

	while (q.next())
	{
		auto out = Course::create();
		out->setId(QUuid(q.value(0).toString()));
		out->setTitle(q.value(1).toString());
		out->setDescription(q.value(2).toString());
		out->setBuiltin(q.value(3).toBool());

		result.push_back(out);
	}

	return result;
}

/**
 * Get all lessons for the given course and replace its lesson list with the result.
 * (This method is only for convenience)
 * @param course
 */
void Db::replaceLessons(Course& course) throw (DbException)
{
	auto lessonList = lessons(course.getId());
	course.clear();
	course.insert(course.begin(), lessonList.begin(), lessonList.end());
}

/**
 * Insert a profile.
 * @param profile
 * @throws DbException
 */
void Db::insert(const Profile& profile) throw (DbException)
{
	open();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(QStringLiteral("INSERT INTO tblProfile VALUES (:name, :skill)"));
	q.bindValue(":name", profile.getName());
	q.bindValue(":skill", profile.getSkillLevel());

	exec_query(q);
}

/**
 * Insert a course.
 * @param course
 * @throws DbException
 */
void Db::insert(const Course& course) throw (DbException)
{
	open();

	begin_transaction(*db);

	try
	{
		insertCourseHelper(*db, course);
		end_transaction(*db);
	}
	catch (...)
	{
		qWarning() << "Transaction failed -> rollback";
		db->rollback();
		throw;
	}
}

void Db::insert(const Stats& stats) throw (DbException)
{
	open();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(QStringLiteral("INSERT INTO tblStats VALUES (:lesson, :profile, :start, :end, :chars, :errors)"));
	q.bindValue(":lesson", stats.getLessonId());
	q.bindValue(":profile", stats.getProfileName());
	q.bindValue(":start", stats.getStart());
	q.bindValue(":end", stats.getEnd());
	q.bindValue(":chars", stats.getCharCount());
	q.bindValue(":errors", stats.getErrorCount());

	exec_query(q);
}

/**
 * Insert a list of courses.
 * @note Courses are inserted in one transaction. That means that when only
 * one Course cannot be inserted the whole transaction is aborted an no Course is
 * inserted.
 * @param courses
 * @throws DbException
 */
void Db::insert(const std::vector<std::shared_ptr<const Course>>& courses) throw (DbException)
{
	open();

	begin_transaction(*db);

	try
	{
		for (const auto& it : courses)
			insertCourseHelper(*db, *it);
		end_transaction(*db);
	}
	catch (...)
	{
		qWarning() << "Transaction failed -> rollback";
		db->rollback();
		throw;
	}
}

void Db::deleteCourse(const QUuid& courseId) throw (DbException)
{
	open();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(
	    QStringLiteral("DELETE FROM tblCourse WHERE pkCourseUuid = :course"));
	q.bindValue(":course", courseId);

	exec_query(q);
}

void Db::deleteLessonList(const QUuid& courseId) throw (DbException)
{
	open();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(
	    QStringLiteral("DELETE FROM tblLessonList WHERE fkCourseUuid IN (SELECT pkCourseUuid FROM vLessons WHERE pkCourseUuid = :course)"));
	q.bindValue(":course", courseId);

	exec_query(q);
}

bool Db::update(const Course& c) throw (DbException)
{
	/*open();*/

	// Try to find the course
	auto cEntry = course(c.getId());
	if (!cEntry)
		return false;

	// Add Lessons
	replaceLessons(*cEntry);

	// Check if update is needed
	if (*cEntry == c)
	{
		qDebug() << "No update needed";
		return true;
	}

	updateCourse(*db, c);

	deleteLessonList(c.getId());

	int parentId = -1;
	for (const auto& it : c)
	{
		// Try to find the lesson
		auto lEntry = lesson(it->getId());
		if (lEntry)
			updateLesson(*db, *it);
		else
			insertLesson(*db, *it);

		// Create list entries
		if (*c.begin() == it)
		{
			// Store the rowId of the last insert on the LessonList for next round
			parentId = insertLessonListHead(*db, *it);
		}
		else
		{
			parentId = insertLessonListNode(*db, *it, parentId);
		}
	}

	return true;
}

} /* namespace qtouch */

