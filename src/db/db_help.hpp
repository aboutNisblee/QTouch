/**
 * \file db_help.hpp
 *
 * \date 10.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef DB_HELP_HPP_
#define DB_HELP_HPP_

#include "db_const.hpp"
#include "utils/exceptions.hpp"

namespace qtouch
{
namespace db
{
namespace internal
{

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

inline void begin_transaction(QSqlDatabase& db)
{
	if (!db.transaction())
		throw DbException(QStringLiteral("Unable to begin transaction"), db.lastError());
}

inline void end_transaction(QSqlDatabase& db)
{
	if (!db.commit())
		throw DbException(QStringLiteral("Unable to commit transaction"), db.lastError());
}

inline void exec_query_string(QSqlQuery& q, const QString& str)
{
	if (!q.exec(str))
		throw DbException(QStringLiteral("Query failed: ") % lastQuery(q), q.lastError());
}

inline void exec_query(QSqlQuery& q)
{
	if (!q.exec())
		throw DbException(QStringLiteral("Query failed: ") % lastQuery(q), q.lastError());
}

void configure(QSqlDatabase& db)
{
	QSqlQuery q(db);
	q.setForwardOnly(true);

	/* FIXME: Binding of values seems to impossible in PRAGMA. */
	QString pragma_user_version = QString("PRAGMA user_version = %1").arg(QTOUCH_DB_TARGET_VERSION);

	exec_query_string(q, pragma_user_version);

	exec_query_string(q, QStringLiteral("PRAGMA foreign_keys = true"));
}

void cleanDanglingLessons(QSqlDatabase& db)
{
	QSqlQuery q(db);
	q.setForwardOnly(true);

	exec_query_string(q, QStringLiteral("DELETE FROM tblLesson WHERE pkLessonUuid IN (SELECT pkLessonUuid FROM vDanglingLessons)"));
}

void updateMeta(QSqlDatabase& db, const QString& key, const QVariant& value)
{
	QSqlQuery q(db);
	q.setForwardOnly(true);

	q.prepare(QStringLiteral("INSERT OR REPLACE INTO tblMeta VALUES (:key, :value)"));
	q.bindValue(":key", key);
	q.bindValue(":value", value);

	exec_query(q);
}

QVariant getMeta(QSqlDatabase& db, const QString& key)
{
	QSqlQuery q(db);
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

void dropTables(QSqlDatabase& db)
{
	QSqlQuery q(db);
	q.setForwardOnly(true);

	begin_transaction(db);

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

		end_transaction(db);
	}
	catch (...)
	{
		qWarning() << "Transaction failed -> Rollback";
		db.rollback();
		throw;
	}
}

void createScheme(QSqlDatabase& db)
{
	QSqlQuery q(db);
	q.setForwardOnly(true);

	begin_transaction(db);

	try
	{
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

		end_transaction(db);
	}
	catch (...)
	{
		qWarning() << "Transaction failed -> Rollback";
		db.rollback();
		throw;
	}
}

void insertCourse(QSqlDatabase& db, const Course& course)
{
	QSqlQuery q(db);
	q.setForwardOnly(true);

	q.prepare(QStringLiteral("INSERT INTO tblCourse VALUES (:id, :title, :description, :builtin)"));
	q.bindValue(":id", course.getId());
	q.bindValue(":title", course.getTitle());
	q.bindValue(":description", course.getDescription());
	q.bindValue(":builtin", course.isBuiltin());

	exec_query(q);
}

void updateCourse(QSqlDatabase& db, const Course& course)
{
	QSqlQuery q(db);
	q.setForwardOnly(true);

	q.prepare(
	    QStringLiteral("UPDATE tblCourse SET cCourseTitle = :title, cDescription = :description, cCourseBuiltin = :builtin WHERE pkCourseUuid = :id"));
	q.bindValue(":title", course.getTitle());
	q.bindValue(":description", course.getDescription());
	q.bindValue(":builtin", course.isBuiltin());
	q.bindValue(":id", course.getId());

	exec_query(q);
}

void insertLesson(QSqlDatabase& db, const Lesson& lesson)
{
	QSqlQuery q(db);
	q.setForwardOnly(true);

	q.prepare(QStringLiteral("INSERT INTO tblLesson VALUES (:id, :title, :newChars, :builtin, :text)"));
	q.bindValue(":id", lesson.getId());
	q.bindValue(":title", lesson.getTitle());
	q.bindValue(":newChars", lesson.getNewChars());
	q.bindValue(":builtin", lesson.isBuiltin());
	q.bindValue(":text", lesson.getText());

	exec_query(q);
}

void updateLesson(QSqlDatabase& db, const Lesson& lesson)
{
	QSqlQuery q(db);
	q.setForwardOnly(true);

	q.prepare(QStringLiteral("UPDATE tblLesson SET cLessonTitle = :title, cNewChars = :newChars, cLessonBuiltin = :builtin, cText = :text WHERE pkLessonUuid = :id"));
	q.bindValue(":title", lesson.getTitle());
	q.bindValue(":newChars", lesson.getNewChars());
	q.bindValue(":builtin", lesson.isBuiltin());
	q.bindValue(":text", lesson.getText());
	q.bindValue(":id", lesson.getId());

	exec_query(q);
}

int insertLessonListHead(QSqlDatabase& db, const Lesson& lesson)
{
	QSqlQuery q(db);
	q.setForwardOnly(true);

	q.prepare(QStringLiteral("INSERT INTO tblLessonList(fkCourseUuid,fkLessonUuid) VALUES (:course_id, :lesson_id)"));
	q.bindValue(":course_id", lesson.getCourse()->getId());
	q.bindValue(":lesson_id", lesson.getId());

	exec_query(q);

	return q.lastInsertId().toInt();
}

int insertLessonListNode(QSqlDatabase& db, const Lesson& lesson, int parent)
{
	QSqlQuery q(db);
	q.setForwardOnly(true);

	q.prepare(QStringLiteral("INSERT INTO tblLessonList(fkLessonUuid,fkParentId) VALUES (:lesson_id, :parent_id)"));
	q.bindValue(":lesson_id", lesson.getId());
	q.bindValue(":parent_id", parent);

	exec_query(q);

	return q.lastInsertId().toInt();
}

} /* namespace qtouch */
} /* namespace db */
} /* namespace internal */


#endif /* DB_HELP_HPP_ */
