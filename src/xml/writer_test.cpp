/**
 * \file writer_test.cpp
 *
 * \date 09.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include <QtTest/QtTest>

#include "writer.hpp"
#include "parser.hpp"

namespace qtouch
{
namespace xml
{

const int TestcourseLessonCount = 2;
const QString OutputFile = QStringLiteral("testcourse_by_writer_test.xml");

class XmlWriterTest: public QObject
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
	//	void cleanup();

	void invalidFilePath();

	void writeTestCourse();

private:
	void verifyTestCourse(const CoursePtr& c);

	ValidatorPtr validator;
	CoursePtr testcourse;
};

void XmlWriterTest::verifyTestCourse(const CoursePtr& c)
{
	QCOMPARE(c->getId().toString(), QStringLiteral("{4e007d5e-613a-4a26-bff3-658d44d9cf10}"));
	QCOMPARE(c->getTitle(), QStringLiteral("TestCourse"));
	QCOMPARE(c->getDescription(),
	         QStringLiteral("This file is only for testing purposes and will be filtered by application."));
	QCOMPARE(c->isBuiltin(), true);

	// TODO: Add keyboard layout

	QVERIFY2(c->size() == TestcourseLessonCount, "Wrong lesson count");

	Course::const_iterator it = c->begin();

	QCOMPARE((*it)->getId().toString(), QStringLiteral("{d6e5a9a9-3c31-4175-8d58-245695c60b08}"));
	QCOMPARE((*it)->getTitle(), QStringLiteral("TestLesson1"));
	QCOMPARE((*it)->getNewChars(), QStringLiteral("fj"));
	QCOMPARE((*it)->getText(), QStringLiteral("fff jjj"));
	QCOMPARE((*it)->isBuiltin(), true);

	++it;

	QCOMPARE((*it)->getId().toString(), QStringLiteral("{e169c06b-71be-46df-97cf-8b3e68c5f75d}"));
	QCOMPARE((*it)->getTitle(), QStringLiteral("TestLesson2"));
	QCOMPARE((*it)->getNewChars(), QStringLiteral("dk"));
	QCOMPARE((*it)->getText(), QStringLiteral("ddd kkk"));
	QCOMPARE((*it)->isBuiltin(), true);
}

void XmlWriterTest::initTestCase()
{
	try
	{
		// Create a validator
		validator = createValidator(QStringLiteral(":/courses/course.xsd"));

		// Let the parser read the testcourse.xml
		ParseResult result;
		QString message;

		testcourse = parseCourse(QStringLiteral(":/testing/courses/testcourse.xml"), validator, &result, &message);

		if (Ok != result)
		{
			QFAIL(qPrintable(message));
		}

		// This should be unnecessary as long as the parser test runs before this test.
		verifyTestCourse(testcourse);
	}
	catch (Exception& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}
}

void XmlWriterTest::invalidFilePath()
{
	QVERIFY_EXCEPTION_THROWN(writeCourse(testcourse, QStringLiteral(":/testing/courses/FOOBAR.xml")), FileException);
}

void XmlWriterTest::writeTestCourse()
{
	try
	{
		if (QFile(OutputFile).exists())
		{
			if (!QFile(OutputFile).remove())
			{
				QFAIL(qPrintable(QStringLiteral("Unable to remove old test output file at ") % OutputFile));
			}
		}

		writeCourse(testcourse, OutputFile);

		// Let the parser read written data
		ParseResult result;
		QString message;

		CoursePtr readback = parseCourse(OutputFile, validator, &result, &message);

		if (!validate(OutputFile, validator))
		{
			QFAIL("Unable to validate written file");
		}

		// Verify the written data
		verifyTestCourse(readback);
	}
	catch (Exception& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}
}

} /* namespace xml */
} /* namespace qtouch */

QTEST_GUILESS_MAIN(qtouch::xml::XmlWriterTest)
#include "writer_test.moc"
