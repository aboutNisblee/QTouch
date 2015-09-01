/**
 * \file dbv1.cpp
 *
 * \date 31.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "dbv1.hpp"

#include <QFileInfo>
#include <QDir>
#include <QtSql/QSqlDatabase>

#include <QDebug>

namespace qtouch
{

namespace
{

/* FIXME: MSVC isn't able to concatenate strings inside the QStringLiteral macro.
 * See: https://blog.qt.io/blog/2014/06/13/qt-weekly-13-qstringliteral/ */
const QString create_tblMeta = QStringLiteral("CREATE TABLE IF NOT EXISTS tblMeta (\n"
                               "	pkKey				TEXT NOT NULL PRIMARY KEY,\n"
                               "	cValue				TEXT\n"
                               ") WITHOUT ROWID;");

const QString create_tblProfile = QStringLiteral("CREATE TABLE IF NOT EXISTS tblProfile (\n"
                                  "	pkProfileName		TEXT NOT NULL PRIMARY KEY,\n"
                                  "	cSkillLevel			INTEGER NOT NULL DEFAULT 0\n"
                                  " ) WITHOUT ROWID;");

const QString create_tblLesson = QStringLiteral("CREATE TABLE IF NOT EXISTS tblLesson (\n"
                                 "	pkLessonUuid		TEXT NOT NULL PRIMARY KEY,\n"
                                 "	cLessonTitle		TEXT NOT NULL,\n"
                                 "	cNewChars			TEXT,\n"
                                 "	cLessonBuiltin		NUMERIC NOT NULL DEFAULT 0,\n"
                                 "	cText				TEXT\n"
                                 ") WITHOUT ROWID;");

const QString create_tblCourse = QStringLiteral("CREATE TABLE IF NOT EXISTS tblCourse (\n"
                                 "	pkCourseUuid		TEXT NOT NULL PRIMARY KEY,\n"
                                 "	cCourseTitle		TEXT NOT NULL,\n"
                                 "	cDescription		TEXT,\n"
                                 "	cCourseBuiltin		NUMERIC NOT NULL DEFAULT 0\n"
                                 ") WITHOUT ROWID;");

const QString create_tblLessonList = QStringLiteral("CREATE TABLE IF NOT EXISTS tblLessonList (\n"
                                     "	pkLessonListId		INTEGER PRIMARY KEY AUTOINCREMENT,\n"
                                     "	fkCourseUuid		TEXT REFERENCES tblCourse(pkCourseUuid) ON UPDATE CASCADE ON DELETE CASCADE,\n"
                                     "	fkLessonUuid		TEXT NOT NULL REFERENCES tblLesson(pkLessonUuid) ON UPDATE CASCADE ON DELETE CASCADE,\n"
                                     "	-- List handling\n"
                                     "	fkParentId			INTEGER DEFAULT NULL REFERENCES tblLessonList(pkLessonListId),\n"
                                     "	fkChildId			INTEGER DEFAULT NULL REFERENCES tblLessonList(pkLessonListId),\n"
                                     "  UNIQUE(fkCourseUuid,fkLessonUuid)\n"
                                     ");");

const QString create_LessonListBeforeInsert =
    QStringLiteral("CREATE TRIGGER IF NOT EXISTS LessonListBeforeInsert BEFORE INSERT ON tblLessonList\n"
                   "BEGIN\n"
                   "	SELECT RAISE(ABORT, \'LessonList constraint failed: List head lacks course\') FROM tblLessonList\n"
                   "		WHERE NEW.fkParentId ISNULL AND (SELECT pkCourseUuid FROM tblCourse WHERE pkCourseUuid = NEW.fkCourseUuid) ISNULL;\n"
                   "\n"
                   "	SELECT RAISE(ABORT, \'LessonList constraint failed: Wrong course ID\') FROM tblLessonList\n"
                   "		WHERE NEW.fkParentId NOTNULL AND (SELECT fkCourseUuid FROM tblLessonList WHERE pkLessonListId = NEW.fkParentId) != NEW.fkCourseUuid;\n"
                   "END;");

