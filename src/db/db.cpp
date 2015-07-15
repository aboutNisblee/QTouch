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

namespace qtouch
{

namespace
{

#include "db_help.hpp"
using namespace qtouch::db::internal;

void ins(DbPtr db, const ConstCoursePtr& course)
{
	QSqlQuery q(*db);

	q.prepare(QStringLiteral("INSERT INTO tblCourse VALUES (:id, :title, :description, :builtin)"));
	q.bindValue(":id", course->getId());
	q.bindValue(":title", course->getTitle());
	q.bindValue(":description", course->getDescription());
	q.bindValue(":builtin", course->isBuiltin());

	//		qDebug() << lastQuery(q);

	exec_query(db, q);

	for (Course::const_iterator it = course->begin(); it != course->end(); ++it)
	{
		q.prepare(QStringLiteral("INSERT INTO tblLesson VALUES (:id, :title, :newChars, :builtin, :text)"));
		q.bindValue(":id", (*it)->getId());
		q.bindValue(":title", (*it)->getTitle());
		q.bindValue(":newChars", (*it)->getNewChars());
		q.bindValue(":builtin", (*it)->isBuiltin());
		q.bindValue(":text", (*it)->getText());

		//			qDebug() << lastQuery(q);

		exec_query(db, q);

		int lessonId = q.lastInsertId().toInt();

		// Create list entry
		if (course->begin() == it)
		{
			q.prepare(QStringLiteral("INSERT INTO tblLessonList(fkCourseUuid,fkLessonUuid) VALUES (:course_id, :lesson_id)"));
			q.bindValue(":course_id", course->getId());
			q.bindValue(":lesson_id", (*it)->getId());
		}
		else
		{
			q.prepare(QStringLiteral("INSERT INTO tblLessonList(fkLessonUuid,fkParentId) VALUES (:lesson_id, :parent_id)"));
			q.bindValue(":lesson_id", (*it)->getId());
			q.bindValue(":parent_id", lessonId);
		}

		exec_query(db, q);
	}
}

} /* namespace */

// Public methods

Db::Db(const QString& path) :
	db(new QSqlDatabase(QSqlDatabase::addDatabase(QStringLiteral("QSQLITE")))), mPath(path)
{
}

Db::~Db()
{
	close();
}

void Db::open(const QString& path, bool force_creation) throw (DatabaseException)
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
		throw SqlDriverException(QStringLiteral("Cannot add database driver"), db->lastError().driverText());

	// If the given path is empty, take the stored one
	QString tmp = path.isEmpty() ? mPath : path;

	// If that was also empty, throw an exception
	if (tmp.isEmpty())
		throw DatabaseException(QStringLiteral("No database path passed"));

	// Check existence of the directory to the database file
	if (!QDir(QFileInfo(tmp).absoluteDir()).exists())
	{
		throw
		DatabaseException(QStringLiteral("Invalid path to database file: ") % tmp);
	}

	// Else open or create the database file
	db->setDatabaseName(tmp);
	if (!db->open())
	{
		throw SqlDriverException(QStringLiteral("Unable to open database at ") % path, db->lastError().driverText());
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

		dropTables(db);

		configure(db);

		createScheme(db);
	}
	else if (version == QTOUCH_DB_TARGET_VERSION)
	{
	}
	else if (version < QTOUCH_DB_TARGET_VERSION)
	{
		throw DatabaseException(QStringLiteral("Upgrade of database not supported yet! Call your local developer ..."));
	}
	else
	{
		/* TODO: At least in this case, we must ask the user!
		 * So use a delegate or an enum as return value and an make the create frunction accessable! */
		throw DatabaseException(
		    QStringLiteral("Seems that you downgraded the application. Backup of old database was made ..."));
	}
}

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

int Db::getUserVersion() throw (DatabaseException)
{
	open();

	int version = -1;
	QSqlQuery q(*db);

	exec_query_string(db, q, QStringLiteral("PRAGMA user_version"));

	if (q.next())
		version = q.value(0).toInt();

	return version;
}

void Db::insert(const Profile& profile) throw (DatabaseException)
{
	open();

	qDebug() << "Implement me!";
}

void Db::insert(const CoursePtr& course) throw (DatabaseException)
{
	open();

	begin_transaction(db);

	try
	{
		ins(db, course);
		end_transaction(db);
	}
	catch (...)
	{
		qWarning() << "Transaction failed -> rollback";
		db->rollback();
		throw;
	}
}

void Db::insert(const ConstCourseList& courses) throw (DatabaseException)
{
	open();

	begin_transaction(db);

	try
	{
		for (ConstCourseList::const_iterator it = courses.begin(); it != courses.end(); ++it)
			ins(db, *it);
		end_transaction(db);
	}
	catch (...)
	{
		qWarning() << "Transaction failed -> rollback";
		db->rollback();
		throw;
	}
}

CourseList Db::courses(COURSE_SELECTOR selector) throw (DatabaseException)
{
	open();

	CourseList result;

	QSqlQuery q(*db);

	QString stmt = QStringLiteral("SELECT pkCourseUuid,cCourseTitle,cDescription,cCourseBuiltin FROM tblCourse");
	if (ALL != selector)
		stmt.append(" WHERE cCourseBuiltin = :builtin");

	// TODO: ORDER?
//	stmt.append(" ORDER BY cCourseTitle");

	q.setForwardOnly(true);
	q.prepare(stmt);

	if (ALL != selector)
		q.bindValue(":builtin", (BUILTIN == selector ? 1 : 0));

	exec_query(db, q);

	while (q.next())
	{
		CoursePtr out = Course::create();
		out->setId(QUuid(q.value(0).toString()));
		out->setTitle(q.value(1).toString());
		out->setDescription(q.value(2).toString());
		out->setBuiltin(q.value(3).toBool());

		result.append(out);
	}

	return result;
}

LessonList Db::lessons(const CoursePtr& course) throw (DatabaseException)
{
	open();

	LessonList result;

	QSqlQuery q(*db);

	q.prepare(
	    QStringLiteral("SELECT pkLessonUuid,cLessonTitle,cNewChars,cLessonBuiltin,cText FROM vLessons WHERE pkCourseUuid = :course_id"));
	q.bindValue(":course_id", course->getId());

	exec_query(db, q);

	while (q.next())
	{
		LessonPtr lesson(new Lesson);
		lesson->setId(QUuid(q.value(0).toString()));
		lesson->setTitle(q.value(1).toString());
		lesson->setNewChars(q.value(2).toString());
		lesson->setBuiltin(q.value(3).toBool());
		lesson->setText(q.value(4).toString());

		result.append(lesson);
	}

	return result;
}

} /* namespace qtouch */
