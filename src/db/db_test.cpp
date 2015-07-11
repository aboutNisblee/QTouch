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

private:
	QScopedPointer<Db> db;

	xml::ValidatorPtr validator;

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
	catch (DatabaseException& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}
}

void DbTest::noPath()
{
	QVERIFY_EXCEPTION_THROWN(db->open(QString()), DatabaseException);
}

void DbTest::invalidPath()
{
	QVERIFY_EXCEPTION_THROWN(db->open(QStringLiteral("\\InvalidPath/")), DatabaseException);

	// Force recreation
	reset();
}

void DbTest::insertCourse()
{
	CoursePtr course;
	xml::ParseResult pr;
	QString msg;

	try
	{
		// Parse the test course
		course = xml::parseCourse(QStringLiteral(":/testing/courses/testcourse.xml"), validator, &pr, &msg);

		if (pr != xml::Ok)
		{
			qWarning() << "Result:" << pr << " " << msg.toLatin1().data();
		}
	}
	catch (Exception& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}

	try
	{
		// and add it to the database
		db->insert(course);
	}
	catch (DatabaseException const& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}

	// A second insert of the same course should trigger a unique constraint violation.
	QVERIFY_EXCEPTION_THROWN(db->insert(course), SqlException);

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

	CourseList clSourse;
	CourseList clTarget;

	xml::ParseResult result;
	QString message;

	// Parse all courses and add it to a list
	QStringListIterator it(sourceFileList);
	while (it.hasNext())
	{
		CoursePtr course;
		try
		{
			course = xml::parseCourse(it.next(), validator, &result, &message);
			if (result != xml::Ok)
			{
				qWarning() << "Result:" << result << " " << message.toLatin1().data();
			}
			clSourse.append(course);
		}
		catch (Exception& e)
		{
			QFAIL(qUtf8Printable(e.message()));
		}
	}

	// The count of found course files should match the parsed courses
	QVERIFY(sourceFileList.size() == clSourse.length());

	try
	{
		// Insert all courses
		db->insert(clSourse);

		// Read the courses back from the database
		clTarget = db->courses(Db::ALL);

		QVERIFY(clTarget.size() == clSourse.length());

		for (CourseList::iterator clTarget_iter = clTarget.begin(); clTarget_iter != clTarget.end(); ++clTarget_iter)
		{
			LessonList lessons = db->lessons(*clTarget_iter);

			// Add lessons to course
			(*clTarget_iter)->replace(lessons);
		}
	}
	catch (DatabaseException& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}

	// Sort source and target by ascending name
	std::sort(clSourse.begin(), clSourse.end(), CourseListAscTitle());
	std::sort(clTarget.begin(), clTarget.end(), CourseListAscTitle());

	// and compare the hashes
	QCOMPARE(Course::hash(clTarget), Course::hash(clSourse));
}

} /* namespace qtouch */

QTEST_GUILESS_MAIN(qtouch::DbTest)
#include "db_test.moc"