const QString create_LessonListAfterInsert =
    QStringLiteral("CREATE TRIGGER IF NOT EXISTS LessonListAfterInsert AFTER INSERT ON tblLessonList\n"
                   "WHEN NEW.fkParentId IS NOT NULL\n"
                   "BEGIN\n"
                   "	-- Ensure that each entry is connected to its course\n"
                   "	UPDATE tblLessonList SET fkCourseUuid = (SELECT fkCourseUuid FROM tblLessonList WHERE pkLessonListId = NEW.fkParentId) WHERE fkCourseUuid ISNULL;\n"
                   "\n"
                   "	-- Add forward pointer\n"
                   "	-- This will trigger LessonListBeforeChildIdUpdate with parent row as NEW\n"
                   "	UPDATE tblLessonList SET fkChildId = NEW.pkLessonListId WHERE pkLessonListId = NEW.fkParentId;\n"
                   "END;");


const QString create_LessonListAfterInsertHead =
    QStringLiteral("CREATE TRIGGER IF NOT EXISTS LessonListAfterInsertHead AFTER INSERT ON tblLessonList\n"
                   "WHEN NEW.fkParentId IS NULL\n"
                   "BEGIN\n"
                   "	-- Ensure that each entry is connected to its course\n"
                   "	UPDATE tblLessonList SET fkCourseUuid = (SELECT fkCourseUuid FROM tblLessonList WHERE pkLessonListId = NEW.fkParentId) WHERE fkCourseUuid ISNULL;\n"
                   "\n"
                   "	-- Update parent pointer of old head\n"
                   "	UPDATE tblLessonList SET fkParentId = NEW.pkLessonListId WHERE fkCourseUuid = NEW.fkCourseUuid AND fkParentId IS NULL AND pkLessonListId != NEW.pkLessonListId;\n"
                   "\n"
                   "	-- Update the child pointer of the new head to point to the old head\n"
                   "	UPDATE tblLessonList SET fkChildId = (SELECT pkLessonListId FROM tblLessonList WHERE fkParentId = NEW.pkLessonListId) WHERE pkLessonListId = NEW.pkLessonListId;\n"
                   "END;");

const QString create_LessonListBeforeChildIdUpdate =
    QStringLiteral("CREATE TRIGGER IF NOT EXISTS LessonListBeforeChildIdUpdate BEFORE UPDATE OF fkChildId ON tblLessonList\n"
                   "WHEN (SELECT count(*) FROM tblLessonList WHERE fkParentId = OLD.pkLessonListId) > 1\n"
                   "BEGIN\n"
                   "	-- Update parent pointer of the old child\n"
                   "	UPDATE tblLessonList SET fkParentId = NEW.fkChildId WHERE pkLessonListId = OLD.fkChildId;\n"
                   "	-- Update child pointer of the new child\n"
                   "	UPDATE tblLessonList SET fkChildId = OLD.fkChildId WHERE pkLessonListId = NEW.fkChildId;\n"
                   "END;");

const QString create_LessonListBeforeDelete =
    QStringLiteral("CREATE TRIGGER IF NOT EXISTS LessonListBeforeDelete BEFORE DELETE ON tblLessonList\n"
                   "BEGIN\n"
                   "	-- Update child pointer of the old parent\n"
                   "	UPDATE tblLessonList SET fkChildId = OLD.fkChildId WHERE pkLessonListId = OLD.fkParentId;\n"
                   "	-- Update parent pointer of the old child\n"
                   "	UPDATE tblLessonList SET fkParentId = OLD.fkParentId WHERE pkLessonListId = OLD.fkChildId;\n"
                   "END;");

const QString create_vDanglingLessons = QStringLiteral("CREATE VIEW IF NOT EXISTS vDanglingLessons AS\n"
                                        "SELECT pkLessonUuid FROM tblLesson WHERE pkLessonUuid NOT IN  (SELECT fkLessonUuid FROM tblLessonList);");

const QString create_vLessonListForward = QStringLiteral("CREATE VIEW IF NOT EXISTS vLessonListForward AS\n"
        "WITH RECURSIVE LessonListForward(pkLessonListId, fkCourseUuid, fkLessonUuid, fkParentId, fkChildId) AS\n"
        "(\n"
        "	-- pkLessonListId, fkCourseUuid, fkLessonUuid, fkParentId, fkChildId\n"
        "	SELECT * FROM tblLessonList WHERE fkParentId IS NULL\n"
        "    UNION ALL\n"
        "    SELECT tblLessonList.pkLessonListId,tblLessonList.fkCourseUuid,tblLessonList.fkLessonUuid,tblLessonList.fkParentId,tblLessonList.fkChildId\n"
        "		FROM tblLessonList, LessonListForward\n"
        "		WHERE LessonListForward.fkChildId = tblLessonList.pkLessonListId\n"
        ")\n"
        "SELECT * FROM LessonListForward;");

