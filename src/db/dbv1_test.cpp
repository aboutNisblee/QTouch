/**
 * \file dbv1_test.cpp
 *
 * \date 31.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include <QtTest/QtTest>
#include <sqlite3.h>

#include "dbv1.hpp"
#include "xml/parser.hpp"

namespace qtouch
{

class DbV1Test: public QObject
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

	void noPathTest();
	void invalidPathTest();

	void insertCourseTest();
	void insertCoursesTest();
	void insertCustomCourseTest();

	void insertProfileTest();
	void insertStatsTest();

	void updateCourseTest();

private:
	void open();
	void reset();
	void insertCourse(const Course& course);
	void getCoursesAndLessons(Db::CourseType type, std::insert_iterator<std::vector<std::shared_ptr<Course>>>);

	std::unique_ptr<DbV1> db;
	std::unique_ptr<QXmlSchemaValidator>  validator;
};

void DbV1Test::initTestCase()
{
	db = DbV1::create();

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
void DbV1Test::cleanup()
{
	db->close();
}

void DbV1Test::open()
{
	try
	{
		db->open(QStringLiteral("TestDb.sqlite"));
	}
	catch (DbException& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}
}

/* Force recreation */
void DbV1Test::reset()
{
	try
	{
		open();
		db->dropSchema();
		db->createSchema();
	}
	catch (DbException& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}
}

void DbV1Test::insertCourse(const Course& course)
{
	db->insert(course);

	int parentId = 0;
	for (const auto& l : course)
	{
		try
		{
			db->insert(*l);
		}
		catch (const DbException& e)
		{
			if (SQLITE_CONSTRAINT == e.sqlErrorCode())
				qDebug() << "INFO: Lesson already present:" << e.databaseMessage();
			else
				throw;
		}
		parentId = db->insert(course.getId(), l->getId(), parentId);
	}
}

void DbV1Test::getCoursesAndLessons(Db::CourseType type,
                                    std::insert_iterator<std::vector<std::shared_ptr<Course>>> result)
{
	auto qC = db->selectCourses(type);

	/* NOTE: It's not possible to have any tests in functions with a return value!
	 * This ... macros are using return to leave the function instead of an exception! */
	QVERIFY(true == qC.next());

	do
	{
		auto target = Course::create();
		// pkCourseUuid, cCourseTitle, cDescription, cCourseBuiltin
		target->setId(QUuid(qC.value("pkCourseUuid").toString()));
		target->setTitle(qC.value("cCourseTitle").toString());
		target->setDescription(qC.value("cDescription").toString());
		target->setBuiltin(qC.value("cCourseBuiltin").toBool());

		// Add lessons to course
		auto qL = db->selectLessonList(target->getId());

		QVERIFY(true == qL.next());

		do
		{
			Lesson lesson;
			// pkLessonUuid, cLessonTitle, cNewChars, cLessonBuiltin, cText
			lesson.setId(QUuid(qL.value("pkLessonUuid").toString()));
			lesson.setTitle(qL.value("cLessonTitle").toString());
			lesson.setNewChars(qL.value("cNewChars").toString());
			lesson.setBuiltin(qL.value("cLessonBuiltin").toBool());
			lesson.setText(qL.value("cText").toString());

			target->push_back(lesson);
		}
		while (qL.next());

		*result = target;
		++result;
	}
	while (qC.next());
}

void DbV1Test::noPathTest()
{
	QVERIFY_EXCEPTION_THROWN(db->open(QString()), DbException);
}

void DbV1Test::invalidPathTest()
{
	QVERIFY_EXCEPTION_THROWN(db->open(QStringLiteral("\\InvalidPath/")), DbException);

	reset();
}

