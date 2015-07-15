/**
 * \file db_const.hpp
 *
 * \date 10.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef DB_CONST_HPP_
#define DB_CONST_HPP_

#include <QString>

namespace qtouch
{
namespace db
{
namespace internal
{

const QString create_tblProfile = QStringLiteral("CREATE TABLE IF NOT EXISTS tblProfile (\n"
                                  "	cProfileName		TEXT NOT NULL PRIMARY KEY,\n"
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
                                     "	fkChildId			INTEGER DEFAULT NULL REFERENCES tblLessonList(pkLessonListId)\n"
                                     ");");

const QString create_LessonListBeforeInsert =
    QStringLiteral("CREATE TRIGGER IF NOT EXISTS LessonListBeforeInsert BEFORE INSERT ON tblLessonList\n"
                   "BEGIN\n"
                   "	SELECT RAISE(ABORT, \'LessonList constraint failed: List head lacks course\') FROM tblLessonList\n"
                   "		WHERE NEW.fkParentId ISNULL AND (SELECT pkCourseUuid FROM tblCourse WHERE pkCourseUuid = NEW.fkCourseUuid) ISNULL;\n"
                   "\n"
                   "	SELECT RAISE(ABORT, \'LessonList constraint failed: Course already connected\') FROM tblLessonList\n"
                   "		WHERE NEW.fkParentId ISNULL AND (SELECT count(*) FROM tblLessonList WHERE fkCourseUuid = NEW.fkCourseUuid) > 0;\n"
                   "\n"
                   "	SELECT RAISE(ABORT, \'LessonList constraint failed: Wrong course ID\') FROM tblLessonList\n"
                   "		WHERE NEW.fkParentId NOTNULL AND (SELECT fkCourseUuid FROM tblLessonList WHERE pkLessonListId = NEW.fkParentId) != NEW.fkCourseUuid;\n"
                   "END;");

const QString create_LessonListAfterInsert =
    QStringLiteral("CREATE TRIGGER IF NOT EXISTS LessonListAfterInsert AFTER INSERT ON tblLessonList\n"
                   "BEGIN\n"
                   "	-- Ensure that each entry is connected to its course\n"
                   "	UPDATE tblLessonList SET fkCourseUuid = (SELECT fkCourseUuid FROM tblLessonList WHERE pkLessonListId = NEW.fkParentId) WHERE fkCourseUuid ISNULL;\n"
                   "\n"
                   "	-- Add forward pointer\n"
                   "	UPDATE tblLessonList SET fkChildId = NEW.pkLessonListId WHERE pkLessonListId = NEW.fkParentId;\n"
                   "END;");


const QString create_LessonListLessonDeleter =
    QStringLiteral("CREATE TRIGGER IF NOT EXISTS LessonListLessonDeleter AFTER DELETE ON tblLessonList\n"
                   "WHEN (SELECT count(*) FROM tblLessonList WHERE fkLessonUuid = OLD.fkLessonUuid) = 0\n"
                   "BEGIN\n"
                   "	DELETE FROM tblLesson WHERE pkLessonUuid = OLD.fkLessonUuid;\n"
                   "END;");

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
                                "	cText\n"
                                "FROM (\n"
                                "	SELECT tblLesson.*,fkCourseUuid\n"
                                "	FROM tblLesson JOIN vLessonListForward ON pkLessonUuid = fkLessonUuid) AS Lesson\n"
                                "JOIN tblCourse ON pkCourseUuid = fkCourseUuid;");

const QString create_tblStats = QStringLiteral("CREATE TABLE IF NOT EXISTS tblStats (\n"
                                "	pkStartDateTime		TEXT NOT NULL,\n"
                                "	pkfkProfileName		TEXT NOT NULL REFERENCES tblProfile(cProfileName) ON UPDATE CASCADE ON DELETE CASCADE,\n"
                                "	cEndDateTime		TEXT NOT NULL,\n"
                                "	cCharCount			INTEGER NOT NULL,\n"
                                "	cErrorCount			INTEGER,\n"
                                "	fkLessonListId		INTEGER NOT NULL REFERENCES tblLessonList(pkLessonListId) ON UPDATE CASCADE ON DELETE CASCADE,\n"
                                "	PRIMARY KEY(pkStartDateTime, pkfkProfileName)\n"
                                ") WITHOUT ROWID;");

const QString create_StatsDateTimeCheck =
    QStringLiteral("CREATE TRIGGER IF NOT EXISTS StatsDateTimeCheck BEFORE INSERT ON tblStats\n"
                   "WHEN strftime(\'%s\',NEW.pkStartDateTime) > strftime(\'%s\',NEW.cEndDateTime)\n"
                   "BEGIN\n"
                   "	SELECT RAISE(ABORT, \'Datetime constraint failed: Start bigger than end time\');\n"
                   "END;");

} /* namespace qtouch */
} /* namespace db */
} /* namespace internal */


#endif /* DB_CONST_HPP_ */
