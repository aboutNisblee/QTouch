PRAGMA user_version = 1;
--PRAGMA foreign_keys = true;

DROP TABLE IF EXISTS "Stats";
DROP TABLE IF EXISTS "CourseLessonResolve";
DROP TABLE IF EXISTS "Course";
DROP TABLE IF EXISTS "Lesson";
DROP TABLE IF EXISTS "Profile";

DROP VIEW IF EXISTS "LessonsOfCourse";

DROP TRIGGER IF EXISTS "StatsDateTimeCheck";

-- User profiles.
CREATE TABLE "Profile" (
	"name"				TEXT NOT NULL PRIMARY KEY,
	"skill_level"		INTEGER NOT NULL DEFAULT 0
) WITHOUT ROWID;

-- Table that holds the lessons.
-- Each lesson has a UUID taken from Ktouch, stored as text and used as PK.
-- The builtin flag is used to destinguish between user defined/created lessons and those delivered with the binary.
-- TODO: What about lessons that are not assigned to any course?
CREATE TABLE "Lesson" (
	"lesson_id"			TEXT NOT NULL PRIMARY KEY,
	"title"				TEXT NOT NULL,
	"new_characters"	TEXT,
	"builtin"			NUMERIC NOT NULL DEFAULT 0,
	"text"				TEXT
) WITHOUT ROWID;

-- A course is mainly a collection of lessons.
-- TODO: How to ensure the right order of lessons?
-- TODO: What about courses without any lesson?
CREATE TABLE "Course" (
	"course_id"			TEXT NOT NULL PRIMARY KEY,
	"title"				TEXT NOT NULL,
	"description"		TEXT,
	"builtin"			NUMERIC NOT NULL DEFAULT 0
) WITHOUT ROWID;

-- For the user to be able to create custom courses from existing lessons a resolution table is needed (m:n).
CREATE TABLE "CourseLessonResolve" (
	"fk_course_id"		TEXT NOT NULL REFERENCES Course("course_id"),
	"fk_lesson_id"		TEXT NOT NULL REFERENCES Lesson("lesson_id"),
	PRIMARY KEY("fk_course_id", "fk_lesson_id")
) WITHOUT ROWID;

-- Table for statistics of passed lessons.
-- Has a composite PK of start time of the lesson and the profile name.
-- DateTimes are stored in ISO format (e.g. 2015-07-01T14:44:52+0200 or hiogher precision).
-- Each entry holds a reference to the resolution table between course and lesson.
-- When the profile, course or lesson is deleted, the stats are deleted too.
CREATE TABLE "Stats" (
	"start_datetime"	TEXT NOT NULL,
	"fk_profile_name"	TEXT NOT NULL REFERENCES Profile("name") ON UPDATE CASCADE ON DELETE CASCADE,
	"end_datetime"		TEXT NOT NULL,
	"char_count"		INTEGER NOT NULL,
	"error_count"		INTEGER,
	"fk_course_id"		TEXT NOT NULL,
	"fk_lesson_id"		TEXT NOT NULL,
	PRIMARY KEY("start_datetime", "fk_profile_name"),
	FOREIGN KEY("fk_course_id", "fk_lesson_id") REFERENCES CourseLessonResolve("fk_course_id", "fk_lesson_id") ON UPDATE CASCADE ON DELETE CASCADE
) WITHOUT ROWID;

-- Check that Stats.end_datetime is bigger that Stats.start_datetime
CREATE TRIGGER "StatsDateTimeCheck" BEFORE INSERT ON "Stats"
WHEN strftime('%s',NEW."start_datetime") > strftime('%s',NEW."end_datetime")
BEGIN
	SELECT RAISE(ABORT, 'Datetime constraint violation: Start bigger than end time');
END;

-- Create a view that eases the handling of courses and lessons
CREATE VIEW "LessonsOfCourse" AS
SELECT "course_id",
	"Course"."title" AS "course_title",
	"description",
	"Course"."builtin" AS "course_builtin",
	"lesson_id",
	"Lesson"."title" AS "lesson_title",
	"new_characters",
	"Lesson"."builtin" AS "lesson_builtin",
	"text"
FROM (
	SELECT "Lesson".*,"fk_course_id"
	FROM "Lesson"
	JOIN "CourseLessonResolve" ON "lesson_id"=="fk_lesson_id") AS "Lesson"
JOIN "Course" ON "course_id"=="fk_course_id";

-- *****************************
-- TESTS
-- *****************************

BEGIN TRANSACTION;

-- Add some example users
INSERT INTO "Profile" VALUES ('TestUser1', 0);
INSERT INTO "Profile" VALUES ('TestUser2', 1);

SELECT 'Profile insert test failed' FROM "Profile"
	WHERE (SELECT COUNT(*) FROM "Profile") != 2;

