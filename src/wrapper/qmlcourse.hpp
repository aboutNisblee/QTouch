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
 * \file qmlcourse.hpp
 *
 * \date 03.10.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef QMLCOURSE_HPP_
#define QMLCOURSE_HPP_

#include <memory>
#include <QObject>

#include "entities/course.hpp"

namespace qtouch
{

class QmlLesson: public QObject, public Lesson
{
	Q_OBJECT

	Q_PROPERTY(QUuid id READ getId)
	Q_PROPERTY(QString title READ getTitle WRITE setTitle NOTIFY titleChanged)
	Q_PROPERTY(QString newChars READ getNewChars WRITE setNewChars NOTIFY newCharsChanged)
	Q_PROPERTY(QString text READ getText WRITE setText NOTIFY textChanged)
	Q_PROPERTY(bool builtin READ isBuiltin)

public:
	QmlLesson();
	explicit QmlLesson(const Lesson& rhs);
	virtual ~QmlLesson();

	void setTitle(const QString& title) Q_DECL_OVERRIDE;
	void setNewChars(const QString& newChars) Q_DECL_OVERRIDE;
	void setText(const QString& text) Q_DECL_OVERRIDE;

signals:
	void titleChanged();
	void newCharsChanged();
	void textChanged();
};

class QmlCourse: public QObject
{
	Q_OBJECT

	Q_PROPERTY(QUuid id READ getCourseId)
	Q_PROPERTY(QString title READ getTitle WRITE setTitle NOTIFY titleChanged)
	Q_PROPERTY(QString description READ getDescription WRITE setDescription NOTIFY descriptionChanged)
	Q_PROPERTY(bool builtin READ isBuiltin)

public:
	QmlCourse();
	explicit QmlCourse(std::shared_ptr<Course> rhs);
	virtual ~QmlCourse();

	inline QUuid getCourseId() const { return mCourse->getId(); }

	inline QString getTitle() const { return mCourse->getTitle(); }
	void setTitle(const QString& title);

	inline QString getDescription() const { return mCourse->getDescription(); }
	void setDescription(const QString& description);

	inline bool isBuiltin() const { return mCourse->isBuiltin(); }

signals:
	void titleChanged();
	void descriptionChanged();

private:
	std::shared_ptr<Course> mCourse;
};

} /* namespace qtouch */

#endif /* QMLCOURSE_HPP_ */
