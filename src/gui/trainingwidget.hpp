/**
 * \file trainingwidget.hpp
 *
 * \date 11.08.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef TRAININGWIDGET_HPP_
#define TRAININGWIDGET_HPP_

#include "textpage.hpp"

namespace qtouch
{

class TrainingWidget: public TextPage
{
	Q_OBJECT

public:
	TrainingWidget(QQuickItem* parent = 0);
	virtual ~TrainingWidget();
};

} /* namespace qtouch */

#endif /* TRAININGWIDGET_HPP_ */
