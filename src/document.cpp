/**
 * \file document.cpp
 *
 * \date 08.10.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "document.hpp"

#include <QPainter>
#include <QTextBlock>
#include <QTextCursor>
#include <QDebug>

namespace qtouch
{

Document::Document(QObject* parent) :
	QTextDocument(parent)
{
	//	setUseDesignMetrics(true);

	mTextBlockFormat.setLineHeight(200, QTextBlockFormat::ProportionalHeight);
	mTextBlockFormat.setAlignment(Qt::AlignJustify);

	// TODO: Let the user choose it via Theme or get it from the system!
	// See: qthelp://org.qt-project.qtgui.542/qtgui/qtextcharformat.html#setFontStyleHint
	mTextCharFormat.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	mTextCharFormat.setFontPointSize(20);
	mTextCharFormat.setForeground(QColor("black"));
	mTextCharFormat.setBackground(Qt::transparent);

	mTitleBlockFormat.setLineHeight(200, QTextBlockFormat::ProportionalHeight);
	mTitleBlockFormat.setLeftMargin(10);
	mTitleBlockFormat.setBottomMargin(10);

	mTitleCharFormat.setFont(QFontDatabase::systemFont(QFontDatabase::TitleFont));
	mTitleCharFormat.setFontPointSize(mTextCharFormat.fontPointSize() * 1.5);
	mTitleCharFormat.setForeground(QColor("black"));
	mTitleCharFormat.setBackground(Qt::transparent);
}

Document::~Document()
{
}

void Document::setTitle(const QString& title)
{
	// Remove line breaks; prevent creation of multiple blocks
	mTitle = title.simplified();
	resetText();

	emit titleChanged();
}

void Document::setText(const QString& text)
{
	/* TODO: Should we remove duplicated spaces from the given text? */
	mText = text;
	resetText();

	emit textChanged();
}

/**
 * Returns a QTextCursor that points to the first text block
 * (i.e. skipping the title block).
 * @return A pointer to the newly allocated cursor.
 */
std::unique_ptr<QTextCursor> Document::getTextCursor()
{
	std::unique_ptr<QTextCursor> c(new QTextCursor(this));
	c->movePosition(QTextCursor::NextBlock);
	return c;
}

QTextBlock Document::getFirstTextBlock()
{
	QTextCursor c(this);
	c.movePosition(QTextCursor::NextBlock);
	return c.block();
}

void Document::setTitleBlockFormat(const QTextBlockFormat& format)
{
	mTitleBlockFormat = format;
}

void Document::setTextBlockFormat(const QTextBlockFormat& format)
{
	mTextBlockFormat = format;
}

void Document::setTitleCharFormat(const QTextCharFormat& format)
{
	mTitleCharFormat = format;
}

void Document::setTextCharFormat(const QTextCharFormat& format)
{
	mTextCharFormat = format;
}

qreal Document::getIdealWidth()
{
	qreal backup = textWidth();
	setTextWidth(-1);
	qreal result = idealWidth();
	setTextWidth(backup);
	return result;
}

void Document::resetText()
{
	clear();
	QTextCursor c(this);

	c.setBlockFormat(mTitleBlockFormat);
	c.insertText(mTitle, mTitleCharFormat);

	c.insertBlock();

	c.setBlockFormat(mTextBlockFormat);
	c.insertText(mText, mTextCharFormat);

	Q_ASSERT(blockCount() >= 2);
}

} /* namespace qtouch */
