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
		lessoncount = 1;

		/* Create some reference data to test against */

		courseTitle = QStringLiteral("TestCourse");
		courseId = QUuid::createUuid();
		courseDescription = QStringLiteral("Only a test course");

		for (quint16 i = 0; i < lessoncount; ++i)
		{
			lessonTitles.append(QStringLiteral("Title No. %1").arg(i + 1));
			lessonIds.append(QUuid::createUuid());
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

	void hash();
	void serialization();

private:
	quint16 lessoncount;

	CoursePtr uutCourse;
	LessonList uutLessons;

	QString courseTitle;
	QUuid courseId;
	QString courseDescription;

	QStringList lessonTitles;
	QList<QUuid> lessonIds;
	QStringList lessonNewCharss;
	QStringList lessonTexts;

	void checkCourse(const CoursePtr& c);
};

void CourseTest::checkCourse(const CoursePtr& c)
{
	// Verify that we have enough reference data
	QVERIFY(((quint16) c->lessonCount()) <= lessoncount);

	QStringListIterator itTitle(lessonTitles);
	QListIterator<QUuid> itId(lessonIds);
	QStringListIterator itNewChars(lessonNewCharss);
	QStringListIterator itText(lessonTexts);

	//	qDebug() << "Course address" << c.data();

	for (Course::const_iterator it = c->begin(); it != c->end(); ++it)
	{
		QCOMPARE((*it)->getTitle(), itTitle.next());
		QCOMPARE((*it)->getId(), itId.next());
		QCOMPARE((*it)->getNewChars(), itNewChars.next());
		QCOMPARE((*it)->getText(), itText.next());

		// Check that all Lessons have a correct back pointer to its Course
		//		qDebug() << "Parent address" << (*it)->getCourse().data();
		QVERIFY((*it)->getCourse() == c.data());
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
		LessonPtr l(new Lesson);
		l->setTitle(lessonTitles[i]);
		l->setId(lessonIds[i]);
		l->setNewChars(lessonNewCharss[i]);
		l->setText(lessonTexts[i]);

		uutLessons.append(l);
	}

	// Add Lessons to Course
	uutCourse->replace(uutLessons);

	// Check initialization
	checkCourse(uutCourse);
}

/*
 * Copy constructor and assignment operator
 */
void CourseTest::cloning()
{
	// Clone the Course
	CoursePtr copy = Course::clone(uutCourse);

	// And check the copy
	checkCourse(copy);

	// Manipulate the copy
	(*copy->begin())->setTitle("XXX");

	// And check original is untouched
	checkCourse(uutCourse);
}

void CourseTest::parentPointer()
{
	LessonPtr lesson;
	{
		// Make a copy of the Course
		CoursePtr copy = Course::clone(uutCourse);

		// Get a reference to the first lesson
		lesson = *copy->begin();

		// And let the copy go out of scope
	}

	// Get parent should return a NULL pointer
	//	qDebug() << lesson->getCourse() << "address" << lesson->getCourse().data();
	QVERIFY(!lesson->getCourse());
}

void CourseTest::hash()
{
	QByteArray buffer = Course::hash(uutCourse);

	QVERIFY(buffer.size() == 16);

	qDebug() << "MD5: " << buffer.toHex();

	// Make a backup of the first hash
	QByteArray hash(buffer);

	buffer.clear();

	// Calculate again
	buffer = Course::hash(uutCourse);

	QVERIFY(buffer.size() == 16);

	// Verify same
	QVERIFY(hash == buffer);
}

void CourseTest::serialization()
{
	QByteArray buffer;
	QDataStream stream(&buffer, QIODevice::WriteOnly);

	qDebug() << uutCourse.isNull();
	stream << uutCourse;

	qDebug() << "Size of serialized test course:" << buffer.size();
	qDebug() << "Value:" << buffer.toHex();
}

} /* namespace qtouch */

QTEST_APPLESS_MAIN(qtouch::CourseTest)
#include "course_test.moc"
