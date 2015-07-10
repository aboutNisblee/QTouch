PRAGMA user_version = 1;
PRAGMA foreign_keys = true;

DROP TRIGGER IF EXISTS StatsDateTimeCheck;
DROP TRIGGER IF EXISTS LessonListLessonDeleter;
DROP TRIGGER IF EXISTS LessonListAfterInsert;
DROP TRIGGER IF EXISTS LessonListBeforeInsert;

DROP VIEW IF EXISTS vLessonListForward;
DROP VIEW IF EXISTS vLessons;

DROP TABLE IF EXISTS tblStats;
DROP TABLE IF EXISTS tblLessonList;
DROP TABLE IF EXISTS tblCourse;
DROP TABLE IF EXISTS tblLesson;
DROP TABLE IF EXISTS tblProfile;

-- User profiles.
CREATE TABLE IF NOT EXISTS tblProfile (
	cProfileName		TEXT NOT NULL PRIMARY KEY,
	cSkillLevel			INTEGER NOT NULL DEFAULT 0
) WITHOUT ROWID;

-- Table that holds the lessons.
-- Each lesson has a UUID taken from Ktouch, stored as text and used as PK.
-- The builtin flag is used to destinguish between user defined/created lessons and those delivered with the binary.
-- Lessons that are not attached to any Course are NOT allowed.
CREATE TABLE IF NOT EXISTS tblLesson (
	pkLessonUuid		TEXT NOT NULL PRIMARY KEY,
	cLessonTitle		TEXT NOT NULL,
	cNewChars			TEXT,
	cLessonBuiltin		NUMERIC NOT NULL DEFAULT 0,
	cText				TEXT
) WITHOUT ROWID;

-- A course is mainly a collection of lessons.
-- Courses without any Lessons are allowed.
-- The right order of the lessons is ensured by the LessonList table.
-- Each course holds a reference to its first lesson list entry.
-- TODO: Add a ON DELETE trigger, that checks whether linked lessons have any parents.
CREATE TABLE IF NOT EXISTS tblCourse (
	pkCourseUuid		TEXT NOT NULL PRIMARY KEY,
	cCourseTitle		TEXT NOT NULL,
	cDescription		TEXT,
	cCourseBuiltin		NUMERIC NOT NULL DEFAULT 0
) WITHOUT ROWID;

-- For the user to be able to create custom courses from existing lessons a resolution table is needed (m:n).
-- This is implemented as a list, to be able to store the order of the lessons.
-- Using a artificial index for the recursion instead of the long UUIDs.
-- For the user to be able to ommit the CourseUUID on subsequent inserts, fkCourseUuid is not forced to be not null.
--	The handling is done in a trigger.
-- FIXME: It is possible to make tree from the list by passing a parentID that differs from the ID of the last entry of a list.
--	For the moment the software must ensure that the right parentID is passed.
CREATE TABLE IF NOT EXISTS tblLessonList (
	pkLessonListId		INTEGER PRIMARY KEY AUTOINCREMENT,
	fkCourseUuid		TEXT REFERENCES tblCourse(pkCourseUuid) ON UPDATE CASCADE ON DELETE CASCADE,
	fkLessonUuid		TEXT NOT NULL REFERENCES tblLesson(pkLessonUuid) ON UPDATE CASCADE ON DELETE CASCADE,
	-- List handling
	fkParentId			INTEGER DEFAULT NULL REFERENCES tblLessonList(pkLessonListId),
	fkChildId			INTEGER DEFAULT NULL REFERENCES tblLessonList(pkLessonListId)
);

