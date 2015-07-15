/**
 * \file mainwindow.cpp
 *
 * \date 20.06.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "mainwindow.hpp"

#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlComponent>
#include <QQuickItem>
#include <QQmlError>

#include <QDebug>

#include "coursemodel.hpp"

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
//	qmlRegisterUncreatableType<MainWindowController>("de.nisble.qtouch", 1, 0, "HomeScreenController",
//	        "Lifetime is maintained by backend");
//	qmlRegisterUncreatableType<MainWindowController>("de.nisble.qtouch", 1, 0, "CourseModel",
//	        "Lifetime is maintained by backend");

	qRegisterMetaType<LessonModel*>("LessonModel*");
	qmlRegisterType<LessonModel>("de.nisble.qtouch", 1, 0, "LessonModel");
}

} /* namespace anonymous */

MainWindow::MainWindow(QQmlEngine* engine, QWindow* parent) :
		QQuickWindow(parent), mwEngine(engine), mwComponent(0), mwItem(0)
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

	// TODO: For what its worth??
	mwEngine->setIncubationController(incubationController());

	// Register needed types
	registerQmlTypes();

	mDataModel.reset(new DataModel());

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
	 * This class cannot be parent, since this is not an QObject.
	 */
	mCourseModel = new CourseModel(mDataModel, this);

	// Embed the course model
	mwEngine->rootContext()->setContextProperty("courseModel", mCourseModel);

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

	// FIXME: Not nice but fixes initialization problem
	mCourseModel->selectCourse(0);

	return true;
}

} /* namespace qtouch */
