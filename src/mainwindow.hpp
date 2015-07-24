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
	explicit MainWindow(QQmlEngine* engine = 0, QWindow* parent = 0);
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