-- Ensure the list head is connected to a valid course before insert.
-- Ensure each coure is connected to only one list head.
-- Ensure that a child does not specify a different course than its parent.
CREATE TRIGGER IF NOT EXISTS LessonListBeforeInsert BEFORE INSERT ON tblLessonList
BEGIN
	SELECT RAISE(ABORT, 'LessonList constraint failed: List head lacks course') FROM tblLessonList
		WHERE NEW.fkParentId ISNULL AND (SELECT pkCourseUuid FROM tblCourse WHERE pkCourseUuid = NEW.fkCourseUuid) ISNULL;

	SELECT RAISE(ABORT, 'LessonList constraint failed: Course already connected') FROM tblLessonList
		WHERE NEW.fkParentId ISNULL AND (SELECT count(*) FROM tblLessonList WHERE fkCourseUuid = NEW.fkCourseUuid) > 0;

	SELECT RAISE(ABORT, 'LessonList constraint failed: Wrong course ID') FROM tblLessonList
		WHERE NEW.fkParentId NOTNULL AND (SELECT fkCourseUuid FROM tblLessonList WHERE pkLessonListId = NEW.fkParentId) != NEW.fkCourseUuid;
END;

-- If the user ommit the course UUID on subsequent inserts, set it automatically to the UUID of the parent
-- Add the forward pointer to the parent after inserting a new lesson linst entry that isn't the first one
CREATE TRIGGER IF NOT EXISTS LessonListAfterInsert AFTER INSERT ON tblLessonList
BEGIN
	-- Ensure that each entry is connected to its course
	UPDATE tblLessonList SET fkCourseUuid = (SELECT fkCourseUuid FROM tblLessonList WHERE pkLessonListId = NEW.fkParentId) WHERE fkCourseUuid ISNULL;

	-- Add forward pointer
	UPDATE tblLessonList SET fkChildId = NEW.pkLessonListId WHERE pkLessonListId = NEW.fkParentId;
END;

-- Prevent dangling Lessons
CREATE TRIGGER IF NOT EXISTS LessonListLessonDeleter AFTER DELETE ON tblLessonList
WHEN (SELECT count(*) FROM tblLessonList WHERE fkLessonUuid = OLD.fkLessonUuid) = 0
BEGIN
	DELETE FROM tblLesson WHERE pkLessonUuid = OLD.fkLessonUuid;
END;

CREATE VIEW IF NOT EXISTS vLessonListForward AS
WITH RECURSIVE LessonListForward(pkLessonListId, fkCourseUuid, fkLessonUuid, fkParentId, fkChildId) AS
(
	-- pkLessonListId, fkCourseUuid, fkLessonUuid, fkParentId, fkChildId
	SELECT * FROM tblLessonList WHERE fkParentId IS NULL
    UNION ALL
    SELECT tblLessonList.pkLessonListId,tblLessonList.fkCourseUuid,tblLessonList.fkLessonUuid,tblLessonList.fkParentId,tblLessonList.fkChildId
		FROM tblLessonList, LessonListForward
		WHERE LessonListForward.fkChildId = tblLessonList.pkLessonListId
)
SELECT * FROM LessonListForward;

CREATE VIEW IF NOT EXISTS vLessons AS
SELECT pkCourseUuid,
	cCourseTitle,
	cDescription,
	cCourseBuiltin,
	pkLessonUuid,
	cLessonTitle,
	cNewChars,
	cLessonBuiltin,
	cText
FROM (
	SELECT tblLesson.*,fkCourseUuid
	FROM tblLesson JOIN vLessonListForward ON pkLessonUuid = fkLessonUuid) AS Lesson
JOIN tblCourse ON pkCourseUuid = fkCourseUuid;

-- Table for statistics of passed lessons.
-- Has a composite PK of start time of the lesson and the profile name.
-- DateTimes are stored in ISO format (e.g. 2015-07-01T14:44:52+0200 or higher precision).
-- Each entry holds a reference to the resolution table between course and lesson.
-- When the profile, course or lesson is deleted, the stats are deleted too.
CREATE TABLE IF NOT EXISTS tblStats (
	pkStartDateTime		TEXT NOT NULL,
	pkfkProfileName		TEXT NOT NULL REFERENCES tblProfile(cProfileName) ON UPDATE CASCADE ON DELETE CASCADE,
	cEndDateTime		TEXT NOT NULL,
	cCharCount			INTEGER NOT NULL,
	cErrorCount			INTEGER,
	fkLessonListId		INTEGER NOT NULL REFERENCES tblLessonList(pkLessonListId) ON UPDATE CASCADE ON DELETE CASCADE,
	PRIMARY KEY(pkStartDateTime, pkfkProfileName)
) WITHOUT ROWID;

