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
 * \file mainwindow.hpp
 *
 * \date 20.06.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QQuickWindow>

#include "datamodel.hpp"

class QQmlEngine;
class QQmlComponent;
class QQuickItem;

/**
 * Namespace of QTouch
 */
namespace qtouch
{

class CourseModel;

class MainWindow: public QQuickWindow
{
	Q_OBJECT

public:
	MainWindow(QQmlEngine* engine = nullptr, QWindow* parent = nullptr);
	~MainWindow();

	bool init();

signals:
	//	void sizeChanged(int width, int height);

public slots:

protected:
	void resizeEvent(QResizeEvent* e);

private:
	QQmlEngine* mwEngine;
	QQmlComponent* mwComponent;
	QQuickItem* mwItem;

	DataModel* mDataModel;
	CourseModel* mCourseModel;

	Q_DISABLE_COPY(MainWindow)
};

} /* namespace qtouch */

#endif // MAINWINDOW_HPP
