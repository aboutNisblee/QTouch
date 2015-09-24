/* Copyright (C) 2015  Moritz Nisblé <moritz.nisble@gmx.de>
 *
 * This file is part of QTouch.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/**
 * \file courseparser_test.cpp
 *
 * \date 06.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include <QtTest/QtTest>
#include <QDir>

#include "parser.hpp"

namespace qtouch
{
namespace xml
{

const int TestcourseLessonCount = 2;

class XmlParserTest: public QObject
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

	void invalidSchemaFilePath();
	void invalidXmlFilePath();

	void invalidSchemaFile();

	void validation();
	void parseTestCourse();
	void parseCourses();

private:
	void verifyTestCourse(const Course& c);

	std::unique_ptr<QXmlSchemaValidator> validator;
};

void XmlParserTest::verifyTestCourse(const Course& c)
{
	QCOMPARE(c.getId().toString(), QStringLiteral("{4e007d5e-613a-4a26-bff3-658d44d9cf10}"));
	QCOMPARE(c.getTitle(), QStringLiteral("TestCourse"));
	QCOMPARE(c.getDescription(),
	         QStringLiteral("This file is only for testing purposes and will be filtered by application."));
	QCOMPARE(c.isBuiltin(), true);

	// TODO: Add keyboard layout

	QVERIFY2(c.size() == TestcourseLessonCount, "Wrong lesson count");

	auto it = c.begin();

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

void XmlParserTest::initTestCase()
{
	try
	{
		// Create a validator
		validator = createValidator(QStringLiteral(":/courses/course.xsd"));
	}
	catch (Exception& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}
}

void XmlParserTest::invalidSchemaFilePath()
{
	QVERIFY_EXCEPTION_THROWN(createValidator(QStringLiteral(":/testing/courses/FOOBAR.xsd")), FileException);
}

void XmlParserTest::invalidXmlFilePath()
{
	QVERIFY_EXCEPTION_THROWN(validate(QStringLiteral(":/testing/courses/FOOBAR.xml"), *validator),
	                         FileException);
}

void XmlParserTest::invalidSchemaFile()
{
	QVERIFY_EXCEPTION_THROWN(createValidator(QStringLiteral(":/testing/courses/testcourse.xml")), XmlException);
}

void XmlParserTest::validation()
{
	QCOMPARE(validate(QStringLiteral(":/testing/courses/testcourse.xml"), *validator), true);
}

void XmlParserTest::parseTestCourse()
{
	ParseResult result;
	QString message;

	try
	{
		auto course = parseCourse(QStringLiteral(":/testing/courses/testcourse.xml"), *validator, &result,
		                     &message);

		verifyTestCourse(*course);
	}
	catch (Exception& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}

}

void XmlParserTest::parseCourses()
{
	QDir coursepath(QStringLiteral(":/courses"), "*.xml", QDir::Name | QDir::IgnoreCase, QDir::Files);

	QStringList coursefiles;
	foreach(QString const & s, coursepath.entryList())
	{
		coursefiles.append(coursepath.filePath(s));
	}

	qDebug() << coursefiles;

	QList<std::shared_ptr<Course>> courseList;

	QStringListIterator it(coursefiles);
	while (it.hasNext())
	{
		ParseResult result;
		QString message;

		try
		{
			auto course = parseCourse(it.next(), *validator, &result, &message);

			if (result != Ok)
			{
				qWarning() << "Result:" << result << " " << message;
			}

			courseList.append(course);
		}
		catch (Exception& e)
		{
			QFAIL(qUtf8Printable(e.message()));
		}
	}

	QCOMPARE(coursepath.entryList().size(), courseList.length());
}

} /* namespace xml */
} /* namespace qtouch */

QTEST_GUILESS_MAIN(qtouch::xml::XmlParserTest)
#include "parser_test.moc"
