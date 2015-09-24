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
 * \file course_test.cpp
 *
 * \date 02.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include <QtTest/QtTest>

#include "course.hpp"

namespace qtouch
{

class CourseTest: public QObject
{
	Q_OBJECT

public:
	CourseTest()
	{
		lessoncount = 100;

		/* Create some reference data to test against */

		courseTitle = QStringLiteral("TestCourse");
		courseId = QUuid::createUuid();
		courseDescription = QStringLiteral("Only a test course");

		for (quint16 i = 0; i < lessoncount; ++i)
		{
			lessonTitles.append(QStringLiteral("Title No. %1").arg(i + 1));
			lessonIds.push_back(QUuid::createUuid());
			lessonNewCharss.append(QStringLiteral("fj"));
			lessonTexts.append(QStringLiteral("fff jjj"));
		}
	}

private slots:
	// will be called before the first test function is executed
	//	void initTestCase();
	//  will be called after the last test function was executed.
	//	void cleanupTestCase();
	// will be called before each test function is executed.
	void init();
	//  will be called after every test function.
	//	void cleanup();

	void cloning();
	void parentPointer();

	void append();

	void hash();
	void hashList();

private:
	quint16 lessoncount;

	std::shared_ptr<Course> uutCourse;
	std::vector<std::shared_ptr<Lesson>> uutLessons;

	QString courseTitle;
	QUuid courseId;
	QString courseDescription;

	QStringList lessonTitles;
	std::vector<QUuid> lessonIds;
	QStringList lessonNewCharss;
	QStringList lessonTexts;

	void checkCourse(const Course& c);
};

void CourseTest::checkCourse(const Course& c)
{
	QCOMPARE(c.getId(), courseId);
	QCOMPARE(c.getTitle(), courseTitle);
	QCOMPARE(c.getDescription(), courseDescription);

	// Verify that we have enough reference data
	QVERIFY(((quint16) c.size()) <= lessoncount);

	QStringListIterator itTitle(lessonTitles);
	std::vector<QUuid>::const_iterator itId = lessonIds.begin();
	QStringListIterator itNewChars(lessonNewCharss);
	QStringListIterator itText(lessonTexts);

	//	qDebug() << "Course address" << c.data();

	for (auto& it : c)
	{
		QCOMPARE(it->getTitle(), itTitle.next());
		QCOMPARE(it->getId(), *itId);
		QCOMPARE(it->getNewChars(), itNewChars.next());
		QCOMPARE(it->getText(), itText.next());

		// Check that all Lessons have a correct back pointer to its Course
		//		qDebug() << "Parent address" << (*it)->getCourse().data();
		QVERIFY(it->getCourse().get() == &c);

		++itId;
	}
}

/*
 * Reset and check initialization before each test case.
 */
void CourseTest::init()
{
	uutCourse = Course::create();

	// Fill Course with reference data
	uutCourse->setTitle(courseTitle);
	uutCourse->setId(courseId);
	uutCourse->setDescription(courseDescription);

	// Clear Lessons. This function is called before each test.
	uutLessons.clear();

	// Create Lessons, fill them with ref data and add it to a list
	for (int i = 0; i < lessoncount; ++i)
	{
		auto l = std::make_shared<Lesson>();
		l->setTitle(lessonTitles[i]);
		l->setId(lessonIds[i]);
		l->setNewChars(lessonNewCharss[i]);
		l->setText(lessonTexts[i]);

		uutLessons.push_back(l);
	}

	// Add Lessons to Course
	uutCourse->clear();
	uutCourse->insert(uutCourse->begin(), uutLessons.begin(), uutLessons.end());

	// Check initialization
	checkCourse(*uutCourse);
}

/*
 * Copy constructor and assignment operator
 */
void CourseTest::cloning()
{
	// Clone the Course
	auto copy = Course::clone(*uutCourse);

	// And check the copy
	checkCourse(*copy);

	// Manipulate the copy by replacing the lesson list
	Lesson lesson;
	lesson.setTitle("XXX");

	copy->clear();
	copy->push_back(lesson);

	// And check original is untouched
	checkCourse(*uutCourse);

	QVERIFY(copy->size() == 1);
}

void CourseTest::parentPointer()
{
	std::shared_ptr<const Lesson> lesson;

	{
		// Make a copy of the Course
		auto copy = Course::clone(*uutCourse);

		// Get a reference to the first lesson
		lesson = *copy->begin();

		// And let the copy go out of scope
	}

	// Get parent should return a NULL pointer
	//	qDebug() << lesson->getCourse() << "address" << lesson->getCourse().data();
	QVERIFY(!lesson->getCourse());
}

void CourseTest::append()
{
	{
		Lesson newLesson;
		uutCourse->push_back(newLesson);
	}

	QVERIFY(uutCourse->size() == lessoncount + 1);
}

void CourseTest::hash()
{
	QByteArray h1 = uutCourse->hash();
	qDebug() << "MD5: " << h1.toHex();

	// Manipulate the course
	uutCourse->setBuiltin(!uutCourse->isBuiltin());

	// Calculate again
	QByteArray h2 = uutCourse->hash();
	qDebug() << "MD5: " << h2.toHex();

	// Verify not same
	QVERIFY(h1 != h2);
}

void CourseTest::hashList()
{
	QByteArray h1 = qtouch::hash(uutLessons.begin(), uutLessons.end());
	qDebug() << "MD5: " << h1.toHex();

	// Manipulate the a lesson
	uutLessons.at(lessoncount / 2)->setBuiltin(!uutLessons.at(lessoncount / 2)->isBuiltin());

	// Calculate again
	QByteArray h2 = qtouch::hash(uutLessons.begin(), uutLessons.end());
	qDebug() << "MD5: " << h2.toHex();

	// Verify not same
	QVERIFY(h1 != h2);
}

} /* namespace qtouch */

QTEST_APPLESS_MAIN(qtouch::CourseTest)
#include "course_test.moc"