void DbV1Test::insertCourseTest()
{
	std::shared_ptr<const Course> source;
	std::shared_ptr<Course> target;

	xml::ParseResult pr;
	QString msg;

	open();

	try
	{
		// Parse the test course
		source = xml::parseCourse(QStringLiteral(":/testing/courses/testcourse.xml"), *validator, &pr, &msg);

		if (pr != xml::Ok)
		{
			QFAIL(qUtf8Printable(QString("Result: %i %s").arg(pr).arg(msg)));
		}
		else
		{
			// and add it to the database
			db->begin_transaction();

			insertCourse(*source);

			db->end_transaction();

			// Read the course back from the database
			std::vector<std::shared_ptr<Course>> list;
			getCoursesAndLessons(Db::All, std::inserter(list, list.begin()));
			QVERIFY(1 == list.size());
			target = *list.begin();
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
void DbV1Test::insertCoursesTest()
{
	open();

	// Get all course files from the resources
	QDir coursepath(QStringLiteral(":/courses"), "*.xml", QDir::Name | QDir::IgnoreCase, QDir::Files);
	QStringList sourceFileList;
	foreach(QString const & s, coursepath.entryList())
	{
		sourceFileList.append(coursepath.filePath(s));
	}

	std::vector<std::shared_ptr<const Course>> clSource;
	std::vector<std::shared_ptr<Course>> clTarget;

	xml::ParseResult pr;
	QString msg;

	// Parse all courses and add it to a list
	QStringListIterator it(sourceFileList);
	while (it.hasNext())
	{
		try
		{
			auto course = xml::parseCourse(it.next(), *validator, &pr, &msg);
			if (pr != xml::Ok)
			{
				QFAIL(qUtf8Printable(QString("Result: %i %s").arg(pr).arg(msg)));
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
		db->begin_transaction();
		for (const auto& c : clSource)
			insertCourse(*c);
		db->end_transaction();

		// Read the courses back from the database
		getCoursesAndLessons(Db::BuiltIn, std::inserter(clTarget, clTarget.begin()));
		QVERIFY(clTarget.size() == clSource.size());
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

void DbV1Test::insertCustomCourseTest()
{
	open();

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
		std::vector<std::shared_ptr<Course>> builtinList;
		getCoursesAndLessons(Db::BuiltIn, std::inserter(builtinList, builtinList.begin()));
		QVERIFY(!builtinList.empty());

		// Get the first builtin course
		auto firstCoursePtr = *builtinList.begin();
		QVERIFY(firstCoursePtr.get() != 0);
		QVERIFY(!firstCoursePtr->empty());

		// Take the lesson of the course and append it to out custom course
		course->push_back(*firstCoursePtr->at(0));

		// Insert the custom Course
		db->begin_transaction();
		insertCourse(*course);
		db->end_transaction();

		// Read it back
		std::vector<std::shared_ptr<Course>> customCourseList;
		getCoursesAndLessons(Db::Custom, std::inserter(customCourseList, customCourseList.begin()));

		// Check custom course count is 1
		QVERIFY(1 == customCourseList.size());

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

void DbV1Test::insertProfileTest()
{
	Profile pSource("TestUser1", Profile::Advanced);

	open();

	try
	{
		// Insert
		db->insert(pSource);

		// Read the profile back from the database
		// pkProfileName, cSkillLevel
		auto pQ = db->selectProfiles();
		QVERIFY(pQ.next() != false);

		Profile pTarget(pQ.value("pkProfileName").toString(),
		                static_cast<Profile::SkillLevel>(pQ.value("cSkillLevel").toInt()));

		QCOMPARE(pTarget.getName(), pSource.getName());
		QCOMPARE(pTarget.getSkillLevel(), pSource.getSkillLevel());
	}
	catch (DbException& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}
}

void DbV1Test::insertStatsTest()
{
	QDateTime start = QDateTime::currentDateTime();
	QDateTime end = QDateTime::currentDateTime().addSecs(60);
	quint32 chars = 100;
	quint32 errors = 5;

	open();

	try
	{
		// Read profiles
		// pkProfileName, cSkillLevel
		auto cP = db->selectProfiles();
		QVERIFY(cP.next() != false);

		// Read Courses
		// pkCourseUuid, cCourseTitle, cDescription, cCourseBuiltin
		auto cC = db->selectCourses(Db::All);
		QVERIFY(cC.next() != false);

		// Read the LessonList of the first Course
		// pkLessonUuid, cLessonTitle, cNewChars, cLessonBuiltin, cText
		auto cL = db->selectLessonList(QUuid(cC.value("pkCourseUuid").toString()));
		QVERIFY(cL.next() != false);

		/* Take the first lesson and create a stats object
		with the lessonId from the Db */
		Stats stats(QUuid(cL.value("pkLessonUuid").toString()), cP.value("pkProfileName").toString(), start);
		stats.setEnd(end);
		stats.setCharCount(chars);
		stats.setErrorCount(errors);

		// Insert it
		db->insert(stats);

		// Read it back
		// pkfkLessonUuid, cStartDateTime, cEndDateTime, cCharCount, cErrorCount
		auto cS = db->selectStats(cP.value("pkProfileName").toString());
		QVERIFY(cS.next() != false);

		QCOMPARE(QUuid(cS.value("pkfkLessonUuid").toString()), QUuid(cL.value("pkLessonUuid").toString()));
		QCOMPARE(cS.value("cStartDateTime").toDateTime(), start);
		QCOMPARE(cS.value("cEndDateTime").toDateTime(), end);
		QCOMPARE(cS.value("cCharCount").toUInt(), chars);
		QCOMPARE(cS.value("cErrorCount").toUInt(), errors);
	}
	catch (DbException& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}
}

/* NOTE: This test currently depends on the Course added by insertCustomCourse!
 * So don't reset the Db in any test before! */
void DbV1Test::updateCourseTest()
{
	std::shared_ptr<Course> org;
	std::shared_ptr<Course> copy;
	std::shared_ptr<Course> readBack;

	open();

	try
	{
		// Read the custom course from Db
		std::vector<std::shared_ptr<Course>> cl;
		getCoursesAndLessons(Db::Custom, std::inserter(cl, cl.begin()));
		QVERIFY(cl.size() >= 1);
		org = cl.front();
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
		db->begin_transaction();

		// Update the course
		db->update(*copy);

		db->deleteLessonList(copy->getId());

		int parentId = 0;
		for (const auto& l : *copy)
		{
			// Insert or update the Lesson
			auto cL = db->selectLesson(l->getId());
			if (cL.next())
				db->update(*l);
			else
				db->insert(*l);

			// Add LessonList entry
			parentId = db->insert(copy->getId(), l->getId(), parentId);
		}

		db->end_transaction();

		// Read it back
		std::vector<std::shared_ptr<Course>> cl;
		getCoursesAndLessons(Db::Custom, std::inserter(cl, cl.begin()));
		QVERIFY(cl.size() >= 1);
		readBack = cl.front();
	}
	catch (DbException const& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}

	// Compare the hashes
	QCOMPARE(*readBack, *copy);
}

} /* namespace qtouch */

QTEST_GUILESS_MAIN(qtouch::DbV1Test)
#include "dbv1_test.moc"
