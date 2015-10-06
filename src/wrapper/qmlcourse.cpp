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
 * \file qmlcourse.cpp
 *
 * \date 03.10.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "qmlcourse.hpp"

namespace qtouch
{

QmlLesson::QmlLesson()
{
	setId(QUuid());
	setBuiltin(false);
}

QmlLesson::QmlLesson(const Lesson& rhs) :
	Lesson(rhs)
{
}

QmlLesson::~QmlLesson()
{
}

void QmlLesson::setTitle(const QString& title)
{
	if (title == mTitle)
		return;

	mTitle = title;
	emit titleChanged();
}

void QmlLesson::setNewChars(const QString& newChars)
{
	if (newChars == mNewChars)
		return;

	mNewChars = newChars;
	emit newCharsChanged();
}

void QmlLesson::setText(const QString& text)
{
	if (text == mText)
		return;

	mText = text;
	emit textChanged();
}

QmlCourse::QmlCourse()
{
	mCourse = Course::create();
	mCourse->setId(QUuid());
	mCourse->setBuiltin(false);
}

QmlCourse::QmlCourse(std::shared_ptr<Course> rhs)
{
	mCourse = rhs;
}

QmlCourse::~QmlCourse()
{
}

void QmlCourse::setTitle(const QString& title)
{
	if (title == mCourse->getTitle())
		return;

	mCourse->setTitle(title);
	emit titleChanged();
}

void QmlCourse::setDescription(const QString& description)
{
	if (description == mCourse->getDescription())
		return;

	mCourse->setDescription(description);
	emit descriptionChanged();
}

} /* namespace qtouch */