const QString create_vLessons = QStringLiteral("CREATE VIEW IF NOT EXISTS vLessons AS\n"
                                "SELECT pkCourseUuid,\n"
                                "	cCourseTitle,\n"
                                "	cDescription,\n"
                                "	cCourseBuiltin,\n"
                                "	pkLessonUuid,\n"
                                "	cLessonTitle,\n"
                                "	cNewChars,\n"
                                "	cLessonBuiltin,\n"
                                "	cText,\n"
                                "	pkLessonListId\n"
                                "FROM (\n"
                                "	SELECT tblLesson.*,fkCourseUuid,pkLessonListId\n"
                                "	FROM tblLesson JOIN vLessonListForward ON pkLessonUuid = fkLessonUuid) AS Lesson\n"
                                "JOIN tblCourse ON pkCourseUuid = fkCourseUuid;");

const QString create_tblStats = QStringLiteral("CREATE TABLE IF NOT EXISTS tblStats (\n"
                                "	pkfkLessonUuid		TEXT NOT NULL REFERENCES tblLesson(pkLessonUuid) ON UPDATE CASCADE ON DELETE CASCADE,\n"
                                "	pkfkProfileName		TEXT NOT NULL REFERENCES tblProfile(pkProfileName) ON UPDATE CASCADE ON DELETE CASCADE,\n"
                                "	cStartDateTime		TEXT NOT NULL,\n"
                                "	cEndDateTime		TEXT NOT NULL,\n"
                                "	cCharCount			INTEGER NOT NULL,\n"
                                "	cErrorCount			INTEGER,\n"
                                "	PRIMARY KEY(pkfkLessonUuid, pkfkProfileName)\n"
                                ") WITHOUT ROWID;");

const QString create_StatsDateTimeCheck =
    QStringLiteral("CREATE TRIGGER IF NOT EXISTS StatsDateTimeCheck BEFORE INSERT ON tblStats\n"
                   "WHEN strftime(\'%s\',NEW.cStartDateTime) > strftime(\'%s\',NEW.cEndDateTime)\n"
                   "BEGIN\n"
                   "	SELECT RAISE(ABORT, \'Datetime constraint failed: Start bigger than end time\');\n"
                   "END;");

inline QString lastQuery(const QSqlQuery& query)
{
	QString str = query.lastQuery();
	QMapIterator<QString, QVariant> it(query.boundValues());
	while (it.hasNext())
	{
		it.next();
		str.replace(it.key(), it.value().toString());
	}
	return str;
}

inline void exec_query(QSqlQuery& q)
{
	if (!q.exec())
		throw DbException(QStringLiteral("Query failed: ") % lastQuery(q), q.lastError());
}

inline void exec_query_string(QSqlQuery& q, const QString& str)
{
	if (!q.exec(str))
		throw DbException(QStringLiteral("Query failed: ") % lastQuery(q), q.lastError());
}

} /* namespace */

std::unique_ptr<DbV1> DbV1::create()
{
	return std::unique_ptr<DbV1>(new DbV1);
}

DbV1::~DbV1()
{
	close();
}

