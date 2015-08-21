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

bool TrainingWidget::resize()
{
	mDoc.setTextWidth(-1);

	// Calculate size
	qreal itemWidth = 0;
	qreal itemHeight = 0;
	if (mAutoWrap)
	{
		/* If autoWrap enabled:
		 * The maxWidth defines the textWidth and width. */
		itemWidth = (mMaxWidth > 0 && mDoc.idealWidth() > mMaxWidth) ? mMaxWidth : mDoc.idealWidth();
	}
	else
	{
		/* If autoWrap disabled:
		 * The idealWidth defines the textWidth and enforces a minimum width. */
		itemWidth = mDoc.idealWidth();
	}

	/* Calculate scale
	 * FIXME: It is impossible to determine the scale when mAutoWrap is true,
	 * cause the whole text has only one line. Increase the char size manually,
	 * when mAutoWrap is true.
	 */
	mTextScale = (mMaxWidth > 0) ? mMaxWidth / itemWidth : 1;

	// Note: Its crucial to define the TextWidth before accessing the size().height()
	mDoc.setTextWidth(itemWidth);

	// height is defined by document as long as a maxHeight isn't specified
	itemHeight = (mMaxHeight > 0 && mDoc.size().height() > mMaxHeight) ? mMaxHeight : mDoc.size().height();

	if ((itemWidth * mTextScale) != width() || (itemHeight * mTextScale) != height())
	{
//		qDebug() << "New TrainingWidget size:" <<
//		         "\n\titemWidth:" << itemWidth << "*" << mTextScale << "=" << (itemWidth * mTextScale) <<
//		         "\n\titemHeight:" << itemHeight << "*" << mTextScale << "=" << (itemHeight * mTextScale);

		setWidth(itemWidth * mTextScale);
		setHeight(itemHeight * mTextScale);

		return true;
	}

	return false;
}

void TrainingWidget::updateImage()
{
	mImage.reset(new QImage(width(), height(), QImage::Format_ARGB32_Premultiplied));
	mImage->fill(0);

	QPainter p(mImage.get());
	p.scale(mTextScale, mTextScale);
	//	p.setRenderHint(QPainter::TextAntialiasing);
	mDoc.drawContents(&p);

	update();
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
