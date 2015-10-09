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
#include <QImage>
#include <QtMath>
#include <QSGSimpleTextureNode>
#include <QQuickWindow>
#include <QPainter>

namespace qtouch
{

TextView::TextView(QQuickItem* parent):
	QQuickItem(parent)
{
	setFlag(ItemHasContents, true);

	mDoc = new Document(this);
	connect(mDoc, &Document::contentsChanged, this, &TextView::resize);

	connect(this, &QQuickItem::windowChanged, this, &TextView::onWindowChanged);
}

TextView::~TextView()
{
}

/**
 * Set a new document.
 * If the passed document has no parent, ownership is taken.
 * @param doc A document.
 */
void TextView::setDocument(Document* doc)
{
	if (nullptr == doc)
		return;

	if (doc != mDoc)
	{
		// Take ownership
		if (nullptr == doc->parent())
			doc->setParent(this);

		disconnect(mDoc, &Document::contentsChanged, this, &TextView::resize);
		if (this == mDoc->parent()) // mDoc cannot be null
			delete mDoc;

		mDoc = doc;
		connect(mDoc, &Document::contentsChanged, this, &TextView::resize);

		emit documentChanged();
	}
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

void TextView::setDocClipRect(QRectF docClipRect)
{
	if (docClipRect.isValid())
	{
		mDocClipRect = docClipRect;
		/*qDebug() << "mDocClipRect" << mDocClipRect;*/
		mDocDirty = true;
		update();
		emit docClipRectChanged();
	}
}

void TextView::resize()
{
	// IdealWith is defined by the longest line plus margins
	qreal idealWidth = mDoc->getIdealWidth();

	// Calculate scale
	if (mMaxWidth > 0 && idealWidth > mMaxWidth) // Scale down
		mDocScale = mMaxWidth / idealWidth;
	else if (mMinWidth > 0 && idealWidth < mMinWidth) // Scale up
		mDocScale = mMinWidth / idealWidth;
	else
		mDocScale = 1;

	emit docScaleChanged();

	// Note: Its crucial to define the TextWidth before accessing the size().height()
	mDoc->setTextWidth(idealWidth);

	// height is defined by document
	qreal itemHeight = mDoc->size().height();

	if (isVisible())
	{
		/*qDebug() << "New TextPage size:" <<
		         "\n\tidealWidth:" << idealWidth << "*" << mDocScale << "=" << (idealWidth * mDocScale) <<
		         "\n\titemHeight:" << itemHeight << "*" << mDocScale << "=" << (itemHeight * mDocScale);*/

		setWidth(idealWidth * mDocScale);
		setHeight(itemHeight * mDocScale);

		mDocDirty = true;
		update();
	}
}

void TextView::onWindowChanged(QQuickWindow* window)
{
	if (window)
		connect(window, &QQuickWindow::beforeSynchronizing, this, &TextView::onBeforeSynchronizing, Qt::DirectConnection);
}

/**
 * Direct connected to QQuickWindow::beforeSynchronizing and called before rendering of next frame.
 * Despite this function is called in render thread it is safe to access members, because the GUI thread
 * is already blocked.
 */
void TextView::onBeforeSynchronizing()
{
	if (mDocDirty)
	{
		//		qDebug() << this << "Updating image: Visible:" << isVisible();

		mImage.reset(new QImage(width(), height(), QImage::Format_ARGB32_Premultiplied));
		mImage->fill(Qt::transparent);
		//	mImage->fill(Qt::white);

		QPainter p(mImage.get());
		//	p.setRenderHint(QPainter::TextAntialiasing);

		/* FIXME: Text scaling seems to be quite inefficient, at least in Linux. */
		p.scale(mDocScale, mDocScale);

		if (mDocClipRect.isValid())
		{
			/* XXX: Using the visible part of the item to clip the painting.
			 * But this way we need to repaint on scrolling ... */
			QRectF clipRec(mDocClipRect.x() / mDocScale, mDocClipRect.y() / mDocScale, mDocClipRect.width() / mDocScale,
			               mDocClipRect.height() / mDocScale);
			mDoc->drawContents(&p, clipRec);
		}
		else
		{
			mDoc->drawContents(&p);
		}

		mDocDirty = false;
	}
}

QSGNode* TextView::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* /*updatePaintNodeData*/)
{
	QSGSimpleTextureNode* node = static_cast<QSGSimpleTextureNode*>(oldNode);
	if (nullptr == node)
	{
		node = new QSGSimpleTextureNode();
		node->setFlag(QSGNode::OwnedByParent);
		node->setFiltering(QSGTexture::Linear);
	}

	if (mImage)
	{
		//		qDebug() << this << "Updating texture: Visible:" << isVisible();

		mTexture.reset(window()->createTextureFromImage(*mImage));
		if (mTexture)
		{
			node->setTexture(mTexture.data());
			node->setRect(QRectF(0, 0, mImage->width(), mImage->height()));
		}
		else
		{
			qCritical() << this << "Unable to create texture";
		}
	}
	return node;
}

} /* namespace qtouch */
