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
 * \file main.cpp
 *
 * \date 20.06.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include <QGuiApplication>
#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlComponent>
#include <QQuickWindow>
#include <QDebug>

#include "utils/exceptions.hpp"
#include "datamodel.hpp"
#include "coursemodel.hpp"
#include "profilemodel.hpp"
#include "wrapper/qmlcourse.hpp"
#include "wrapper/qmlprofile.hpp"
#include "document.hpp"
#include "recorder.hpp"
#include "gui/svgelementprovider.hpp"
#include "gui/textview.hpp"
#include "gui/trainingwidget.hpp"

namespace
{

bool componentError(QQmlComponent* c)
{
	if (c->isError())
	{
		QList<QQmlError> errorList = c->errors();
		for (const QQmlError& error : errorList)
			QMessageLogger(qPrintable(error.url().toString()), error.line(), 0).warning() << error;
		return true;
	}
	return false;
}

void registerQmlTypes()
{
	qmlRegisterType<qtouch::QmlLesson>("de.nisble.qtouch", 1, 0, "Lesson");
	qmlRegisterType<qtouch::QmlCourse>("de.nisble.qtouch", 1, 0, "Course");
	qmlRegisterType<qtouch::CourseModel>("de.nisble.qtouch", 1, 0, "CourseModel");
	qmlRegisterType<qtouch::LessonModel>();

	qmlRegisterType<qtouch::QmlProfile>("de.nisble.qtouch", 1, 0, "Profile");
//	qmlRegisterType<qtouch::QmlSkillLevel>("de.nisble.qtouch", 1, 0, "SkillLevel");
	qmlRegisterType<qtouch::QmlStats>("de.nisble.qtouch", 1, 0, "Stats");
	qmlRegisterType<qtouch::ProfileModel>();

	qmlRegisterType<qtouch::Document>();
	qmlRegisterType<qtouch::Recorder>("de.nisble.qtouch", 1, 0, "Recorder");

	qmlRegisterType<qtouch::Border>();
	qmlRegisterType<qtouch::TextView>("de.nisble.qtouch", 1, 0, "TextView");
	qmlRegisterType<qtouch::TrainingWidget>("de.nisble.qtouch", 1, 0, "TrainingWidget");
}

} /* namespace anonymous */

int main(int argc, char* argv[])
{
	QGuiApplication app(argc, argv);

	// Use QCommandLineParser to parse arguments (see documentation)

	QQmlEngine engine;

	// Register needed types
	registerQmlTypes();

	// Add image provider
	engine.addImageProvider(QStringLiteral("svgelement"), new qtouch::SvgElementProvider(QQmlImageProviderBase::Image,
	                        QUrl(QStringLiteral("qrc:///images/"))));

	qtouch::DataModel dataModel;
	try
	{
		dataModel.init();
	}
	catch (qtouch::Exception& e)
	{
		qCritical() << e.message();
		return EXIT_FAILURE;
	}

	qtouch::CourseModel* courseModel = new qtouch::CourseModel(&dataModel, &app);
	qtouch::ProfileModel* profileModel = new qtouch::ProfileModel(&dataModel, &app);
	// Embed view models
	engine.rootContext()->setContextProperty("$courseModel", courseModel);
	engine.rootContext()->setContextProperty("$profileModel", profileModel);

	// Create root component
	QQmlComponent component(&engine);
	QQuickWindow::setDefaultAlphaBuffer(true);
	component.loadUrl(QUrl(QStringLiteral("qrc:/qml/MainWindow.qml")));

	if (componentError(&component))
		return EXIT_FAILURE;

	if (component.isReady())
	{
		component.create();
		if (componentError(&component))
			return EXIT_FAILURE;
	}
	else
	{
		qWarning() << component.errorString();
		return EXIT_FAILURE;
	}

	// FIXME: Not nice but fixes initialization problem
	courseModel->selectCourse(0);

	return app.exec();
}
