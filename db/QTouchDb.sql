-- Copyright (C) 2015  Moritz Nisblé <moritz.nisble@gmx.de>
--
-- This file is part of QTouch.
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 2 of the License, or
-- (at your option) any later version.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License along
-- with this program; if not, write to the Free Software Foundation, Inc.,
-- 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

PRAGMA user_version = 1;
PRAGMA foreign_keys = true;

DROP TRIGGER IF EXISTS LessonListBeforeDelete;
DROP TRIGGER IF EXISTS LessonListBeforeChildIdUpdate;
DROP TRIGGER IF EXISTS LessonListAfterInsertHead;
DROP TRIGGER IF EXISTS LessonListAfterInsert;
DROP TRIGGER IF EXISTS LessonListBeforeInsert;

DROP VIEW IF EXISTS vLessons;
DROP VIEW IF EXISTS vLessonListForward;
DROP VIEW IF EXISTS vDanglingLessons;

DROP TABLE IF EXISTS tblStats;
DROP TABLE IF EXISTS tblLessonList;
DROP TABLE IF EXISTS tblCourse;
DROP TABLE IF EXISTS tblLesson;
DROP TABLE IF EXISTS tblProfile;
DROP TABLE IF EXISTS tblMeta;

-- Key/Value table for meta information.
CREATE TABLE IF NOT EXISTS tblMeta (
	pkKey				TEXT NOT NULL PRIMARY KEY,
	cValue				TEXT
) WITHOUT ROWID;

-- User profiles.
CREATE TABLE IF NOT EXISTS tblProfile (
	pkProfileName		TEXT NOT NULL PRIMARY KEY,
	cSkillLevel			INTEGER NOT NULL DEFAULT 0
) WITHOUT ROWID;

-- Table that holds the lessons.
-- Each lesson has a UUID taken from Ktouch, stored as text and used as PK.
-- The builtin flag is used to destinguish between user defined/created lessons and those delivered with the binary.
-- Lessons that are not attached to any Course can be found with the help of vDanglingLessons.
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
CREATE TABLE IF NOT EXISTS tblCourse (
	pkCourseUuid		TEXT NOT NULL PRIMARY KEY,
	cCourseTitle		TEXT NOT NULL,
	cDescription		TEXT,
	cCourseBuiltin		NUMERIC NOT NULL DEFAULT 0
) WITHOUT ROWID;

-- For the user to be able to create custom courses from existing lessons a resolution table is needed (m:n).
-- This is implemented as a list, to be able to store the order of the lessons.
-- When either the course or the lesson is deleted, the corresponding entry in the list is also deleted.
--  The trigger LessonListBeforeDelete ensures correct list pointers on deletion in the middle of the list.
-- Using a artificial index for the recursion instead of the long UUIDs.
-- For the user to be able to ommit the CourseUUID on subsequent inserts, fkCourseUuid is not forced to be not null.
--  The handling is done in a trigger.
-- The unique constraint forbids adding the same lesson twice to one course list. That would make things much more
--  complicated since the application would have to be aware of the pkLessonListId to distinguish between the lessons.
--  Furthermore it makes no sense to add a lesson twice, since the user could simply replay the same lesson.
--  HINT: The GUI should make such a constallation impossible by using e.g. a drag and drop system.
CREATE TABLE IF NOT EXISTS tblLessonList (
	pkLessonListId		INTEGER PRIMARY KEY AUTOINCREMENT,
	fkCourseUuid		TEXT REFERENCES tblCourse(pkCourseUuid) ON UPDATE CASCADE ON DELETE CASCADE,
	fkLessonUuid		TEXT NOT NULL REFERENCES tblLesson(pkLessonUuid) ON UPDATE CASCADE ON DELETE CASCADE,
	-- List handling
	fkParentId			INTEGER DEFAULT NULL REFERENCES tblLessonList(pkLessonListId),
	fkChildId			INTEGER DEFAULT NULL REFERENCES tblLessonList(pkLessonListId),
	UNIQUE(fkCourseUuid,fkLessonUuid)
);

