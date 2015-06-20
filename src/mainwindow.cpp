#include "mainwindow.hpp"

#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlComponent>
#include <QQuickItem>
#include <QDebug>
#include <QQmlError>

MainWindow::MainWindow(QQmlEngine* engine) : mwEngine(engine)
{
    init();
}

MainWindow::~MainWindow()
{
}

/* Overwriting resizeEvent of QQuickWindow.
 * Forcing the rootItem to fill the window. */
void MainWindow::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e)
    if (rootItem)
    {
        if (!qFuzzyCompare(width(), rootItem->width()))
            rootItem->setWidth(width());
        if (!qFuzzyCompare(height(), rootItem->height()))
            rootItem->setHeight(height());
    }
}

void MainWindow::init()
{
    // If engine not passed via constructor, create a new one
    if (!mwEngine)
        mwEngine = new QQmlEngine(this);

    // TODO: For what its worth??
    mwEngine->setIncubationController(incubationController());

}
