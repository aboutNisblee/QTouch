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

#include "textformat.hpp"
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

	//	connect(&mDoc, &QTextDocument::cursorPositionChanged, this, &TrainingWidget::updateCursorRectangle);
}

TrainingWidget::~TrainingWidget()
{
}

void TrainingWidget::reset()
{
	initializeDoc();
	resetCursor();
	updateImage();
}

void TrainingWidget::resetCursor()
{
	mCursor = getTextCursor();
	mDoc.clearUndoRedoStacks();
}

void TrainingWidget::updateCursorRectangle(const QTextCursor& /*cursor*/)
{
	qDebug() << "cursorPositionChanged";
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

	bool dirty = false;

	//	qDebug() << "Input: (" << QStringLiteral("0x%1").arg(keyEvent->key(), 8, 16, QChar('0')) << ")" << keyEvent->text();

	// Catch backspace
	if (keyEvent->key() == Qt::Key_Backspace)
	{
		if (mDoc.isUndoAvailable())
		{
			mCursor->clearSelection();
			mDoc.undo(mCursor.get());
			mCursor->movePosition(QTextCursor::PreviousCharacter);
			dirty = true;
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
				qDebug() << "Line break!";
				bool lineBreakWithSpace = false;
				bool lineBreakWithReturn = true;

				if ((lineBreakWithReturn && (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter))
				        || (lineBreakWithSpace && keyEvent->key() == Qt::Key_Space))
				{
					mCursor->movePosition(QTextCursor::NextBlock);
					dirty = true;
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
					dirty = true;
				}
				else
				{
					mCursor->movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
					mCursor->insertText(cTyped, mFailureTextCharFormat);
					dirty = true;
				}
			}
		} // for-loop

		mCursor->endEditBlock();
	}
	else
	{
		qDebug() << "Key text empty";
	}

	if (dirty)
		updateImage();
}

QSGNode* TrainingWidget::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* /*updatePaintNodeData*/)
{
	QSGNode* rootNode = TextPage::updatePaintNode(oldNode, nullptr);

	// Get the top left edge of the current text block
	QPointF blockTopLeft = mDoc.documentLayout()->blockBoundingRect(mCursor->block()).topLeft();
	// Get the current text line from the text layout of the current block
	QTextLine textLine = mCursor->block().layout()->lineForTextPosition(mCursor->positionInBlock());
	// Calculate the unscaled top left position of the cursor
	QPointF topLeft(blockTopLeft.x() + textLine.cursorToX(mCursor->positionInBlock()), blockTopLeft.y() + textLine.y());
	// And the bottom right position
	QPointF bottomRight(topLeft.x() + 1, topLeft.y() + textLine.height());
	// Use the points to span the cursor rectangle
	QRectF cursorRectangle(topLeft * mDocScale, bottomRight * mDocScale);

	QSGSimpleRectNode* cursorNode = static_cast<QSGSimpleRectNode*>(rootNode->firstChild());
	if (nullptr == cursorNode)
	{
		cursorNode = new QSGSimpleRectNode(cursorRectangle, mCursor->charFormat().foreground().color());
		rootNode->appendChildNode(cursorNode);
	}
	else
	{
		cursorNode = static_cast<QSGSimpleRectNode*>(rootNode->firstChild());
		cursorNode->setRect(cursorRectangle);
		cursorNode->setColor(mCursor->charFormat().foreground().color());
	}

	return rootNode;
}

} /* namespace qtouch */
