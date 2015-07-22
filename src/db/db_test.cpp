/**
 * \file db_test.cpp
 *
 * \date 06.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include <QtTest/QtTest>

#include <algorithm>

#include "db.hpp"
#include "xml/parser.hpp"

namespace qtouch
{

class DbTest: public QObject
{
	Q_OBJECT

private slots:
	// will be called before the first test function is executed
	void initTestCase();
	//  will be called after the last test function was executed.
	//	void cleanupTestCase();
	// will be called before each test function is executed.
	//	void init();
	//  will be called after every test function.
	void cleanup();

	void noPath();
	void invalidPath();

	void insertCourse();
	void insertCourses();
	void insertCustomCourse();

	void insertProfile();
	void insertStats();

	void updateCourse();

private:
	QScopedPointer<Db> db;

	std::unique_ptr<QXmlSchemaValidator>  validator;

	void reset();
};

void DbTest::initTestCase()
{
	// Init with an empty path
	db.reset(new Db(QString()));

	try
	{
		validator = xml::createValidator(QStringLiteral(":/courses/course.xsd"));
	}
	catch (Exception& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}
}

/* Closes Db after each test! */
void DbTest::cleanup()
{
	db->close();
}

/* Force recreation */
void DbTest::reset()
{
	try
	{
		db->open(QStringLiteral("TestDb.sqlite"), true);
	}
	catch (DbException& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}
}

void DbTest::noPath()
{
	QVERIFY_EXCEPTION_THROWN(db->open(QString()), DbException);
}

void DbTest::invalidPath()
{
	QVERIFY_EXCEPTION_THROWN(db->open(QStringLiteral("\\InvalidPath/")), DbException);

	// Force recreation
	reset();
}

void DbTest::insertCourse()
{
	std::shared_ptr<const Course> source;
	std::shared_ptr<Course> target;

	xml::ParseResult pr;
	QString msg;

	try
	{
		// Parse the test course
		source = xml::parseCourse(QStringLiteral(":/testing/courses/testcourse.xml"), *validator, &pr, &msg);

		if (pr != xml::Ok)
		{
			qWarning() << "Result:" << pr << " " << msg.toLatin1().data();
		}
		else
		{
			// and add it to the database
			db->insert(*source);

			// Read the course back from the database
			auto cl = db->courses(Db::All);
			QVERIFY(1 == cl.size());
			target = cl.front();

			// Add lessons to course
			db->replaceLessons(*target);
		}
	}
	catch (Exception& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}

	// Compare the hashes
	QCOMPARE(*target, *source);

	// A second insert of the same course should trigger a unique constraint violation.
	QVERIFY_EXCEPTION_THROWN(db->insert(*source), DbException);

	// Force recreation
	reset();
}

/* Insert course on closed Db. Automatic opening tested. */
void DbTest::insertCourses()
{
	// Get all course files from the resources
	QDir coursepath(QStringLiteral(":/courses"), "*.xml", QDir::Name | QDir::IgnoreCase, QDir::Files);
	QStringList sourceFileList;
	foreach(QString const & s, coursepath.entryList())
	{
		sourceFileList.append(coursepath.filePath(s));
	}

	std::vector<std::shared_ptr<const Course>> clSource;
	std::vector<std::shared_ptr<Course>> clTarget;

	xml::ParseResult result;
	QString message;

	// Parse all courses and add it to a list
	QStringListIterator it(sourceFileList);
	while (it.hasNext())
	{
		try
		{
			auto course = xml::parseCourse(it.next(), *validator, &result, &message);
			if (result != xml::Ok)
			{
				qWarning() << "Result:" << result << " " << message.toLatin1().data();
			}
			clSource.push_back(course);
		}
		catch (Exception& e)
		{
			QFAIL(qUtf8Printable(e.message()));
		}
	}

	// The count of found course files should match the parsed courses
	QVERIFY(static_cast<Course::size_type>(sourceFileList.size()) == clSource.size());

	try
	{
		// Insert all courses
		db->insert(clSource);

		// Read the courses back from the database
		clTarget = db->courses(Db::BuiltIn);

		QVERIFY(clTarget.size() == clSource.size());

		for (auto& clTarget_iter : clTarget)
		{
			auto lessons = db->lessons(clTarget_iter->getId());

			// Add lessons to course
			clTarget_iter->clear();
			clTarget_iter->insert(clTarget_iter->begin(), lessons.begin(), lessons.end());
		}
	}
	catch (DbException& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}

	// Sort source and target by ascending name
	std::sort(clSource.begin(), clSource.end(), CourseListAscTitle());
	std::sort(clTarget.begin(), clTarget.end(), CourseListAscTitle());

	// and compare the hashes
	QCOMPARE(hash(clTarget.begin(), clTarget.end()), hash(clSource.begin(), clSource.end()));
}

