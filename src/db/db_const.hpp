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

const QString create_tblMeta = QStringLiteral("CREATE TABLE IF NOT EXISTS tblMeta (\n"
                               "	pkKey				TEXT NOT NULL PRIMARY KEY,\n"
                               "	cValue				TEXT\n"
                               ") WITHOUT ROWID;");

const QString meta_courseHash = QStringLiteral("BuiltInCourseHash");

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

} /* namespace qtouch */
} /* namespace db */
} /* namespace internal */


#endif /* DB_CONST_HPP_ */
