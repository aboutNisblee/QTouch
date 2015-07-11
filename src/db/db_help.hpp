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

typedef QSharedPointer<QSqlDatabase> DbPtr;

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

inline void begin_transaction(DbPtr db)
{
	if (!db->transaction())
		throw SqlException(QStringLiteral("Unable to begin transaction"), db->lastError().databaseText());
}

inline void end_transaction(DbPtr db)
{
	if (!db->commit())
		throw SqlException(QStringLiteral("Unable to commit transaction"), db->lastError().databaseText());
}

inline void exec_query_string(DbPtr db, QSqlQuery& q, const QString& str)
{
	if (!q.exec(str))
		throw SqlException(QStringLiteral("Query failed: ") % lastQuery(q), db->lastError().databaseText());
}

inline void exec_query(DbPtr db, QSqlQuery& q)
{
	if (!q.exec())
		throw SqlException(QStringLiteral("Query failed: ") % lastQuery(q), db->lastError().databaseText());
}

void configure(DbPtr db)
{
	QSqlQuery q(*db);

	/* FIXME: Binding of values seems to impossible in PRAGMA. */
	QString pragma_user_version = QString("PRAGMA user_version = %1").arg(QTOUCH_DB_TARGET_VERSION);

	exec_query_string(db, q, pragma_user_version);

	exec_query_string(db, q, QStringLiteral("PRAGMA foreign_keys = true"));
}

/*
 * Drop all tables, views and triggers.
 */
void dropTables(DbPtr db)
{
	QSqlQuery q(*db);

	begin_transaction(db);

	try
	{
		exec_query_string(db, q, "DROP TRIGGER IF EXISTS StatsDateTimeCheck");
		exec_query_string(db, q, "DROP TRIGGER IF EXISTS LessonListLessonDeleter");
		exec_query_string(db, q, "DROP TRIGGER IF EXISTS LessonListAfterInsert");
		exec_query_string(db, q, "DROP TRIGGER IF EXISTS LessonListBeforeInsert");

		exec_query_string(db, q, "DROP VIEW IF EXISTS vLessonListForward");
		exec_query_string(db, q, "DROP VIEW IF EXISTS vLessons");

		exec_query_string(db, q, "DROP TABLE IF EXISTS tblStats");
		exec_query_string(db, q, "DROP TABLE IF EXISTS tblLessonList");
		exec_query_string(db, q, "DROP TABLE IF EXISTS tblCourse");
		exec_query_string(db, q, "DROP TABLE IF EXISTS tblLesson");
		exec_query_string(db, q, "DROP TABLE IF EXISTS tblProfile");

		end_transaction(db);
	}
	catch (...)
	{
		qWarning() << "Transaction failed -> Rollback";
		db->rollback();
		throw;
	}
}

void createScheme(DbPtr db)
{
	QSqlQuery q(*db);

	begin_transaction(db);

	try
	{
		exec_query_string(db, q, create_tblProfile);
		exec_query_string(db, q, create_tblLesson);
		exec_query_string(db, q, create_tblCourse);
		exec_query_string(db, q, create_tblLessonList);
		exec_query_string(db, q, create_tblStats);

		exec_query_string(db, q, create_vLessonListForward);
		exec_query_string(db, q, create_vLessons);

		exec_query_string(db, q, create_LessonListBeforeInsert);
		exec_query_string(db, q, create_LessonListAfterInsert);
		exec_query_string(db, q, create_LessonListLessonDeleter);

		exec_query_string(db, q, create_StatsDateTimeCheck);

		end_transaction(db);
	}
	catch (...)
	{
		qWarning() << "Transaction failed -> Rollback";
		db->rollback();
		throw;
	}
}

} /* namespace qtouch */
} /* namespace db */
} /* namespace internal */


#endif /* DB_HELP_HPP_ */
