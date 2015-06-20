#include <QApplication>
#include <QDebug>

#include "mainwindow.hpp"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Use QCommandLineParser to parse arguments (see documentation)

    QScopedPointer<MainWindow> mw(new MainWindow);
    mw->show();

    return app.exec();
}
