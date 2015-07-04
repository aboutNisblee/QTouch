#include <QApplication>
#include <QDebug>

#include "mainwindow.hpp"

int main(int argc, char* argv[])
{
	QApplication app(argc, argv);

	// Use QCommandLineParser to parse arguments (see documentation)

	QScopedPointer<qtouch::MainWindow> mw(new qtouch::MainWindow);
	mw->show();

	return app.exec();
}
