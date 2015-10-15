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
 * \file coursemodel.hpp
 *
 * \date 12.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef COURSEMODEL_HPP_
#define COURSEMODEL_HPP_

#include <QAbstractListModel>
#include <QUuid>

#include "wrapper/qmlcourse.hpp"

namespace qtouch
{

class DataModel;
class CourseModel;

class LessonModel: public QAbstractListModel
{
	Q_OBJECT

	Q_PROPERTY(int index READ getIndex WRITE selectLesson NOTIFY indexChanged)
	Q_PROPERTY(qtouch::QmlLesson* lesson READ getLesson NOTIFY lessonChanged)

public:
	/** LessonModelRoles */
	enum LessonModelRoles
	{
		UuidRole = Qt::UserRole + 1,//!< UuidRole lId
		TitleRole,                  //!< TitleRole lTitle
		NewCharsRole,               //!< NewCharsRole lNewChars
		BuiltinRole,                //!< BuiltinRole lBuiltin
		TextRole                    //!< TextRole lText
	};

	explicit LessonModel(QObject* parent = nullptr);
	explicit LessonModel(DataModel* model, QObject* parent = nullptr);
	virtual ~LessonModel();

	int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
	QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;

	Q_INVOKABLE QVariantMap get(int index);

	inline int getIndex() const { return mSelected; }
	void selectLesson(int index);

	QmlLesson* getLesson() const;

signals:
	void indexChanged();
	void lessonChanged();

protected:
	virtual QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

private:
	friend class CourseModel;
	inline void setCourse(int index) { mCourseIndex = index; }

	DataModel* mDm;

	int mCourseIndex;
	int mSelected;
};

class CourseModel: public QAbstractListModel
{
	Q_OBJECT

	Q_PROPERTY(int index READ getIndex WRITE selectCourse NOTIFY indexChanged)
	Q_PROPERTY(qtouch::QmlCourse* course READ getCourse NOTIFY courseChanged)

	/**
	 * The LessonModel for the lessons of the currently selected course.
	 * @note Since the pointer never changes its value you cannot rebind this property to another one
	 * and catch the onChanged signal. Use property aliasing instead of rebinding.
	 * Binding to this property directly works, because of the explicit emission of selectedLessonModelChanged.
	 */
	Q_PROPERTY(qtouch::LessonModel* lessonModel READ getLessonModel NOTIFY lessonModelChanged)

public:
	/** CourseModelRoles */
	enum CourseModelRoles
	{
		UuidRole = Qt::UserRole + 1,//!< UuidRole cId
		TitleRole,                  //!< TitleRole cTitle
		DescriptionRole,            //!< DescriptionRole cDescription
		BuiltinRole,                //!< BuiltinRole cBuiltin
	};

	explicit CourseModel(QObject* parent = nullptr);
	explicit CourseModel(DataModel* model, QObject* parent = nullptr);
	virtual ~CourseModel();

	// TODO: Add flags to control enabled items! E.g. different language, never used, ...

	int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
	QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;

	Q_INVOKABLE QVariantMap get(int index);

	inline int getIndex() const { return mSelected; }
	void selectCourse(int index);

	QmlCourse* getCourse() const;

	inline LessonModel* getLessonModel() const { return mLessonModel; }

signals:
	void indexChanged();
	void courseChanged();
	void lessonModelChanged();

protected:
	virtual QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

private:
	DataModel* mDm;

	// Lifetime managed by Qt
	LessonModel* mLessonModel;
	int mSelected;
};

} /* namespace qtouch */

#endif /* COURSEMODEL_HPP_ */
