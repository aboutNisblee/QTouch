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
 * \file trainingwidget.cpp
 *
 * \date 11.08.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include <gui/trainingwidget.hpp>

#include <QTextCursor>
#include <QTextBlock>
#include <QPointF>
#include <QRectF>
#include <QPainter>
#include <QAbstractTextDocumentLayout>
#include <QTimer>

#include "utils/exceptions.hpp"

namespace qtouch
{

TrainingWidget::TrainingWidget(QQuickItem* parent) :
	TextView(parent)
{
	connectToDocument();
	configureTextFormat();

	mCursor = mDoc->getTextCursor();

	mBlinkTimer = new QTimer(this);
	mBlinkTimer->setInterval(500);
	// TODO: Make me configurable!
	mBlinkTimer->setSingleShot(false);
	mBlinkTimer->start();
	connect(mBlinkTimer, &QTimer::timeout, [&]
	{
		mCursorVisible = !mCursorVisible;
		updateCursorRect();
		update();
	});

	connect(this, &TextView::documentChanged, this, &TrainingWidget::connectToDocument);
	connect(this, &TextView::documentChanged, this, &TrainingWidget::configureTextFormat);
}

TrainingWidget::~TrainingWidget()
{
}

/**
 * Set type recorder.
 * If passed object hasn't a parent, the ownership is taken.
 * @param recorder A Recorder.
 */
void TrainingWidget::setRecoder(Recorder* recorder)
{
	if (nullptr == recorder)
		return;

	if (recorder != mRecorder)
	{
		// Take ownership
		if (nullptr == recorder->parent())
			recorder->setParent(this);

		//		disconnect(mTC, &Document::contentsChanged, this, &TextView::resize);
		if (mRecorder && this == mRecorder->parent())
			delete mRecorder;

		mRecorder = recorder;
		//		connect(mRecorder, &Recorder::show, this, &TextView::resize);

		emit recorderChanged();
	}
}

int TrainingWidget::getActiveLineNumber() const
{
	return mCursor->blockNumber() - 1;
}

int TrainingWidget::getCursorPosition() const
{
	return mCursor->positionInBlock();
}

void TrainingWidget::reset()
{
	mDoc->resetText();
	resetCursor();
	if (mRecorder)
		mRecorder->reset();
}

void TrainingWidget::showHint()
{
	qDebug() << "Show hint";
}

void TrainingWidget::hideHint()
{
	qDebug() << "Hide hint";
}

void TrainingWidget::connectToDocument()
{
	connect(mDoc, &Document::titleChanged, this, &TrainingWidget::resetCursor);
	connect(mDoc, &Document::textChanged, this, &TrainingWidget::resetCursor);
}

void TrainingWidget::configureTextFormat()
{
	// Use standard char format for correctly typed chars
	mCorrectTextCharFormat.merge(mDoc->getTextCharFormat());

	// Copy and modify standard char format for failures
	mFailureTextCharFormat.merge(mDoc->getTextCharFormat());
	mFailureTextCharFormat.setFontUnderline(true);
	mFailureTextCharFormat.setUnderlineColor(QColor("red"));

	// Modify standard char format for unwritten text
	QTextCharFormat standardFormat = mDoc->getTextCharFormat();
	standardFormat.setForeground(QColor("gray"));
	mDoc->setTextCharFormat(standardFormat);
}

void TrainingWidget::resetCursor()
{
	mCursor = mDoc->getTextCursor();
	mDoc->clearUndoRedoStacks();
	emit activeLineNumberChanged();
	emit cursorPositionChanged();
}

void TrainingWidget::updateCursorRect()
{
	// Get the top left edge of the current text block
	QPointF blockTopLeft = mDoc->documentLayout()->blockBoundingRect(mCursor->block()).topLeft();
	// Get the current text line from the text layout of the current block
	QTextLine textLine = mCursor->block().layout()->lineForTextPosition(mCursor->positionInBlock());
	// Calculate the unscaled top left position of the cursor
	QPointF topLeft(blockTopLeft.x() + textLine.cursorToX(mCursor->positionInBlock()), blockTopLeft.y() + textLine.y());
	// And the bottom right position
	QPointF bottomRight(topLeft.x() + 1, topLeft.y() + textLine.height());

	// Use the points to span the cursor rectangle
	mCursorRectangle.setTopLeft(topLeft);
	mCursorRectangle.setBottomRight(bottomRight);

	emit cursorRectangleChanged();
}

void TrainingWidget::updateProgress(qreal percent)
{
	mProgress = percent;
	emit progressChanged();
}

void TrainingWidget::keyPressEvent(QKeyEvent* keyEvent)
{
	// Qt::WA_KeyCompression should be disabled
	Q_ASSERT(keyEvent->count() == 1);

	// Catch configured escape key
	if (keyEvent->modifiers() == Qt::NoModifier && keyEvent->key() == mEscKey)
	{
		keyEvent->setAccepted(true);
		emit escape();
		return;
	}

	bool cursorMoved = false;

	//	qDebug() << "Input: (" << QStringLiteral("0x%1").arg(keyEvent->key(), 8, 16, QChar('0')) << ")" << keyEvent->text();

	// Catch backspace
	if (keyEvent->key() == Qt::Key_Backspace)
	{
		if (mDoc->isUndoAvailable())
		{
			int lineNumber = mCursor->blockNumber();

			// FIXME: This is all crap!!

			mCursor->movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
			QString deleted = mCursor->selectedText();
			mCursor->movePosition(QTextCursor::PreviousCharacter);

			mDoc->undo(mCursor.get());
			mCursor->movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
			QString ref = mCursor->selectedText();
			mCursor->clearSelection();

			if (mRecorder)
			{
				if (deleted == ref)
					mRecorder->unhit();
				else
					mRecorder->unmiss();
			}

			// If block number changed while undo, inform clients
			if (mCursor->blockNumber() != lineNumber)
				emit activeLineNumberChanged();

			cursorMoved = true;
		}
	}
	else if (!keyEvent->text().isEmpty())
	{
		// Extract reference text from mText variable
		int refStart = mCursor->position() - mDoc->getTitle().size() - 1;
		QString refText = mDoc->getText().mid(refStart, keyEvent->text().size());

		// If no text left, end is reached.
		if (refText.isEmpty())
		{
			qDebug() << "Finished!";
			return;
		}

		mCursor->beginEditBlock();
		for (int i = 0; i < refText.size(); ++i)
		{
			// Pay attention with comparison when input method produce multiple chars at once!

			mCursor->clearSelection();

			// Line break handling
			if (mCursor->atBlockEnd())
			{
				// XXX: Make me configurable!
				bool lineBreakWithSpace = true;
				bool lineBreakWithReturn = true;

				if ((lineBreakWithReturn && (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter))
				        || (lineBreakWithSpace && keyEvent->key() == Qt::Key_Space))
				{
					mCursor->movePosition(QTextCursor::NextBlock);
					emit activeLineNumberChanged();

					if (mRecorder)
						mRecorder->hit();

					cursorMoved = true;
				}
				else
				{
					qDebug() << "Action needed";
					if (mRecorder)
						mRecorder->miss();
				}
			}
			else
			{
				const QChar cTyped = keyEvent->text().at(i);
				const QChar cRef = refText.at(i);

				if (!cTyped.isPrint()) // Filter unprintable characters
				{
					qDebug() << "Unprintable character";
					if (mRecorder)
						mRecorder->miss();
				}
				else if (cTyped == cRef)
				{
					// Select next char in document
					mCursor->movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
					// and set format
					mCursor->insertText(cRef, mCorrectTextCharFormat);

					cursorMoved = true;
					if (mRecorder)
						mRecorder->hit();
				}
				else
				{
					mCursor->movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
					mCursor->insertText(cTyped, mFailureTextCharFormat);

					cursorMoved = true;
					if (mRecorder)
						mRecorder->miss();
				}
			}
		} // for-loop

		mCursor->endEditBlock();
	}
	else
	{
		qDebug() << "Key text empty";
	}

	if (cursorMoved)
	{
		emit cursorPositionChanged();

		updateCursorRect();
		updateProgress((mCursor->position() - mDoc->getTitle().size() - 1) / static_cast<qreal>(mDoc->getText().size()));
		update();
	}
}

void TrainingWidget::paint(QPainter* painter)
{
	TextView::paint(painter);

	if (mCursorVisible)
		painter->fillRect(mCursorRectangle, mCursor->charFormat().foreground().color());
}

} /* namespace qtouch */