-- Ensure the list head is connected to a valid course before insert.
-- Ensure that a child does not specify a different course than its parent.
CREATE TRIGGER IF NOT EXISTS LessonListBeforeInsert BEFORE INSERT ON tblLessonList
BEGIN
	SELECT RAISE(ABORT, 'LessonList constraint failed: List head lacks course') FROM tblLessonList
		WHERE NEW.fkParentId ISNULL AND (SELECT pkCourseUuid FROM tblCourse WHERE pkCourseUuid = NEW.fkCourseUuid) ISNULL;

	SELECT RAISE(ABORT, 'LessonList constraint failed: Wrong course ID') FROM tblLessonList
		WHERE NEW.fkParentId NOTNULL AND (SELECT fkCourseUuid FROM tblLessonList WHERE pkLessonListId = NEW.fkParentId) != NEW.fkCourseUuid;
END;

-- If the user ommit the course UUID on subsequent inserts, set it automatically to the UUID of the parent
-- Update forward pointers after inserting a new lesson list entry that isn't the first one
CREATE TRIGGER IF NOT EXISTS LessonListAfterInsert AFTER INSERT ON tblLessonList
WHEN NEW.fkParentId IS NOT NULL
BEGIN
	-- Ensure that each entry is connected to its course
	UPDATE tblLessonList SET fkCourseUuid = (SELECT fkCourseUuid FROM tblLessonList WHERE pkLessonListId = NEW.fkParentId) WHERE fkCourseUuid ISNULL;

	-- Add forward pointer
	-- This will trigger LessonListBeforeChildIdUpdate with parent row as NEW
	UPDATE tblLessonList SET fkChildId = NEW.pkLessonListId WHERE pkLessonListId = NEW.fkParentId;
END;

-- If the user ommit the course UUID on subsequent inserts, set it automatically to the UUID of the parent
-- Update forward pointers after inserting a new lesson list head
CREATE TRIGGER IF NOT EXISTS LessonListAfterInsertHead AFTER INSERT ON tblLessonList
WHEN NEW.fkParentId IS NULL
BEGIN
	-- Ensure that each entry is connected to its course
	UPDATE tblLessonList SET fkCourseUuid = (SELECT fkCourseUuid FROM tblLessonList WHERE pkLessonListId = NEW.fkParentId) WHERE fkCourseUuid ISNULL;

	-- Update parent pointer of old head
	UPDATE tblLessonList SET fkParentId = NEW.pkLessonListId WHERE fkCourseUuid = NEW.fkCourseUuid AND fkParentId IS NULL AND pkLessonListId != NEW.pkLessonListId;

	-- Update the child pointer of the new head to point to the old head
	UPDATE tblLessonList SET fkChildId = (SELECT pkLessonListId FROM tblLessonList WHERE fkParentId = NEW.pkLessonListId) WHERE pkLessonListId = NEW.pkLessonListId;
END;

-- Correct the pointers when an update of fkChildId leads to multiple childs having the same parent
CREATE TRIGGER IF NOT EXISTS LessonListBeforeChildIdUpdate BEFORE UPDATE OF fkChildId ON tblLessonList
WHEN (SELECT count(*) FROM tblLessonList WHERE fkParentId = OLD.pkLessonListId) > 1
BEGIN
	-- Update parent pointer of the old child
	UPDATE tblLessonList SET fkParentId = NEW.fkChildId WHERE pkLessonListId = OLD.fkChildId;
	-- Update child pointer of the new child
	UPDATE tblLessonList SET fkChildId = OLD.fkChildId WHERE pkLessonListId = NEW.fkChildId;
END;

-- Correct the pointers when a Lesson is deleted from the List
CREATE TRIGGER IF NOT EXISTS LessonListBeforeDelete BEFORE DELETE ON tblLessonList
BEGIN
	-- Update child pointer of the old parent
	UPDATE tblLessonList SET fkChildId = OLD.fkChildId WHERE pkLessonListId = OLD.fkParentId;
	-- Update parent pointer of the old child
	UPDATE tblLessonList SET fkParentId = OLD.fkParentId WHERE pkLessonListId = OLD.fkChildId;
END;

-- Search for dangling Lessons
CREATE VIEW IF NOT EXISTS vDanglingLessons AS
SELECT pkLessonUuid FROM tblLesson WHERE pkLessonUuid NOT IN  (SELECT fkLessonUuid FROM tblLessonList);

-- Recursive scanning of the LessonList
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

-- Combine Courses and its Lessons in the correct order
CREATE VIEW IF NOT EXISTS vLessons AS
SELECT pkCourseUuid,
	cCourseTitle,
	cDescription,
	cCourseBuiltin,
	pkLessonUuid,
	cLessonTitle,
	cNewChars,
	cLessonBuiltin,
	cText,
	pkLessonListId
