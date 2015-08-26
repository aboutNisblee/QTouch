/**
 * \file trainingwidget.cpp
 *
 * \date 11.08.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "trainingwidget.hpp"

#include <QPainter>

namespace qtouch
{

TrainingWidget::TrainingWidget(QQuickItem* parent) :
	TextPage(parent)
{
}

TrainingWidget::~TrainingWidget()
{
}

bool TrainingWidget::event(QEvent* event)
{
	//	qDebug() << event;
	if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
		if (keyEvent->key() == mEscKey)
			emit escape();
	}

	event->ignore();
	return false;
}

} /* namespace qtouch */
