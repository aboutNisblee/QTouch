/**
 * \file datamodel.cpp
 *
 * \date 16.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "datamodel.hpp"

#include <QDir>
#include <QDebug>

#include "xml/parser.hpp"

namespace qtouch
{

namespace
{

ConstCourseMap createCourseMap(const ConstCourseList& list)
{
	ConstCourseMap map;
	foreach(const ConstCoursePtr & c, list)
	{
		map.insert(c->getId(), c);
	}
	return map;
}

} /* namespace anonymous */

DataModel::DataModel()
{
}

DataModel::~DataModel()
{
}

void DataModel::init() throw (Exception)
{
	// Get all course files from the resources
	QDir coursepath(QStringLiteral(":/courses"), "*.xml", QDir::Name | QDir::IgnoreCase, QDir::Files);
	QStringList sourceFileList;
	foreach(QString const & s, coursepath.entryList())
	{
		sourceFileList.append(coursepath.filePath(s));
	}

	// FIXME: Might throw
	xml::ValidatorPtr validator;

	try
	{
		validator = xml::createValidator(QStringLiteral(":/courses/course.xsd"));
	}
	catch (Exception& e)
	{
		qCritical() << e.message();
		throw;
	}

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

			//			beginInsertRows(QModelIndex(), mCourses.size(), mCourses.size());

			mCourses.append(course);

			//			endInsertRows();
		}
		catch (Exception& e)
		{
			qCritical() << e.message();
			throw;
		}
	}

	std::sort(mCourses.begin(), mCourses.end(), CourseListAscTitle());

	mCourseMap = createCourseMap(mCourses);
}

int DataModel::getCourseCount() const
{
	return mCourses.size();
}

QString DataModel::getCourseTitle(const QUuid& courseId) const
{
	if(isValidCourse(courseId))
		return mCourseMap.value(courseId)->getTitle();
	else
		return QString();
}

QString DataModel::getCourseDescription(const QUuid& courseId) const
{
	if(isValidCourse(courseId))
		return mCourseMap.value(courseId)->getDescription();
	else
		return QString();
}

bool DataModel::isCourseBuiltin(const QUuid& courseId) const
{
	if(isValidCourse(courseId))
		return mCourseMap.value(courseId)->isBuiltin();
	else
		return false;
}

CoursePtr DataModel::getCourseCopy(const QUuid& courseId) const
{
	CoursePtr copy;
	if (isValidCourse(courseId))
	{
		copy = Course::clone(mCourseMap.value(courseId));
	}

	return copy;
}

int DataModel::getLessonCount(const QUuid& courseId) const
{
	if(isValidCourse(courseId))
		return mCourseMap.value(courseId)->size();
	else
		return 0;
}

QString DataModel::getLessonTitle(const QUuid& courseId, const QUuid& lessonId) const
{
	if(isValidLesson(courseId, lessonId))
		return mCourseMap.value(courseId)->get(lessonId)->getTitle();
	else
		return QString();
}

QString DataModel::getLessonNewChars(const QUuid& courseId, const QUuid& lessonId) const
{
	if(isValidLesson(courseId, lessonId))
		return mCourseMap.value(courseId)->get(lessonId)->getNewChars();
	else
		return QString();
}

bool DataModel::isLessonBuiltin(const QUuid& courseId, const QUuid& lessonId) const
{
	if(isValidLesson(courseId, lessonId))
		return mCourseMap.value(courseId)->get(lessonId)->isBuiltin();
	else
		return false;
}

QString DataModel::getLessonText(const QUuid& courseId, const QUuid& lessonId) const
{
	if(isValidLesson(courseId, lessonId))
		return mCourseMap.value(courseId)->get(lessonId)->getText();
	else
		return QString();
}

} /* namespace qtouch */