FROM (
	SELECT tblLesson.*,fkCourseUuid,pkLessonListId
	FROM tblLesson JOIN vLessonListForward ON pkLessonUuid = fkLessonUuid) AS Lesson
JOIN tblCourse ON pkCourseUuid = fkCourseUuid;

-- Table for statistics of passed lessons.
-- Has a composite PK of the lesson UUID, the profile name and the start date.
-- DateTimes are stored in ISO format (e.g. 2015-07-01T14:44:52+0200 or higher precision).
-- The stats does not depend on the LessonList entries anymore. That has the advantage that LessonList entries
--	can be deleted and stored in another order without invalidating the stats.
-- When the profile or the lesson is deleted, the stats are deleted too.
CREATE TABLE IF NOT EXISTS tblStats (
	pkfkLessonListId	INTEGER NOT NULL REFERENCES tblLessonList(pkLessonListId) ON UPDATE CASCADE ON DELETE CASCADE,
	pkfkProfileName		TEXT NOT NULL REFERENCES tblProfile(pkProfileName) ON UPDATE CASCADE ON DELETE CASCADE,
	pkStartDateTime		TEXT NOT NULL,
	cTime				INTEGER NOT NULL,
	cCharCount			INTEGER NOT NULL,
	cErrorCount			INTEGER,
	PRIMARY KEY(pkfkLessonListId, pkfkProfileName, pkStartDateTime)
) WITHOUT ROWID;


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

BEGIN TRANSACTION;
SELECT('[1] SHOULD FAIL: ''UNIQUE constraint failed''');
INSERT INTO tblLessonList (fkLessonUuid,fkParentId) VALUES ('L4', (SELECT last_insert_rowid() FROM tblLessonList WHERE fkLessonUuid = 'L2'));
ROLLBACK;

-- With the help of the inser trigger we should now have:
-- L1 <-> L2 <-> L3 <-> L4
-- L2 <-> L4

