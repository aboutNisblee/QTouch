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

#include "trainingwidget.hpp"

#include <QTextBlock>
#include <QSGSimpleRectNode>
#include <QPointF>
#include <QRectF>
#include <QAbstractTextDocumentLayout>

#include "utils/exceptions.hpp"

namespace qtouch
{

TrainingWidget::TrainingWidget(QQuickItem* parent) :
	TextPage(parent)
{
	mCursor = std::unique_ptr<QTextCursor>(new QTextCursor(&mDoc));

	// Use standard char format for correctly typed chars
	mCorrectTextCharFormat.merge(mTextCharFormat);

	// Copy and modify standard char format for failures
	mFailureTextCharFormat.merge(mTextCharFormat);
	mFailureTextCharFormat.setFontUnderline(true);
	mFailureTextCharFormat.setUnderlineColor(QColor("red"));

	// Modify standard char format for unwritten text
	mTextCharFormat.setForeground(QColor("gray"));

	connect(this, &TextPage::titleChanged, this, &TrainingWidget::resetCursor);
	connect(this, &TextPage::textChanged, this, &TrainingWidget::resetCursor);

	/*connect(&mDoc, &QTextDocument::cursorPositionChanged, this, &TrainingWidget::updateCursorRectangle);*/
}

TrainingWidget::~TrainingWidget()
{
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
	resetText();
	resetCursor();
	mDocDirty = true;
}

void TrainingWidget::resetCursor()
{
	mCursor = getTextCursor();
	mDoc.clearUndoRedoStacks();
	emit activeLineNumberChanged();
	emit cursorPositionChanged();
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
		if (mDoc.isUndoAvailable())
		{
			int lineNumber = mCursor->blockNumber();

			mCursor->clearSelection();
			mDoc.undo(mCursor.get());
			mCursor->movePosition(QTextCursor::PreviousCharacter);

			// If block number changed while undo, inform clients
			if (mCursor->blockNumber() != lineNumber)
				emit activeLineNumberChanged();

			cursorMoved = true;
		}
	}
	else if (!keyEvent->text().isEmpty())
	{
		// Extract reference text from mText variable
		int refStart = mCursor->position() - mTitle.size() - 1;
		QString refText = mText.mid(refStart, keyEvent->text().size());

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
				bool lineBreakWithReturn = false;

				if ((lineBreakWithReturn && (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter))
				        || (lineBreakWithSpace && keyEvent->key() == Qt::Key_Space))
				{
					mCursor->movePosition(QTextCursor::NextBlock);
					emit activeLineNumberChanged();

					cursorMoved = true;
				}
				else
				{
					qDebug() << "Action needed";
				}
			}
			else
			{
				const QChar cTyped = keyEvent->text().at(i);
				const QChar cRef = refText.at(i);

				if (!cTyped.isPrint()) // Filter unprintable characters
				{
					qDebug() << "Unprintable character";
				}
				else if (cTyped == cRef)
				{
					// Select next char in document
					mCursor->movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
					// and set format
					mCursor->insertText(cRef, mCorrectTextCharFormat);

					cursorMoved = true;
				}
				else
				{
					mCursor->movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
					mCursor->insertText(cTyped, mFailureTextCharFormat);

					cursorMoved = true;
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

		updateProgress((mCursor->position() - mTitle.size() - 1) / static_cast<qreal>(mText.size()));
		mDocDirty = true;
		mCursorDirty = true;
		update();
	}
}

void TrainingWidget::onBeforeSynchronizing()
{
	/* Also need to update on dirty image to correct the position of the cursor after resize. */
	if (mDocDirty || mCursorDirty)
	{
		// Get the top left edge of the current text block
		QPointF blockTopLeft = mDoc.documentLayout()->blockBoundingRect(mCursor->block()).topLeft();
		// Get the current text line from the text layout of the current block
		QTextLine textLine = mCursor->block().layout()->lineForTextPosition(mCursor->positionInBlock());
		// Calculate the unscaled top left position of the cursor
		QPointF topLeft(blockTopLeft.x() + textLine.cursorToX(mCursor->positionInBlock()), blockTopLeft.y() + textLine.y());
		// And the bottom right position
		QPointF bottomRight(topLeft.x() + 1, topLeft.y() + textLine.height());

		// Use the points to span the cursor rectangle
		mCursorRectangle.setTopLeft(topLeft * mDocScale);
		mCursorRectangle.setBottomRight(bottomRight * mDocScale);

		if (mCursorDirty)
			emit cursorRectangleChanged();

		mCursorDirty = false;
	}

	TextPage::onBeforeSynchronizing();
}

QSGNode* TrainingWidget::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* /*updatePaintNodeData*/)
{
	QSGNode* textPageNode = TextPage::updatePaintNode(oldNode, nullptr);

	if (textPageNode != nullptr)
	{
		QSGSimpleRectNode* cursorNode = static_cast<QSGSimpleRectNode*>(textPageNode->firstChild());
		if (nullptr == cursorNode)
		{
			cursorNode = new QSGSimpleRectNode(mCursorRectangle, mCursor->charFormat().foreground().color());
			textPageNode->appendChildNode(cursorNode);
			cursorNode->setFlag(QSGNode::OwnedByParent);
		}
		else
		{
			cursorNode->setRect(mCursorRectangle);
			cursorNode->setColor(mCursor->charFormat().foreground().color());
		}
	}
	return textPageNode;
}

} /* namespace qtouch */
