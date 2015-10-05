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
 * \file coursemodel.cpp
 *
 * \date 12.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "coursemodel.hpp"

#include <algorithm>

#include <QDebug>

#include "datamodel.hpp"

namespace qtouch
{

LessonModel::LessonModel(QObject* parent):
	QAbstractListModel(parent), mDm(nullptr), mCourseIndex(-1), mSelected(-1)
{
}

LessonModel::LessonModel(DataModel* model, QObject* parent) :
	QAbstractListModel(parent), mDm(model), mCourseIndex(-1), mSelected(-1)
{
}

LessonModel::~LessonModel()
{
}

int LessonModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	if (mDm->isValidCourseIndex(mCourseIndex))
		return mDm->getLessonCount(mCourseIndex);
	else
		return 0;
}

QVariant LessonModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (!mDm->isValidLessonIndex(mCourseIndex, index.row()))
		return QVariant();

	switch (role)
	{
	// TODO: Handle t roles
	case UuidRole:
		return mDm->getLesson(mCourseIndex, index.row())->getId();
		break;
	case TitleRole:
		return mDm->getLesson(mCourseIndex, index.row())->getTitle();
		break;
	case NewCharsRole:
		return mDm->getLesson(mCourseIndex, index.row())->getNewChars();
		break;
	case BuiltinRole:
		return mDm->getLesson(mCourseIndex, index.row())->isBuiltin();
		break;
	case TextRole:
		return mDm->getLesson(mCourseIndex, index.row())->getText();
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
	if (!mDm->isValidLessonIndex(mCourseIndex, index))
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

	emit selectedLessonIdChanged();
	emit selectedLessonTitleChanged();
	emit selectedLessonNewCharsChanged();
	emit selectedLessonBuiltinChanged();
	emit selectedLessonTextChanged();
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
	QAbstractListModel(parent), mDm(nullptr), mLessonModel(0), mSelected(-1)
{
}

CourseModel::CourseModel(DataModel* model, QObject* parent):
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
		return mDm->getCourse(index.row())->getId();
		break;
	case TitleRole:
		return mDm->getCourse(index.row())->getTitle();
		break;
	case DescriptionRole:
		return mDm->getCourse(index.row())->getDescription();
		break;
	case BuiltinRole:
		return mDm->getCourse(index.row())->isBuiltin();
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
		mLessonModel->setCourse(index);
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