void DbV1::open(const QString& path)
{
	if (!db)
		db.reset(new QSqlDatabase(QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"))));

	// Check for valid driver
	if (!db->isValid())
		throw DbException(QStringLiteral("Cannot add database driver"), db->lastError());

	if (path.isEmpty())
		throw DbException(QStringLiteral("No database path!"));

	// Check existence of the directory to the database file
	if (!QDir(QFileInfo(path).absoluteDir()).exists())
	{
		throw
		DbException(QStringLiteral("Invalid path to database: ") % path);
	}

	// Else open or create the database file
	db->setDatabaseName(path);
	if (!db->open())
	{
		throw DbException(QStringLiteral("Unable to open database at ") % path, db->lastError());
	}
	else
	{
		qDebug() << "Opened database at:" << QFileInfo(path).absolutePath();
	}
}

void DbV1::close()
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

void DbV1::createSchema()
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	begin_transaction();

	try
	{
		QString pragma_user_version = QString("PRAGMA user_version = %1").arg(VERSION);
		exec_query_string(q, pragma_user_version);
		exec_query_string(q, QStringLiteral("PRAGMA foreign_keys = true"));

		exec_query_string(q, create_tblMeta);
		exec_query_string(q, create_tblProfile);
		exec_query_string(q, create_tblLesson);
		exec_query_string(q, create_tblCourse);
		exec_query_string(q, create_tblLessonList);
		exec_query_string(q, create_tblStats);

		exec_query_string(q, create_vDanglingLessons);
		exec_query_string(q, create_vLessonListForward);
		exec_query_string(q, create_vLessons);

		exec_query_string(q, create_LessonListBeforeInsert);
		exec_query_string(q, create_LessonListAfterInsert);
		exec_query_string(q, create_LessonListAfterInsertHead);
		exec_query_string(q, create_LessonListBeforeChildIdUpdate);
		exec_query_string(q, create_LessonListBeforeDelete);
		exec_query_string(q, create_StatsDateTimeCheck);

		setMeta(Db::metaSchemaVersionKey, VERSION);

		end_transaction();
	}
	catch (...)
	{
		qWarning() << "Transaction failed -> Rollback";
		rollback();
		throw;
	}
}

void DbV1::dropSchema()
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	begin_transaction();

	try
	{
		exec_query_string(q, "DROP TRIGGER IF EXISTS StatsDateTimeCheck");
		exec_query_string(q, "DROP TRIGGER IF EXISTS LessonListBeforeDelete");
		exec_query_string(q, "DROP TRIGGER IF EXISTS LessonListBeforeChildIdUpdate");
		exec_query_string(q, "DROP TRIGGER IF EXISTS LessonListAfterInsertHead");
		exec_query_string(q, "DROP TRIGGER IF EXISTS LessonListAfterInsert");
		exec_query_string(q, "DROP TRIGGER IF EXISTS LessonListBeforeInsert");

		exec_query_string(q, "DROP VIEW IF EXISTS vLessons");
		exec_query_string(q, "DROP VIEW IF EXISTS vLessonListForward");
		exec_query_string(q, "DROP VIEW IF EXISTS vDanglingLessons");

		exec_query_string(q, "DROP TABLE IF EXISTS tblStats");
		exec_query_string(q, "DROP TABLE IF EXISTS tblLessonList");
		exec_query_string(q, "DROP TABLE IF EXISTS tblCourse");
		exec_query_string(q, "DROP TABLE IF EXISTS tblLesson");
		exec_query_string(q, "DROP TABLE IF EXISTS tblProfile");
		exec_query_string(q, "DROP TABLE IF EXISTS tblMeta");

		end_transaction();
	}
	catch (...)
	{
		qWarning() << "Transaction failed -> Rollback";
		rollback();
		throw;
	}
}

void DbV1::setMeta(const QString& key, const QVariant& value)
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(QStringLiteral("INSERT OR REPLACE INTO tblMeta VALUES (:key, :value)"));
	q.bindValue(":key", key);
	q.bindValue(":value", value);

	exec_query(q);
}

QVariant DbV1::getMeta(const QString& key)
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(QStringLiteral("SELECT cValue FROM tblMeta WHERE pkKey = :key"));
	q.bindValue(":key", key);

	exec_query(q);

	if (!q.next())
	{
		return QVariant();
	}
	return q.value(0);
}

void DbV1::begin_transaction()
{
	if (isOpen() && !db->transaction())
		throw DbException(QStringLiteral("Unable to begin transaction") % (!isOpen() ? ": Database not open" : ""),
		                  db->lastError());
}

void DbV1::end_transaction()
{
	if (isOpen() && !db->commit())
		throw DbException(QStringLiteral("Unable to commit transaction") % (!isOpen() ? ": Database not open" : ""),
		                  db->lastError());
}

void DbV1::rollback()
{
	if (isOpen())
		db->rollback();
}

void DbV1::insert(const Profile& profile)
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(QStringLiteral("INSERT INTO tblProfile VALUES (:name, :skill)"));
	q.bindValue(":name", profile.getName());
	q.bindValue(":skill", profile.getSkillLevel());

	exec_query(q);
}