-- Check that Stats.end_datetime is bigger that Stats.start_datetime
CREATE TRIGGER IF NOT EXISTS StatsDateTimeCheck BEFORE INSERT ON tblStats
WHEN strftime('%s',NEW.pkStartDateTime) > strftime('%s',NEW.cEndDateTime)
BEGIN
	SELECT RAISE(ABORT, 'Datetime constraint failed: Start bigger than end time');
END;

-- *****************************
-- TESTS
-- *****************************

-- Add some example users
INSERT INTO tblProfile VALUES
	('TestUser1', 0),
	('TestUser2', 1);
-- Check count
SELECT 'Profile insert test failed'
	WHERE (SELECT COUNT(*) FROM tblProfile) != 2;

-- Add some courses
-- NOTE: Using short identifiers instead of long UUIDs for testing purposes
INSERT INTO tblCourse VALUES
	('C1', 'Course 1', 'Description', 1),
	('C2', 'Course 2', 'Description', 0),
	('C3', 'Course 3', 'Description', 0);
-- Check count
SELECT 'Course: Wrong row count'
	WHERE (SELECT COUNT(*) FROM tblCourse) != 3;

-- Add some lessons
INSERT INTO tblLesson VALUES
	('L1', 'Lesson 1', 'f', 1, 'fff'),
	('L2', 'Lesson 2', 'j', 1, 'fff jjj'),
	('L3', 'Lesson 3', 'd', 0, 'ddd'),
	('L4', 'Lesson 4', 'k', 0, 'ddd kkk');
-- Check count
SELECT 'Lesson: Wrong row count'
	WHERE (SELECT COUNT(*) FROM tblLesson) != 4;

-- Create a LessonsList by forward adding LessonList entries with a reference to a Lesson entry.
-- The first entry has no parent key.
INSERT INTO tblLessonList (fkCourseUuid,fkLessonUuid) VALUES ('C1','L1');
-- Append the second Lesson to the first one. The insert trigger should add the child key to the parent.
INSERT INTO tblLessonList (fkLessonUuid,fkParentId) VALUES ('L2', (SELECT pkLessonListId FROM tblLessonList WHERE fkLessonUuid = 'L1'));
INSERT INTO tblLessonList (fkLessonUuid,fkParentId) VALUES ('L3', (SELECT pkLessonListId FROM tblLessonList WHERE fkLessonUuid = 'L2'));
INSERT INTO tblLessonList (fkLessonUuid,fkParentId) VALUES ('L4', (SELECT pkLessonListId FROM tblLessonList WHERE fkLessonUuid = 'L3'));

-- Lesson 2 starts a new list and Lesson 4 is appended
INSERT INTO tblLessonList (fkLessonUuid,fkCourseUuid) VALUES ('L2','C2');
INSERT INTO tblLessonList (fkLessonUuid,fkParentId) VALUES ('L4', (SELECT last_insert_rowid() FROM tblLessonList WHERE fkLessonUuid = 'L2'));

-- With the help of the inser trigger we should now have:
-- L1 <-> L2 <-> L3 <-> L4
-- L2 <-> L4

-- Check the links
SELECT 'LessonListAfterInsert: Parent or child link broken'
	WHERE (SELECT fkParentId FROM tblLessonList WHERE fkLessonUuid = 'L1') != NULL
	OR (SELECT fkChildId FROM tblLessonList WHERE fkLessonUuid = 'L1') != NULL;

SELECT 'LessonListAfterInsert: Parent or child link broken'
	WHERE
		(SELECT fkParentId FROM tblLessonList WHERE fkLessonUuid = 'L2') != (SELECT pkLessonListId FROM tblLessonList WHERE fkLessonUuid = 'L1')
	OR
		(SELECT fkChildId FROM tblLessonList WHERE fkLessonUuid = 'L2') != (SELECT pkLessonListId FROM tblLessonList WHERE fkLessonUuid = 'L3') ;

