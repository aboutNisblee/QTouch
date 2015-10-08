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
 * \file trainingwidget.hpp
 *
 * \date 11.08.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef TRAININGWIDGET_HPP_
#define TRAININGWIDGET_HPP_

#include <QTextCursor>

#include "textview.hpp"

namespace qtouch
{

class TrainingWidget: public TextView
{
	Q_OBJECT

	Q_PROPERTY(Qt::Key escapeKey MEMBER mEscKey NOTIFY escapeKeyChanged)
	Q_PROPERTY(qreal progress READ getProgress NOTIFY progressChanged)
	Q_PROPERTY(QRectF cursorRectangle READ getCursorRectangle NOTIFY cursorRectangleChanged)

	/* The number of the line the cursor is currently in starting at 0. */
	Q_PROPERTY(int activeLineNumber READ getActiveLineNumber NOTIFY activeLineNumberChanged)
	/* Current position of the cursor in the currently active line starting at 0. */
	Q_PROPERTY(int cursorPosition READ getCursorPosition NOTIFY cursorPositionChanged)

public:
	TrainingWidget(QQuickItem* parent = 0);
	virtual ~TrainingWidget();

	inline qreal getProgress() const { return mProgress; }
	inline QRectF getCursorRectangle() const { return mCursorRectangle; }
	int getActiveLineNumber() const;
	int getCursorPosition() const;

public slots:
	void reset();

signals:
	void escape();
	void escapeKeyChanged();
	void progressChanged();
	void cursorRectangleChanged();
	void activeLineNumberChanged();
	void cursorPositionChanged();

protected:
	virtual void keyPressEvent(QKeyEvent*) Q_DECL_OVERRIDE;

	virtual void onBeforeSynchronizing() Q_DECL_OVERRIDE;
	virtual QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) Q_DECL_OVERRIDE;

private:
	void connectToDocument();
	void configureTextFormat();
	void resetCursor();
	void updateProgress(qreal percent);

	Qt::Key mEscKey = Qt::Key_Escape;
	std::unique_ptr<QTextCursor> mCursor;

	QTextCharFormat mCorrectTextCharFormat;
	QTextCharFormat mFailureTextCharFormat;

	QRectF mCursorRectangle;
	qreal mProgress = 0;
	bool mCursorDirty = false;
};

} /* namespace qtouch */

#endif /* TRAININGWIDGET_HPP_ */