-- Check the links of Course C1
SELECT 'LessonListAfterInsert: Parent or child link broken'
	-- L1
	WHERE (SELECT fkParentId FROM tblLessonList WHERE fkCourseUuid = 'C1' AND fkLessonUuid = 'L1') IS NOT NULL
	OR (SELECT fkChildId FROM tblLessonList WHERE fkCourseUuid = 'C1' AND fkLessonUuid = 'L1') != (SELECT pkLessonListId FROM tblLessonList WHERE fkCourseUuid = 'C1' AND fkLessonUuid = 'L2')
	-- L2
	OR (SELECT fkParentId FROM tblLessonList WHERE fkCourseUuid = 'C1' AND fkLessonUuid = 'L2') != (SELECT pkLessonListId FROM tblLessonList WHERE fkCourseUuid = 'C1' AND fkLessonUuid = 'L1')
	OR (SELECT fkChildId FROM tblLessonList WHERE fkCourseUuid = 'C1' AND fkLessonUuid = 'L2') != (SELECT pkLessonListId FROM tblLessonList WHERE fkCourseUuid = 'C1' AND fkLessonUuid = 'L3')
	-- L3
	OR (SELECT fkParentId FROM tblLessonList WHERE fkCourseUuid = 'C1' AND fkLessonUuid = 'L3') != (SELECT pkLessonListId FROM tblLessonList WHERE fkCourseUuid = 'C1' AND fkLessonUuid = 'L2')
	OR (SELECT fkChildId FROM tblLessonList WHERE fkCourseUuid = 'C1' AND fkLessonUuid = 'L3') != (SELECT pkLessonListId FROM tblLessonList WHERE fkCourseUuid = 'C1' AND fkLessonUuid = 'L4')
	-- L4
	OR (SELECT fkParentId FROM tblLessonList WHERE fkCourseUuid = 'C1' AND fkLessonUuid = 'L4') != (SELECT pkLessonListId FROM tblLessonList WHERE fkCourseUuid = 'C1' AND fkLessonUuid = 'L3')
	OR (SELECT fkChildId FROM tblLessonList WHERE fkCourseUuid = 'C1' AND fkLessonUuid = 'L4') IS NOT NULL;

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
	(SELECT pkLessonListId FROM tblLessonList WHERE fkCourseUuid = 'C1' AND fkLessonUuid = 'L1'),
	'TestUser1',
	strftime('%Y-%m-%dT%H:%M:%f','now'),
	60000,
	220,
	10
), (
	(SELECT pkLessonListId FROM tblLessonList WHERE fkCourseUuid = 'C1' AND fkLessonUuid = 'L1'),
	'TestUser1',
	strftime('%Y-%m-%dT%H:%M:%f','now','+1 minutes'),
	60000,
	240,
	12
), (
	(SELECT pkLessonListId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L2'),
	'TestUser1',
	strftime('%Y-%m-%dT%H:%M:%f','now','+2 minutes'),
	60000,
	210,
	16
);
SELECT 'Stats: Wrong row count'
	WHERE (SELECT count(*) FROM tblStats) != 3;

-- HINT: Get stats with course and lesson titles
--SELECT * FROM tblStats JOIN vLessons ON pkLessonListId = pkfkLessonListId WHERE pkfkProfileName = 'TestUser1';

-- Delete a course and check the delete cascades
BEGIN TRANSACTION;
DELETE FROM tblCourse WHERE pkCourseUuid = 'C1';
SELECT 'Course: Wrong row count'
	WHERE (SELECT count(*) FROM tblCourse) != 2;
SELECT 'LessonList: Wrong row count'
	WHERE (SELECT count(*) FROM tblLessonList) != 2;
SELECT 'Stats: Wrong row count'
	WHERE (SELECT count(*) FROM tblStats) != 1;
-- Delete dangling Lessons
DELETE FROM tblLesson WHERE pkLessonUuid IN (SELECT pkLessonUuid FROM vDanglingLessons);
SELECT 'Lesson: Wrong row count'
	WHERE (SELECT count(*) FROM tblLesson) != 2;
SELECT 'Stats: Wrong row count'
	WHERE (SELECT count(*) FROM tblStats) != 1;
ROLLBACK;

-- Check updates
BEGIN TRANSACTION;

-- Update CourseTitle
UPDATE tblCourse SET cCourseTitle = 'Course 1 Updated' WHERE pkCourseUuid = 'C1';
-- Check row count
SELECT 'Course: Wrong row count'
	WHERE (SELECT COUNT(*) FROM tblCourse) != 3;
-- Check CourseTitle
SELECT printf('Course: cCourseTitle not updated. (still %Q)', cCourseTitle) FROM tblCourse
	WHERE pkCourseUuid = 'C1' AND cCourseTitle != 'Course 1 Updated';
-- Check update of LessonList
SELECT 'Course: tblLessonList not updated correctly while updating cCourseTitle'
	WHERE (SELECT count(*) FROM vLessons WHERE pkCourseUuid = 'C1' AND cCourseTitle = 'Course 1 Updated') != 4;

-- Update CourseUuid (PK)
UPDATE tblCourse SET pkCourseUuid = 'C1U' WHERE pkCourseUuid = 'C1';
-- Check row count
SELECT 'Course: Wrong row count'
	WHERE (SELECT COUNT(*) FROM tblCourse) != 3;
-- Check CourseUuid
SELECT 'Course: pkCourseUuid not updated'
	WHERE (SELECT count(*) FROM tblCourse WHERE pkCourseUuid != 'C1U' AND cCourseTitle = 'Course 1 Updated') != 0;
-- Check update of LessonList
SELECT 'Course: tblLessonList not updated correctly while updating pkCourseUuid'
	WHERE (SELECT count(*) FROM vLessons WHERE pkCourseUuid = 'C1U' AND cCourseTitle = 'Course 1 Updated') != 4;

-- Update LessonTitle
UPDATE tblLesson SET cLessonTitle = 'Lesson 2 Updated' WHERE pkLessonUuid = 'L2';
-- Check row count
SELECT 'Lesson: Wrong row count'
	WHERE (SELECT COUNT(*) FROM tblLesson) != 4;
-- Check LessonTitle
SELECT printf('Lesson: cLessonTitle not updated. (still %Q)', cLessonTitle) FROM tblLesson
	WHERE pkLessonUuid = 'L2' AND cLessonTitle != 'Lesson 2 Updated';
-- Check update of LessonList
SELECT 'Lesson: tblLessonList not updated correctly while updating cLessonTitle'
	WHERE (SELECT count(*) FROM vLessons WHERE pkLessonUuid = 'L2' AND cLessonTitle = 'Lesson 2 Updated') != 2;

-- Update LessonUuid (PK)
UPDATE tblLesson SET pkLessonUuid = 'L2U' WHERE pkLessonUuid = 'L2';
-- Check row count
SELECT 'Lesson: Wrong row count'
	WHERE (SELECT COUNT(*) FROM tblLesson) != 4;
-- Check LessonUuid
SELECT 'Lesson: pkLessonUuid not updated'
	WHERE (SELECT count(*) FROM tblLesson WHERE pkLessonUuid != 'L2U' AND cLessonTitle = 'Lesson 2 Updated') != 0;
-- Check update of LessonList
SELECT 'Lesson: tblLessonList not updated correctly while updating pkLessonUuid'
	WHERE (SELECT count(*) FROM vLessons WHERE pkLessonUuid = 'L2U' AND cLessonTitle = 'Lesson 2 Updated') != 2;

ROLLBACK;


-- Manipulate LessonList
-- Insert L3 between L2 and L4 into Course C2
-- L2 <-> L3 <-> L4
INSERT INTO tblLessonList (fkLessonUuid,fkParentId) VALUES ('L3', (SELECT pkLessonListId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L2'));
-- Check new row count
SELECT 'LessonListBeforeChildIdUpdate: Parent or child link broken'
	WHERE (SELECT COUNT(*) FROM vLessonListForward WHERE fkCourseUuid = 'C2') != 3;
-- Check the links
SELECT 'LessonListBeforeChildIdUpdate: Parent or child links broken'
	-- L2 links
	WHERE (SELECT fkParentId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L2') IS NOT NULL
	OR (SELECT fkChildId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L2') != (SELECT pkLessonListId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L3')
	-- L3 links
	OR (SELECT fkParentId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L3') != (SELECT pkLessonListId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L2')
	OR (SELECT fkChildId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L3') != (SELECT pkLessonListId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L4')
	-- L4 links
	OR (SELECT fkParentId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L4') != (SELECT pkLessonListId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L3')
	OR (SELECT fkChildId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L4') IS NOT NULL;

-- Delete L3 between L2 and L4 from Course C2
-- L2 <-> L4
DELETE FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L3';
-- Check new row count
SELECT 'LessonListBeforeChildIdUpdate: Parent or child link broken'
	WHERE (SELECT COUNT(*) FROM vLessonListForward WHERE fkCourseUuid = 'C2') != 2;
-- Check the links
SELECT 'LessonListBeforeChildIdUpdate: Parent or child links broken'
	-- L2 links
	WHERE (SELECT fkParentId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L2') IS NOT NULL
	OR (SELECT fkChildId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L2') != (SELECT pkLessonListId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L4')
	-- L4 links
	OR (SELECT fkParentId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L4') != (SELECT pkLessonListId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L2')
	OR (SELECT fkChildId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L4') IS NOT NULL;

-- Delete L4 from Course C2
DELETE FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L4';
-- Check new row count
SELECT 'LessonListBeforeChildIdUpdate: Parent or child link broken'
	WHERE (SELECT COUNT(*) FROM vLessonListForward WHERE fkCourseUuid = 'C2') != 1;
-- Check the links
SELECT 'LessonListBeforeChildIdUpdate: Parent or child links broken'
	-- L2 links
	WHERE (SELECT fkParentId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L2') IS NOT NULL
	OR (SELECT fkChildId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L2') IS NOT NULL;

-- Prepend L4 before L2 into Course C2
-- L4 <-> L2
INSERT INTO tblLessonList (fkLessonUuid,fkCourseUuid) VALUES ('L4','C2');
-- Insert L3 between L4 and L2 into Course C2
-- L4 <-> L3 <-> L2
INSERT INTO tblLessonList (fkLessonUuid,fkParentId) VALUES ('L3', (SELECT pkLessonListId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L4'));
-- Check new row count
SELECT 'LessonListBeforeChildIdUpdate: Parent or child link broken'
	WHERE (SELECT COUNT(*) FROM vLessonListForward WHERE fkCourseUuid = 'C2') != 3;
-- Check the links
SELECT 'LessonListBeforeChildIdUpdate: Parent or child links broken'
	-- L4 links
	WHERE (SELECT fkParentId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L4') IS NOT NULL
	OR (SELECT fkChildId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L4') != (SELECT pkLessonListId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L3')
	-- L3 links
	OR (SELECT fkParentId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L3') != (SELECT pkLessonListId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L4')
	OR (SELECT fkChildId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L3') != (SELECT pkLessonListId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L2')
	-- L4 links
	OR (SELECT fkParentId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L2') != (SELECT pkLessonListId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L3')
	OR (SELECT fkChildId FROM tblLessonList WHERE fkCourseUuid = 'C2' AND fkLessonUuid = 'L2') IS NOT NULL;