SELECT 'LessonListAfterInsert: Parent or child link broken'
	WHERE
		(SELECT fkParentId FROM tblLessonList WHERE fkLessonUuid = 'L3') != (SELECT pkLessonListId FROM tblLessonList WHERE fkLessonUuid = 'L2')
	OR
		(SELECT fkChildId FROM tblLessonList WHERE fkLessonUuid = 'L3') != (SELECT pkLessonListId FROM tblLessonList WHERE fkLessonUuid = 'L4') ;

SELECT 'LessonListAfterInsert: Parent or child link broken'
	WHERE
		(SELECT fkParentId FROM tblLessonList WHERE fkLessonUuid = 'L4') != (SELECT pkLessonListId FROM tblLessonList WHERE fkLessonUuid = 'L3')
	OR
		(SELECT fkChildId FROM tblLessonList WHERE fkLessonUuid = 'L4') != NULL;

SELECT 'LessonListAfterInsert: Parent or child link broken'
	WHERE (SELECT COUNT(*) FROM vLessonListForward WHERE fkCourseUuid = 'C1') != 4;

SELECT 'LessonListAfterInsert: Parent or child link broken'
	WHERE (SELECT COUNT(*) FROM vLessonListForward WHERE fkCourseUuid = 'C2') != 2;

-- TODO: Add a test for vLessons view
-- SELECT 'vLessons';
-- SELECT * FROM vLessons;
-- SELECT 'vLessons for C1';
-- SELECT * FROM vLessons WHERE pkCourseUuid = 'C1';
-- SELECT 'vLessons for C2';
-- SELECT * FROM vLessons WHERE pkCourseUuid = 'C2';

-- Test tblLessonLists insert triggers
BEGIN TRANSACTION;
SELECT('[1] SHOULD FAIL: ''Course already connected''');
INSERT INTO tblLessonList (fkCourseUuid, fkLessonUuid) VALUES ('C2', 'L1');
ROLLBACK;

BEGIN TRANSACTION;
-- Insert a new list head without passing a course UUID.
SELECT('[2] SHOULD FAIL: ''List head lacks course''');
INSERT INTO tblLessonList (fkLessonUuid) VALUES ('L1');
ROLLBACK;

BEGIN TRANSACTION;
INSERT INTO tblLessonList (fkCourseUuid,fkLessonUuid) VALUES ('C3', 'L1');
-- Adding a lesson and pass a different course UUID than its parent has.
SELECT('[3] SHOULD FAIL: ''Wrong course ID''');
INSERT INTO tblLessonList (fkCourseUuid,fkLessonUuid,fkParentId) VALUES
	('C2', 'L2', (SELECT last_insert_rowid() FROM tblLessonList));
ROLLBACK;

-- DELETE FROM tblCourse WHERE pkCourseUuid = 'C2';

-- Let the user practice
INSERT INTO tblStats VALUES (
	strftime('%Y-%m-%dT%H:%M:%f','now'),
	'TestUser1',
	strftime('%Y-%m-%dT%H:%M:%f','now','+5 minutes'),
	100,
	10,
	(SELECT pkLessonListId FROM tblLessonList WHERE fkCourseUuid = 'C1' AND fkLessonUuid = 'L1')
);
SELECT 'Stats: Wrong row count'
	WHERE (SELECT count(*) FROM tblStats) != 1;

-- Delete a course and check the delete cascades
BEGIN TRANSACTION;
DELETE FROM tblCourse WHERE pkCourseUuid = 'C1';
SELECT 'Course: Wrong row count'
	WHERE (SELECT count(*) FROM tblCourse) != 2;
SELECT 'LessonList: Wrong row count'
	WHERE (SELECT count(*) FROM tblLessonList) != 2;
SELECT 'Lesson: Wrong row count'
	WHERE (SELECT count(*) FROM tblLesson) != 2;
SELECT 'Stats: Wrong row count'
	WHERE (SELECT count(*) FROM tblStats) != 0;
ROLLBACK;
