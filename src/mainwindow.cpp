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
 * \file mainwindow.cpp
 *
 * \date 20.06.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include <gui/textview.hpp>
#include <gui/trainingwidget.hpp>
#include "mainwindow.hpp"

#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlComponent>
#include <QQuickItem>
#include <QQmlError>

#include <QDebug>

#include "utils/exceptions.hpp"
#include "coursemodel.hpp"
#include "gui/svgelementprovider.hpp"

namespace qtouch
{

namespace
{

bool componentError(QQmlComponent* c)
{
	if (c->isError())
	{
		QList<QQmlError> errorList = c->errors();
		foreach(const QQmlError & error, errorList)
		{
			QMessageLogger(qPrintable(error.url().toString()), error.line(), 0).warning() << error;
		}
		return true;
	}
	return false;
}

void registerQmlTypes()
{
	qRegisterMetaType<CourseModel*>("CourseModel*");
	qmlRegisterType<CourseModel>("de.nisble.qtouch", 1, 0, "CourseModel");

	qRegisterMetaType<LessonModel*>("LessonModel*");
	//	qmlRegisterType<LessonModel>("de.nisble.qtouch", 1, 0, "LessonModel");
	qmlRegisterType<LessonModel>();

	qmlRegisterType<TextView>("de.nisble.qtouch", 1, 0, "TextPage");
	qmlRegisterType<TrainingWidget>("de.nisble.qtouch", 1, 0, "TrainingWidget");
}

} /* namespace anonymous */

MainWindow::MainWindow(QQmlEngine* engine, QWindow* parent) :
	QQuickWindow(parent), mwEngine(engine), mwComponent(nullptr), mwItem(nullptr), mDataModel(nullptr),
	mCourseModel(nullptr)
{
}

MainWindow::~MainWindow()
{
}

/* Overwriting resizeEvent of QQuickWindow.
 * Forcing the mwItem to fill the window. */
void MainWindow::resizeEvent(QResizeEvent* e)
{
	Q_UNUSED(e)
	if (mwItem)
	{
		if (!qFuzzyCompare(width(), mwItem->width()))
			mwItem->setWidth(width());
		if (!qFuzzyCompare(height(), mwItem->height()))
			mwItem->setHeight(height());
	}
}

bool MainWindow::init()
{
	// Create the QmlEngine, if not passed via constructor
	if (!mwEngine)
		mwEngine = new QQmlEngine(this);

	// Install the IncubationController of QQuickWindow
	mwEngine->setIncubationController(incubationController());

	// Register needed types
	registerQmlTypes();

	// Add image provider
	mwEngine->addImageProvider(QStringLiteral("svgelement"), new SvgElementProvider(QQmlImageProviderBase::Image,
	                           QUrl(QStringLiteral("qrc:///images/"))));

	mDataModel = new DataModel(this);

	try
	{
		mDataModel->init();
	}
	catch (Exception& e)
	{
		qCritical() << e.message();
		return false;
	}

	/* TODO: Check it!
	 * QML cares about deletion, when no parent is set!?
	 */
	mCourseModel = new CourseModel(mDataModel, this);

	// Embed the course model
	mwEngine->rootContext()->setContextProperty("$courseModel", mCourseModel);

	// Create root component
	mwComponent = new QQmlComponent(mwEngine, QUrl(QStringLiteral("qrc:/qml/MainWindow.qml")), this);

	if (componentError(mwComponent))
		return false;

	// Instantiate component in root context
	QObject* object = mwComponent->create();

	if (componentError(mwComponent))
	{
		delete object;
		return false;
	}

	// Get the root Item back
	mwItem = qobject_cast<QQuickItem*>(object);
	if (mwItem)
	{
		// and set the visual parent of the component to the invisible root item of the QQuickWindow
		mwItem->setParentItem(contentItem());
	}
	else
	{
		qWarning() << "The root item of MainWindow.qml must be derived from QQuickItem";
		delete mwItem;
		return false;
	}

	// Set the window to the size defined in QML root item
	QSize size(mwItem->width(), mwItem->height());
	if (size.isValid())
		resize(size);
	else
		qWarning() << "Invalid size of root item";

	// XXX: DEBUGGING
	connect(this, &QQuickWindow::activeFocusItemChanged, [&] { qDebug() << "Active focus given to" << activeFocusItem(); });

	// FIXME: Not nice but fixes initialization problem
	mCourseModel->selectCourse(0);

	return true;
}

} /* namespace qtouch */
