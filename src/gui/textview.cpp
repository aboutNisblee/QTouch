/* Copyright (C) 2015  Moritz Nisblé <moritz.nisble@gmx.de>
 *
 * This file is part of QTouch.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/**
 * \file textview.cpp
 *
 * \date 06.08.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include <gui/textview.hpp>

#include <QtMath>
#include <QPainter>
#include <QPen>

namespace qtouch
{

TextView::TextView(QQuickItem* parent):
	QQuickPaintedItem(parent), mDoc(new Document(this)), mBorder(new Border(this))
{
	setFlag(ItemHasContents, true);
	setRenderTarget(QQuickPaintedItem::FramebufferObject);
	// TODO: Make this configurable. It eats too much graphics memory to be enabled by default.
	/*setPerformanceHint(QQuickPaintedItem::FastFBOResizing);*/
	setAntialiasing(true);
	setFillColor(QColor("white"));

	connect(mDoc, &Document::contentsChanged, this, &TextView::resize);
}

TextView::~TextView()
{
}

void TextView::setMaxWidth(qreal maxWidth)
{
	if (maxWidth > 0 && !qFuzzyCompare(maxWidth, mMaxWidth))
	{
		mMaxWidth = maxWidth;
		resize();
		emit maxWidthChanged();
	}
}

void TextView::setMinWidth(qreal minWidth)
{
	if (minWidth > 0 && !qFuzzyCompare(minWidth, mMinWidth))
	{
		mMinWidth = minWidth;
		resize();
		emit minWidthChanged();
	}
}

void TextView::resize()
{
	// IdealWith is defined by the longest line plus margins
	qreal idealWidth = mDoc->getIdealWidth();

	// Calculate scale
	qreal scale = 1;
	if (mMaxWidth > 0 && idealWidth > mMaxWidth) // Scale down
		scale = mMaxWidth / idealWidth;
	else if (mMinWidth > 0 && idealWidth < mMinWidth) // Scale up
		scale = mMinWidth / idealWidth;

	setContentsScale(scale);

	// Note: Its crucial to define the TextWidth before accessing the size().height()
	mDoc->setTextWidth(idealWidth);

	// height is defined by document
	qreal itemHeight = mDoc->size().height();

	if (isVisible())
	{
		setWidth(idealWidth * scale);
		setHeight(itemHeight * scale);

		setContentsSize(QSize(idealWidth, itemHeight));

		update();
	}
}

void TextView::paint(QPainter* painter)
{
	//	qDebug() << "PAINTING" << this;
	//	qDebug() << "width:" << width();
	//	qDebug() << "height:" << height();
	//	qDebug() << "contentsSize:" << contentsSize();
	//	qDebug() << "contentsScale:" << contentsScale();
	//	qDebug() << "contentsBoundingRect:" << contentsBoundingRect();

	int width = mBorder->property("width").toInt();
	if (width > 0)
	{
		QColor color = qvariant_cast<QColor>(mBorder->property("color"));
		painter->save();
		painter->setBackground(QColor("transparent"));
		painter->setPen(QPen(color, width));
		painter->scale(1 / contentsScale(), 1 / contentsScale());
		painter->drawRect(contentsBoundingRect());
		painter->restore();
	}
	mDoc->drawContents(painter);
}

} /* namespace qtouch */
