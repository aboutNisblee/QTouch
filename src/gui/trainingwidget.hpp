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

	Q_PROPERTY(Qt::Key escapeKey MEMBER mEscKey NOTIFY escapeKeyChanged)

public:
	TrainingWidget(QQuickItem* parent = 0);
	virtual ~TrainingWidget();

signals:
	void escape();
	void escapeKeyChanged();

protected:
	virtual bool resize() override;
	virtual void updateImage() override;

protected:
	virtual bool event(QEvent*) override;

private:
	qreal mTextScale = 1;
	Qt::Key mEscKey = Qt::Key_Escape;
};

} /* namespace qtouch */

#endif /* TRAININGWIDGET_HPP_ */