-- Some courses
INSERT INTO "Course" VALUES ('88859dc2-383f-434d-bc6c-1e4ac7025308', 'Course 1', 'Description', 1);
INSERT INTO "Course" VALUES ('152b5fd8-b8ba-447a-b6e0-618fd7b07b3d', 'Course 2', 'Description', 0);

SELECT 'Course: Wrong row count' FROM "Course"
	WHERE (SELECT COUNT(*) FROM "Course") != 2;

-- Some lessons
INSERT INTO "Lesson" VALUES ('4e1d2bbf-af23-448e-a239-a9d4a87b4fc7', 'Lesson 1', 'f', 1, 'fff');
INSERT INTO "Lesson" VALUES ('33bfa3ca-63c0-431b-aac4-a6e28959f7e1', 'Lesson 2', 'j', 1, 'fff jjj');
INSERT INTO "Lesson" VALUES ('635672a9-2ed4-4808-ba5a-0f5903326f27', 'Lesson 3', 'd', 0, 'ddd');
INSERT INTO "Lesson" VALUES ('cb2519c1-0124-4dbe-87f6-19d43d66ab5e', 'Lesson 4', 'k', 0, 'ddd kkk');

SELECT 'Lesson: Wrong row count' FROM "Lesson"
	WHERE (SELECT COUNT(*) FROM "Lesson") != 4;

-- And connect courses and lessons
INSERT INTO "CourseLessonResolve" VALUES ((SELECT "course_id" FROM "Course" WHERE "title"=='Course 1'), (SELECT "lesson_id" FROM "Lesson" WHERE "title"=='Lesson 1'));
INSERT INTO "CourseLessonResolve" VALUES ((SELECT "course_id" FROM "Course" WHERE "title"=='Course 1'), (SELECT "lesson_id" FROM "Lesson" WHERE "title"=='Lesson 2'));

INSERT INTO "CourseLessonResolve" VALUES ((SELECT "course_id" FROM "Course" WHERE "title"=='Course 2'), (SELECT "lesson_id" FROM "Lesson" WHERE "title"=='Lesson 3'));
INSERT INTO "CourseLessonResolve" VALUES ((SELECT "course_id" FROM "Course" WHERE "title"=='Course 2'), (SELECT "lesson_id" FROM "Lesson" WHERE "title"=='Lesson 4'));

-- Builtin lesson 2 is linked into custom course 2
INSERT INTO "CourseLessonResolve" VALUES ((SELECT "course_id" FROM "Course" WHERE "title"=='Course 2'), (SELECT "lesson_id" FROM "Lesson" WHERE "title"=='Lesson 2'));

SELECT 'CourseLessonResolve: Wrong row count' FROM "CourseLessonResolve"
	WHERE (SELECT COUNT(*) FROM "CourseLessonResolve") != 5;

-- Check the count of lessons connected to the courses
SELECT 'LessonsOfCourse: Wrong row count' FROM "LessonsOfCourse"
	WHERE (SELECT COUNT(*)  FROM "LessonsOfCourse" WHERE "course_title"=='Course 1') != 2;

SELECT 'LessonsOfCourse: Wrong row count' FROM "LessonsOfCourse"
	WHERE (SELECT COUNT(*)  FROM "LessonsOfCourse" WHERE "course_title"=='Course 2') != 3;

-- And the count of courses connected to a lesson
SELECT 'LessonsOfCourse: Wrong row count' FROM "LessonsOfCourse"
	WHERE (SELECT COUNT(*)  FROM "LessonsOfCourse" WHERE "lesson_title"=='Lesson 2') != 2;

-- Let the user practice
INSERT INTO "Stats" VALUES (
	strftime('%Y-%m-%dT%H:%M:%f','now'),
	'TestUser1',
	strftime('%Y-%m-%dT%H:%M:%f','now','+5 minutes'),
	100,
	10,
	(SELECT "course_id" FROM "LessonsOfCourse" WHERE "course_title"=='Course 2'),
	(SELECT "lesson_id" FROM "LessonsOfCourse" WHERE "lesson_title"=='Lesson 2')
);

-- This should activate the StatsDateTimeCheck trigger
-- INSERT INTO "Stats" VALUES (
-- 	strftime('%Y-%m-%dT%H:%M:%f','now','+1 minutes'),
-- 	'TestUser1',
-- 	strftime('%Y-%m-%dT%H:%M:%f','now'),
-- 	100,
-- 	10,
-- 	(SELECT "course_id" FROM "LessonsOfCourse" WHERE "course_title"=='Course 2'),
-- 	(SELECT "lesson_id" FROM "LessonsOfCourse" WHERE "lesson_title"=='Lesson 2')
-- );

ROLLBACK;