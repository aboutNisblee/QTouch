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
#include <QQuickPaintedItem>
#include <QColor>

#include "document.hpp"

namespace qtouch
{

class Border: public QObject
{
	Q_OBJECT
	Q_PROPERTY(QColor color MEMBER mColor NOTIFY colorChanged)
	Q_PROPERTY(int width MEMBER mWidth NOTIFY widthChanged)
public:
	explicit Border(QObject* parent = nullptr) : QObject(parent) {}
	virtual ~Border() {}
signals:
	void colorChanged();
	void widthChanged();
private:
	QColor mColor;
	int mWidth = 0;
};

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
class TextView: public QQuickPaintedItem
{
	Q_OBJECT

	Q_PROPERTY(qtouch::Document* document READ getDocument WRITE setDocument NOTIFY documentChanged)
	Q_PROPERTY(qreal maxWidth READ getMaxWidth WRITE setMaxWidth NOTIFY maxWidthChanged)
	Q_PROPERTY(qreal minWidth READ getMinWidth WRITE setMinWidth NOTIFY minWidthChanged)
	Q_PROPERTY(qtouch::Border* border READ getBorder CONSTANT)

public:
	TextView(QQuickItem* parent = nullptr);
	virtual ~TextView();

	inline Document* getDocument() const { return mDoc; }
	void setDocument(Document* doc);

	inline qreal getMaxWidth() const { return mMaxWidth; }
	void setMaxWidth(qreal maxWidth);

	inline qreal getMinWidth() const { return mMinWidth; }
	void setMinWidth(qreal minWidth);

	inline Border* getBorder() const { return mBorder; }

signals:
	void documentChanged();
	void maxWidthChanged();
	void minWidthChanged();
	void docScaleChanged();
	void docClipRectChanged();

protected:
	virtual void resize();

	virtual void paint(QPainter* painter) Q_DECL_OVERRIDE;

	Document* mDoc;

	qreal mMaxWidth = 0;
	qreal mMinWidth = 0;
	Border* mBorder;
};

} /* namespace qtouch */

#endif /* TEXTVIEW_HPP_ */
