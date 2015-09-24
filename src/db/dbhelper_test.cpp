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
 * \file dbhelper_test.cpp
 *
 * \date 31.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include <QtTest/QtTest>
#include <sqlite3.h>

#include "dbhelper.hpp"
#include "dbv1.hpp"
#include "xml/parser.hpp"

namespace qtouch
{

class DbHelperTest: public QObject
{
	Q_OBJECT

private slots:
	//  will be called before the first test function is executed
	void initTestCase();
	//  will be called after the last test function was executed.
	//	void cleanupTestCase();
	//  will be called before each test function is executed.
	//	void init();
	//  will be called after every test function.
	void cleanup();

	void insertCourseTest();

private:
	void reset();

	std::shared_ptr<DbV1> mDb;
	std::unique_ptr<DbHelper> mDbHelper;
	std::unique_ptr<QXmlSchemaValidator>  validator;
};

void DbHelperTest::initTestCase()
{
	mDb = DbV1::create();
	mDbHelper = std::unique_ptr<DbHelper>(new DbHelper(mDb, QStringLiteral("TestDb.sqlite")));

	try
	{
		validator = xml::createValidator(QStringLiteral(":/courses/course.xsd"));

		reset();
	}
	catch (Exception& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}
}

/* Closes Db after each test! */
void DbHelperTest::cleanup()
{
	mDb->close();
}

/* Force recreation */
void DbHelperTest::reset()
{
	try
	{
		mDb->open(QStringLiteral("TestDb.sqlite"));
		mDb->dropSchema();
		mDb->createSchema();
		mDb->close();
	}
	catch (DbException& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}
}

void DbHelperTest::insertCourseTest()
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
			QFAIL(qUtf8Printable(QString("Result: %i %s").arg(pr).arg(msg)));
		}
	}
	catch (Exception& e)
	{
		QFAIL(qUtf8Printable(e.message()));
	}

	// and add it to the database
	if (!mDbHelper->insert(*source))
		QFAIL("Unable to insert Course");

	// Read the course back from the database
	std::vector<std::shared_ptr<Course>> list;
	if(!mDbHelper->getCourses(Db::All, std::inserter(list, list.begin()), true))
		QFAIL("Unable to read Courses back");

	QVERIFY(1 == list.size());
	target = *list.begin();

	// Compare the hashes
	QCOMPARE(*target, *source);

	// A second insert of the same course should trigger a unique constraint violation.
	QVERIFY(mDbHelper->insert(*source) != true);

	// Force recreation
	reset();
}

} /* namespace qtouch */

QTEST_GUILESS_MAIN(qtouch::DbHelperTest)
#include "dbhelper_test.moc"
