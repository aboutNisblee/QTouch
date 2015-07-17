/**
 * \file coursemodel.cpp
 *
 * \date 12.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "coursemodel.hpp"

#include <algorithm>

#include <QDebug>

namespace qtouch
{

LessonModel::LessonModel(QObject* parent):
	QAbstractListModel(parent), mSelected(-1)
{
}

LessonModel::LessonModel(DataModelPtr model, QObject* parent) :
	QAbstractListModel(parent), mDm(model), mSelected(-1)
{
}

LessonModel::~LessonModel()
{
}

int LessonModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	if (mDm->isValidCourse(mCourseId))
		return mDm->getLessonCount(mCourseId);
	else
		return 0;
}

QVariant LessonModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (!mDm->isValidLessonIndex(mCourseId, index.row()))
		return QVariant();

	QUuid lessonId = mDm->getLessonId(mCourseId, index.row());

	switch (role)
	{
	// TODO: Handle t roles
	case UuidRole:
		return mDm->getLessonId(mCourseId, index.row());
		break;
	case TitleRole:
		return mDm->getLessonTitle(mCourseId, lessonId);
		break;
	case NewCharsRole:
		return mDm->getLessonNewChars(mCourseId, lessonId);
		break;
	case BuiltinRole:
		return mDm->isLessonBuiltin(mCourseId, lessonId);
		break;
	case TextRole:
		return mDm->getLessonText(mCourseId, lessonId);
		break;
	default:
		return QVariant();
		break;
	}
}

/**
 * Access model directly by using Roles.
 * E.g. lessonModel.get(lessonModel.get.selectedLessonIndex).lNewChars
 * @param i The course index.
 * @return A QMap that associates the roles and the values as QVariants.
 */
QVariantMap LessonModel::get(int row)
{
	// QVariantMap is a synonym for QMap<QString, QVariant>
	QVariantMap result;

	// Get the hash table dictionary of role names
	QHash<int, QByteArray> names = roleNames();

	QHashIterator<int, QByteArray> i(names);
	while (i.hasNext())
	{
		i.next();

		// Get the index for the given row
		QModelIndex idx = index(row, 0);
		// Use the index to access the data
		QVariant data = idx.data(i.key());

		result[i.value()] = data;
	}
	return result;
}

void LessonModel::selectLesson(int index)
{
	if (!mDm->isValidLessonIndex(mCourseId, index))
	{
		qWarning() << this << "Invalid index:" << index;
		mSelected = -1;
	}
	else /* Do net check for index changes. Simply update and fire! */
		//	else if(mSelected != index)
	{
		mSelected = index;
	}

	emit selectedLessonIndexChanged();
}

QHash<int, QByteArray> LessonModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[UuidRole] = "lId";
	roles[TitleRole] = "lTitle";
	roles[NewCharsRole] = "lNewChars";
	roles[BuiltinRole] = "lBuiltin";
	roles[TextRole] = "lText";
	return roles;
}

CourseModel::CourseModel(QObject* parent):
	QAbstractListModel(parent), mLessonModel(0), mSelected(-1)
{
}

CourseModel::CourseModel(DataModelPtr model, QObject* parent):
	QAbstractListModel(parent), mDm(model)
{
	mLessonModel = new LessonModel(mDm, this);
	selectCourse(0);
}

CourseModel::~CourseModel()
{
}

int CourseModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return mDm->getCourseCount();
}

QVariant CourseModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (!mDm->isValidCourseIndex(index.row()))
		return QVariant();

	switch (role)
	{
	// TODO: Handle default roles
	case UuidRole:
		return mDm->getCourseId(index.row());
		break;
	case TitleRole:
		return mDm->getCourseTitle(mDm->getCourseId(index.row()));
		break;
	case DescriptionRole:
		return mDm->getCourseDescription(mDm->getCourseId(index.row()));
		break;
	case BuiltinRole:
		return mDm->isCourseBuiltin(mDm->getCourseId(index.row()));
		break;
	default:
		return QVariant();
		break;
	}
}

/**
 * Access model directly by using Roles.
 * E.g. courseModel.get(courseModel.selectedCourseIndex).cDescription
 * @param i The course index.
 * @return A QMap that associates the roles and the values as QVariants.
 */
QVariantMap CourseModel::get(int i)
{
	// QVariantMap is a synonym for QMap<QString, QVariant>
	QVariantMap result;

	// Get the hash table dictionary of role names
	QHash<int, QByteArray> names = roleNames();

	QHashIterator<int, QByteArray> it(names);
	while (it.hasNext())
	{
		it.next();

		// Get the index for the given row
		QModelIndex idx = index(i, 0);
		// Use the index to access the data
		QVariant data = idx.data(it.key());

		result[it.value()] = data;
	}
	return result;
}

void CourseModel::selectCourse(int index)
{
	if (!mDm->isValidCourseIndex(index))
	{
		qWarning() << this << "Invalid index:" << index;
		mSelected = -1;
	}
	else /* Do net check for index changes. Simply update and fire! */
	{
		mSelected = index;

		// Change the model before firing index changed!
		mLessonModel->setCourse(mDm->getCourseId(index));
		emit selectedLessonModelChanged();
		emit selectedCourseIndexChanged();

		emit selectedCourseIdChanged();
		emit selectedCourseTitleChanged();
		emit selectedCourseDescriptionChanged();
		emit selectedCourseBuiltinChanged();

		// This fires selectedLessonIndexChanged
		mLessonModel->selectLesson(0);
	}
}

/**
 * If one can trust the documentation the QmlEngine cares about cleanup
 * when no parent is set!
 * @param index
 * @return
 */
//LessonModel* CourseModel::getLessonModel(int index) const
//{
//	LessonModel* hopefullyTheQmlEngineCaresAboutCleanup = new LessonModel();
//
//	if (index >= 0 && index < mDm->courseCount())
//		hopefullyTheQmlEngineCaresAboutCleanup->setCourse(mCourses.at(index));
//
//	return hopefullyTheQmlEngineCaresAboutCleanup;
//}

QHash<int, QByteArray> CourseModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[UuidRole] = "cId";
	roles[TitleRole] = "cTitle";
	roles[DescriptionRole] = "cDescription";
	roles[BuiltinRole] = "cBuiltin";
	return roles;
}

} /* namespace qtouch */
