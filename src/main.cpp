/**
 * \file main.cpp
 *
 * \date 20.06.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include <QApplication>
#include <QDebug>

#include "mainwindow.hpp"

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	// Use QCommandLineParser to parse arguments (see documentation)

	QScopedPointer<qtouch::MainWindow> mw(new qtouch::MainWindow);

	if (!mw->init())
	{
		qCritical() << "Unable to initialize main window controller";
		return EXIT_FAILURE;
	}

	// Show ...
	mw->show();

	return app.exec();
}
