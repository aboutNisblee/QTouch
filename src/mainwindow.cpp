#include "mainwindow.hpp"

#include <QQmlEngine>
#include <QQmlContext>
#include <QQmlComponent>
#include <QQuickItem>
#include <QDebug>
#include <QQmlError>

#include "homescreencontroller.hpp"
#include "lessonmodel.hpp"

namespace qtouch
{

MainWindow::MainWindow(QQmlEngine* engine) :
	mwEngine(engine)
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

	//	// Embed controller objects
	//	hsController = new HomeScreenController(this);
	//	mwEngine->rootContext()->setContextObject(hsController);

	lessonModel = new LessonModel(this);
	mwEngine->rootContext()->setContextProperty("lessonModel", lessonModel);

	qmlRegisterType<HomeScreenController>("de.nisble.qtouch", 1, 0, "HomeScreenController");

	// Create root component
	component = new QQmlComponent(mwEngine.data(), QUrl(QStringLiteral("qrc:/qml/MainWindow.qml")), this);

	if (componentError(component))
		return;

	// Instantiate component in root context
	QObject* object = component->create();

	if (componentError(component))
	{
		delete object;
		return;
	}

	rootItem = qobject_cast<QQuickItem*>(object);
	if (rootItem)
	{
		// Set the visual parent of the component to the invisible root item of the QQuickWindow
		rootItem->setParentItem(contentItem());
	}
	else
	{
		qWarning() << "The root item of MainWindow.qml must be derived from QQuickItem";
		delete rootItem;
		return;
	}

	// Set the window to the size defined in QML root item
	QSize size(rootItem->width(), rootItem->height());
	if (size.isValid())
		resize(size);
	else
		qWarning() << "Invalid size of root item";
}

bool MainWindow::componentError(QQmlComponent* c)
{
	if (component->isError())
	{
		QList<QQmlError> errorList = component->errors();
		foreach(const QQmlError & error, errorList)
		{
			QMessageLogger(error.url().toString().toLatin1().constData(), error.line(), 0).warning() << error;
		}
		return true;
	}
	return false;
}

} /* namespace qtouch */
