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
 * \file textview.hpp
 *
 * \date 06.08.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef TEXTVIEW_HPP_
#define TEXTVIEW_HPP_

#include <memory>
#include <QQuickItem>
#include <QRectF>

#include "document.hpp"

class QImage;
class QSGTexture;

namespace qtouch
{

class TextFormat;

/** Qt Quick item that is able to draw text.
 * This item is mimics the behavior of a TextArea but allows full control
 * over the internal QTextDocument. It is used in the LessonPreview item to
 * show the preview of the Lesson text and serves as base class for the view
 * of the TrainingWidget.\n
 * Because the position of the line breaks is probably relevant to the
 * lesson, the lesson text dictates the line wrapping. For the item to be able to
 * scale the text to the right size, there are two properties: maxWidth and minWidth.
 * When the ideal width for the given font size is bigger than maxWidth, the text is
 * scaled down. When it's smaller than minWidth it's scaled up.\n
 * The height of the whole document depends on the count of lines and the calculated
 * scale (use a Flickable/ScrollView).
 */
class TextView: public QQuickItem
{
	Q_OBJECT

	Q_PROPERTY(Document* document READ getDocument WRITE setDocument NOTIFY documentChanged)
	Q_PROPERTY(qreal maxWidth READ getMaxWidth WRITE setMaxWidth NOTIFY maxWidthChanged)
	Q_PROPERTY(qreal minWidth READ getMinWidth WRITE setMinWidth NOTIFY minWidthChanged)
	Q_PROPERTY(qreal docScale READ getDocScale NOTIFY docScaleChanged)
	Q_PROPERTY(QRectF docClipRect READ getDocClipRect WRITE setDocClipRect NOTIFY docClipRectChanged)

public:
	TextView(QQuickItem* parent = 0);
	virtual ~TextView();

	inline Document* getDocument() const { return mDoc; }
	void setDocument(Document* doc);

	void setOwnsDocument(bool ownsDoc);

	inline qreal getMaxWidth() const { return mMaxWidth; }
	void setMaxWidth(qreal maxWidth);

	inline qreal getMinWidth() const { return mMinWidth; }
	void setMinWidth(qreal minWidth);

	inline qreal getDocScale() const { return mDocScale; }

	inline QRectF getDocClipRect() const { return mDocClipRect; }
	void setDocClipRect(QRectF docClipRect);

signals:
	void documentChanged();
	void maxWidthChanged();
	void minWidthChanged();
	void docScaleChanged();
	void docClipRectChanged();

protected:
	virtual void resize();

	void onWindowChanged(QQuickWindow*);
	virtual void onBeforeSynchronizing();
	virtual QSGNode* updatePaintNode(QSGNode*, UpdatePaintNodeData*) Q_DECL_OVERRIDE;

	Document* mDoc;

	bool mOwnsDoc = true;

	bool mDocDirty = false;

	qreal mDocScale = 1;
	std::unique_ptr<QImage> mImage;

	qreal mMaxWidth = 0;
	qreal mMinWidth = 0;

	QRectF mDocClipRect;

	QScopedPointer<QSGTexture> mTexture;
};

} /* namespace qtouch */

#endif /* TEXTVIEW_HPP_ */
