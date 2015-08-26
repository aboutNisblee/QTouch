/**
 * \file textpage.cpp
 *
 * \date 06.08.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "textpage.hpp"

#include <QImage>
#include <QPainter>
#include <QtMath>
#include <QSGSimpleTextureNode>
#include <QQuickWindow>

#include <QTextBlock>
#include <QTextCursor>
#include <QStringList>

namespace qtouch
{

namespace
{
enum { TITLE_BLOCK, TEXT_BLOCK };
}

TextPage::TextPage(QQuickItem* parent):
	QQuickItem(parent)
{
	setFlag(ItemHasContents, true);

	QTextOption textOption;
	textOption.setAlignment(Qt::AlignJustify);
	textOption.setWrapMode(QTextOption::WordWrap);
	mDoc.setDefaultTextOption(textOption);

	mDoc.setUseDesignMetrics(true);

	mTitleBlockFormat.setLineHeight(200, QTextBlockFormat::ProportionalHeight);
	mTitleBlockFormat.setLeftMargin(10);
	mTitleBlockFormat.setBottomMargin(10);

	mTextBlockFormat.setLineHeight(200, QTextBlockFormat::ProportionalHeight);
	mTextBlockFormat.setAlignment(Qt::AlignJustify);

	// TODO: Let the user choose it via Theme or get it from the system!
	// See: qthelp://org.qt-project.qtgui.542/qtgui/qtextcharformat.html#setFontStyleHint
	mTextCharFormat.setFontFamily("monospace");
	mTextCharFormat.setFontPointSize(10);
	mTextCharFormat.setForeground(QColor("#000"));
	mTextCharFormat.setFontHintingPreference(QFont::PreferVerticalHinting);

	mTitleCharFormat = mTextCharFormat;
	mTitleCharFormat.setFontFamily("sans-serif");
	mTitleCharFormat.setFontPointSize(mTextCharFormat.fontPointSize() * 1.5);
}

TextPage::~TextPage()
{
}

void TextPage::setTitle(const QString& title)
{
	mTitle = title;
	updateDoc();
	if (resize())
		updateImage();

	emit titleChanged();
}

void TextPage::setText(const QString& text)
{
	mText = text;
	updateDoc();
	if (resize())
		updateImage();

	emit textChanged();
}

void TextPage::setAutoWrap(bool enable)
{
	mAutoWrap = enable;
	emit autoWrapChanged();
}

void TextPage::setTextMargin(qreal textMargin)
{
	mDoc.setDocumentMargin(textMargin);
	emit textMarginChanged();
}

void TextPage::setMaxWidth(qreal maxWidth)
{
	mMaxWidth = maxWidth;
	if (maxWidth > 0 && resize())
		updateImage();

	emit maxWidthChanged();
}

void TextPage::setMaxHeight(qreal maxHeight)
{
	mMaxHeight = maxHeight;
	if (maxHeight > 0 && resize())
		updateImage();

	emit maxHeightChanged();
}

/*
 * TODO: Should also remove duplicated spaces from the given text when
 * autoWrap is disabled??
 */
void TextPage::updateDoc()
{
	mDoc.clear();
	QTextCursor c(&mDoc);

	c.setBlockFormat(mTitleBlockFormat);
	c.insertText((mAutoWrap) ? mTitle.simplified() : mTitle, mTitleCharFormat);

	c.insertBlock();

	c.setBlockFormat(mTextBlockFormat);
	c.insertText((mAutoWrap) ? mText.simplified() : mText, mTextCharFormat);
}

bool TextPage::resize()
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
		//		qDebug() << "New TextPage size:" <<
		//		         "\n\titemWidth:" << itemWidth << "*" << mTextScale << "=" << (itemWidth * mTextScale) <<
		//		         "\n\titemHeight:" << itemHeight << "*" << mTextScale << "=" << (itemHeight * mTextScale);

		setWidth(itemWidth * mTextScale);
		setHeight(itemHeight * mTextScale);

		return true;
	}

	return false;
}

void TextPage::updateImage()
{
	mImage.reset(new QImage(width(), height(), QImage::Format_ARGB32_Premultiplied));
	mImage->fill(0);

	QPainter p(mImage.get());
	p.scale(mTextScale, mTextScale);
	p.setRenderHint(QPainter::TextAntialiasing);
	mDoc.drawContents(&p);

	update();
}

QSGNode* TextPage::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* /*updatePaintNodeData*/)
{
	QSGSimpleTextureNode* node = static_cast<QSGSimpleTextureNode*>(oldNode);
	if (!node)
	{
		node = new QSGSimpleTextureNode();
		node->setOwnsTexture(true);
		node->setFiltering(QSGTexture::Linear);
	}

	QSGTexture* newTexture = window()->createTextureFromImage(*mImage);
	if (nullptr == newTexture)
		qWarning() << "newTexture is NULL";

	//	qDebug() << "Updating texture to width:" << mImage->width() << "height:" << mImage->height();
	node->setRect(QRectF(0, 0, mImage->width(), mImage->height()));
	node->setTexture(newTexture);

	return node;
}

} /* namespace qtouch */