void DbV1::insert(const Stats& stats)
{
	checkOpen();

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

void DbV1::insert(const Course& course)
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(QStringLiteral("INSERT INTO tblCourse VALUES (:id, :title, :description, :builtin)"));
	q.bindValue(":id", course.getId());
	q.bindValue(":title", course.getTitle());
	q.bindValue(":description", course.getDescription());
	q.bindValue(":builtin", course.isBuiltin());

	exec_query(q);
}

void DbV1::insert(const Lesson& lesson)
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(QStringLiteral("INSERT INTO tblLesson VALUES (:id, :title, :newChars, :builtin, :text)"));
	q.bindValue(":id", lesson.getId());
	q.bindValue(":title", lesson.getTitle());
	q.bindValue(":newChars", lesson.getNewChars());
	q.bindValue(":builtin", lesson.isBuiltin());
	q.bindValue(":text", lesson.getText());

	exec_query(q);
}

int DbV1::insert(const QUuid& courseId, const QUuid& lessonId, int parentId)
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	if (!parentId)
	{
		q.prepare(QStringLiteral("INSERT INTO tblLessonList(fkCourseUuid,fkLessonUuid) VALUES (:course_id, :lesson_id)"));
		q.bindValue(":course_id", courseId);
		q.bindValue(":lesson_id", lessonId);
	}
	else
	{
		q.prepare(
		    QStringLiteral("INSERT INTO tblLessonList(fkCourseUuid,fkLessonUuid,fkParentId) VALUES (:course_id, :lesson_id, :parent_id)"));
		q.bindValue(":course_id", courseId);
		q.bindValue(":lesson_id", lessonId);
		q.bindValue(":parent_id", parentId);
	}

	exec_query(q);

	return q.lastInsertId().toInt();
}

void DbV1::update(const Profile& profile)
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(
	    QStringLiteral("UPDATE tblProfile SET pkProfileName = :profile, cSkillLevel = :skill WHERE pkProfileName = :profile"));
	q.bindValue(":profile", profile.getName());
	q.bindValue(":skill", profile.getSkillLevel());

	exec_query(q);
}

void DbV1::update(const Stats& /*stats*/)
{
	checkOpen();

	// TODO: Implement me!
	qWarning() << "To be implemented!";
}

void DbV1::update(const Course& course)
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(
	    QStringLiteral("UPDATE tblCourse SET cCourseTitle = :title, cDescription = :description, cCourseBuiltin = :builtin WHERE pkCourseUuid = :id"));
	q.bindValue(":title", course.getTitle());
	q.bindValue(":description", course.getDescription());
	q.bindValue(":builtin", course.isBuiltin());
	q.bindValue(":id", course.getId());

	exec_query(q);
}

void DbV1::update(const Lesson& lesson)
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(
	    QStringLiteral("UPDATE tblLesson SET cLessonTitle = :title, cNewChars = :newChars, cLessonBuiltin = :builtin, cText = :text WHERE pkLessonUuid = :id"));
	q.bindValue(":title", lesson.getTitle());
	q.bindValue(":newChars", lesson.getNewChars());
	q.bindValue(":builtin", lesson.isBuiltin());
	q.bindValue(":text", lesson.getText());
	q.bindValue(":id", lesson.getId());

	exec_query(q);
}

/**
 * Select all Profiles.
 * Valid columns: pkProfileName, cSkillLevel
 * @return The query.
 */
QSqlQuery DbV1::selectProfiles()
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	exec_query_string(q, QStringLiteral("SELECT pkProfileName,cSkillLevel FROM tblProfile"));

	return q;
}

/**
 * Select the Stats for a given ProfileName
 * Valid columns: pkfkLessonUuid, cStartDateTime, cEndDateTime, cCharCount, cErrorCount
 * @param profileName A ProfileName
 * @return The query.
 */
QSqlQuery DbV1::selectStats(const QString& profileName)
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(
	    QStringLiteral("SELECT pkfkLessonUuid,cStartDateTime,cEndDateTime,cCharCount,cErrorCount FROM tblStats WHERE pkfkProfileName = :profileName"));
	q.bindValue(":profileName", profileName);

	exec_query(q);

	return q;
}