void DbTest::insertCustomCourse()
{
	std::vector<std::shared_ptr<Course>> builtin;

	// Create a custom course
	auto course = Course::create();
	course->setId(QUuid());
	course->setTitle("Custom Course");
	course->setBuiltin(false);

	// And a custom lLesson
	Lesson lesson;
	lesson.setId(QUuid());
	lesson.setTitle("Custom Lesson");
	lesson.setBuiltin(false);

	// Add the custom lesson to the course
	course->push_back(lesson);

	try
	{
		// Read builtin courses from Db
		auto builtinList = db->courses(Db::BuiltIn);
		QVERIFY(!builtinList.empty());

		// Get the first builtin course
		auto firstCoursePtr = *builtinList.begin();
		QVERIFY(firstCoursePtr.get() != 0);

		// Read the lessons of the first builtin course
		auto lessonList = db->lessons(firstCoursePtr->getId());
		QVERIFY(!lessonList.empty());

		// Get the first lesson from the lesson list of the first course
		auto firstLessonPtr = *lessonList.begin();
		QVERIFY(firstLessonPtr.get() != 0);

		// And append to our custom Course
		course->push_back(*firstLessonPtr);

		// Insert the custom Course
		db->insert(*course);

		// Read it back
		auto customCourseList = db->courses(Db::Custom);

		// Check custom course count is 1
		QVERIFY(1 == customCourseList.size());

		// Load the lessons for the custom course
		db->replaceLessons(*customCourseList.at(0));

		// Check lesson count is 2
		QVERIFY(2 == customCourseList.at(0)->size());

		// and compare the hashes
		QCOMPARE(*course, *customCourseList.at(0));
	}
	catch (DbException const& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}
}

void DbTest::insertProfile()
{
	Profile pSource("TestUser1", Profile::Advanced);

	try
	{
		// Insert
		db->insert(pSource);

		// Read the profile back from the database
		auto pTargetList = db->profiles();

		QVERIFY(pTargetList.size() == 1);

		QCOMPARE(pTargetList.at(0)->getName(), pSource.getName());
		QCOMPARE(pTargetList.at(0)->getSkillLevel(), pSource.getSkillLevel());
	}
	catch (DbException& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}
}

void DbTest::insertStats()
{
	QDateTime start = QDateTime::currentDateTime();
	QDateTime end = QDateTime::currentDateTime().addSecs(60);
	quint32 chars = 100;
	quint32 errors = 5;

	try
	{
		// Read profiles use first
		auto profileList = db->profiles();
		QVERIFY(profileList.size() >= 1);
		auto profilePtr = profileList.at(0);
		QVERIFY(profilePtr.get() != 0);

		// Read Courses use first
		auto courseList = db->courses();
		QVERIFY(courseList.size() >= 1);
		auto coursePtr = courseList.at(0);
		QVERIFY(coursePtr.get() != 0);

		// Read Lessons and add them to the course
		db->replaceLessons(*coursePtr);
		QVERIFY(coursePtr->size() >= 1);

		// Take the first lesson
		auto lessonPtr = coursePtr->at(0);
		QVERIFY(lessonPtr.get() != 0);

		// Create a stats object
		Stats stats(lessonPtr->getId(), profilePtr->getName(), start);
		stats.setEnd(end);
		stats.setCharCount(chars);
		stats.setErrorCount(errors);

		// Insert it
		db->insert(stats);

		// Read it back
		auto statsList = db->stats(profilePtr->getName());

		QVERIFY(1 == statsList.size());

		auto result = statsList.at(0);
		QVERIFY(result.get() != 0);

		QCOMPARE(result->getStart(), start);
		QCOMPARE(result->getProfileName(), profilePtr->getName());
		QCOMPARE(result->getEnd(), end);
		QCOMPARE(result->getCharCount(), chars);
		QCOMPARE(result->getErrorCount(), errors);
		QCOMPARE(result->getLessonId(), lessonPtr->getId());
	}
	catch (DbException& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}
}

/* NOTE: This test currently depends on the Course added by insertCustomCourse!
 * So don't reset the Db in any test before! */
void DbTest::updateCourse()
{
	std::shared_ptr<Course> org;
	std::shared_ptr<Course> copy;
	std::shared_ptr<Course> readBack;

	try
	{
		// Read the custom courses from Db
		auto cl = db->courses(Db::Custom);
		QVERIFY(1 == cl.size());
		org = cl.front();

		// Fill it with its Lessons
		db->replaceLessons(*org);
	}
	catch (DbException const& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}

	// Make a copy
	copy = Course::clone(*org);

	// Manipulate the Course
	copy->setTitle("Updated Custom Course");

	// And a custom lesson
	Lesson lesson;
	lesson.setId(QUuid());
	lesson.setTitle("Custom Lesson");
	lesson.setBuiltin(false);

	// Add the custom lesson to the course
	copy->push_back(lesson);

	try
	{
		// Update the course
		db->update(*copy);

		// Read it back
		auto cl = db->courses(Db::Custom);
		QVERIFY(1 == cl.size());
		readBack = cl.front();

		// Fill it with its Lessons
		db->replaceLessons(*readBack);
	}
	catch (DbException const& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}

	// Compare the hashes
	QCOMPARE(*readBack, *copy);
}

} /* namespace qtouch */

QTEST_GUILESS_MAIN(qtouch::DbTest)
#include "db_test.moc"