/**
 * Select all Courses.
 * @note The corresponding Lessons are NOT selected!
 * Valid columns: pkCourseUuid, cCourseTitle, cDescription, cCourseBuiltin
 * @param type Select a subset.
 * @return The query.
 */
QSqlQuery DbV1::selectCourses(Db::CourseType type)
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	QString stmt = QStringLiteral("SELECT pkCourseUuid,cCourseTitle,cDescription,cCourseBuiltin FROM tblCourse");
	if (Db::All != type)
		stmt.append(" WHERE cCourseBuiltin = :builtin");

	q.prepare(stmt);

	if (Db::All != type)
		q.bindValue(":builtin", (Db::BuiltIn == type ? "1" : "0"));

	exec_query(q);

	return q;
}

/**
 * Select specific Course.
 * Valid columns: pkCourseUuid, cCourseTitle, cDescription, cCourseBuiltin
 * @param courseId A CourseUuid.
 * @return The query.
 */
QSqlQuery DbV1::selectCourse(const QUuid& courseId)
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(
	    QStringLiteral("SELECT pkCourseUuid,cCourseTitle,cDescription,cCourseBuiltin FROM tblCourse WHERE pkCourseUuid = :course_id"));
	q.bindValue(":course_id", courseId);

	exec_query(q);

	return q;
}

/**
 * Select a lesson by its UUID.
 * Valid columns: pkLessonUuid, cLessonTitle, cNewChars, cLessonBuiltin, cText
 * @param lessonId A LessonUuid
 * @return The query.
 */
QSqlQuery DbV1::selectLesson(const QUuid& lessonId)
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(
	    QStringLiteral("SELECT pkLessonUuid,cLessonTitle,cNewChars,cLessonBuiltin,cText FROM tblLesson WHERE pkLessonUuid = :lesson_id"));
	q.bindValue(":lesson_id", lessonId);

	exec_query(q);

	return q;
}

/**
 * Select the LessonList of a specific Course.
 * Valid columns: pkLessonUuid, cLessonTitle, cNewChars, cLessonBuiltin, cText
 * @param courseId A CourseUuid.
 * @return The query.
 */
QSqlQuery DbV1::selectLessonList(const QUuid& courseId)
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(
	    QStringLiteral("SELECT pkLessonUuid,cLessonTitle,cNewChars,cLessonBuiltin,cText FROM vLessons WHERE pkCourseUuid = :course_id"));
	q.bindValue(":course_id", courseId);

	exec_query(q);

	return q;
}

/**
 * Select all Lessons that do not have any connected Course.
 * Valid column: pkLessonUuid
 * @return The query.
 */
QSqlQuery DbV1::selectDanglingLesson()
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	exec_query_string(q, QStringLiteral("SELECT pkLessonUuid FROM vDanglingLessons"));

	return q;
}

void DbV1::deleteProfile(const QString& profileName)
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(
	    QStringLiteral("DELETE FROM tblProfile WHERE pkProfileName = :profile"));
	q.bindValue(":profile", profileName);

	exec_query(q);
}

void DbV1::deleteStats(const QString& profileName)
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(
	    QStringLiteral("DELETE FROM tblStats WHERE pkfkProfileName = :profile"));
	q.bindValue(":profile", profileName);

	exec_query(q);
}

void DbV1::deleteCourse(const QUuid& courseId)
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(
	    QStringLiteral("DELETE FROM tblCourse WHERE pkCourseUuid = :course"));
	q.bindValue(":course", courseId);

	exec_query(q);
}

void DbV1::deleteLesson(const QUuid& lessonId)
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(
	    QStringLiteral("DELETE FROM tblLesson WHERE pkLessonUuid = :lesson"));
	q.bindValue(":lesson", lessonId);

	exec_query(q);
}

void DbV1::deleteLessonList(const QUuid& courseId)
{
	checkOpen();

	QSqlQuery q(*db);
	q.setForwardOnly(true);

	q.prepare(
	    QStringLiteral("DELETE FROM tblLessonList WHERE fkCourseUuid IN (SELECT pkCourseUuid FROM vLessons WHERE pkCourseUuid = :course)"));
	q.bindValue(":course", courseId);

	exec_query(q);
}

} /* namespace qtouch */
