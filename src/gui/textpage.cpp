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

TextPage::TextPage(QQuickItem* parent):
	QQuickItem(parent)
{
	setFlag(ItemHasContents, true);

	QTextOption textOption;
	textOption.setAlignment(Qt::AlignJustify);
	textOption.setWrapMode(QTextOption::WordWrap);
	mDoc.setDefaultTextOption(textOption);

	//	mDoc.setUseDesignMetrics(true);

	mTextBlockFormat.setLineHeight(200, QTextBlockFormat::ProportionalHeight);
	mTextBlockFormat.setAlignment(Qt::AlignJustify);

	// TODO: Let the user choose it via Theme or get it from the system!
	// See: qthelp://org.qt-project.qtgui.542/qtgui/qtextcharformat.html#setFontStyleHint
	mTextCharFormat.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
	mTextCharFormat.setFontPointSize(14);
	mTextCharFormat.setForeground(QColor("black"));
	//	mTextCharFormat.setBackground(Qt::transparent);
	//	mTextCharFormat.setFontHintingPreference(QFont::PreferVerticalHinting);

	mTitleBlockFormat.setLineHeight(200, QTextBlockFormat::ProportionalHeight);
	mTitleBlockFormat.setLeftMargin(10);
	mTitleBlockFormat.setBottomMargin(10);

	mTitleCharFormat.setFont(QFontDatabase::systemFont(QFontDatabase::TitleFont));
	mTitleCharFormat.setFontPointSize(mTextCharFormat.fontPointSize() * 1.5);

	connect(this, &QQuickItem::windowChanged, this, &TextPage::onWindowChanged);
}

TextPage::~TextPage()
{
}

void TextPage::setTitle(const QString& title)
{
	// Remove line breaks; prevent creation of multiple blocks
	mTitle = title.simplified();
	resetText();

	emit titleChanged();
}

void TextPage::setText(const QString& text)
{
	/* TODO: Should we remove duplicated spaces from the given text? */
	mText = text;
	resetText();

	emit textChanged();
}

void TextPage::setTextMargin(qreal textMargin)
{
	mDoc.setDocumentMargin(textMargin);
	resize();
	emit textMarginChanged();
}

void TextPage::setMaxWidth(qreal maxWidth)
{
	if (maxWidth > 0)
	{
		mMaxWidth = maxWidth;
		resize();
		emit maxWidthChanged();
	}
}

void TextPage::setMinWidth(qreal minWidth)
{
	if (minWidth > 0)
	{
		mMinWidth = minWidth;
		resize();
		emit minWidthChanged();
	}
}

void TextPage::setDocClipRect(QRectF docClipRect)
{
	if (docClipRect.isValid())
	{
		mDocClipRect = docClipRect;
		/*qDebug() << "mDocClipRect" << mDocClipRect;*/
		mImageDirty = true;
		update();
		emit docClipRectChanged();
	}
}

/**
 * Returns a QTextCursor that points to the first text block
 * (i.e. skipping the title block).
 * @return A pointer to the newly allocated cursor.
 */
std::unique_ptr<QTextCursor> TextPage::getTextCursor()
{
	std::unique_ptr<QTextCursor> c(new QTextCursor(&mDoc));
	c->movePosition(QTextCursor::NextBlock);
	return c;
}

QTextBlock TextPage::getFirstTextBlock()
{
	QTextCursor c(&mDoc);
	c.movePosition(QTextCursor::NextBlock);
	return c.block();
}

void TextPage::resetText()
{
	mDoc.clear();
	QTextCursor c(&mDoc);

	c.setBlockFormat(mTitleBlockFormat);
	c.insertText(mTitle, mTitleCharFormat);

	c.insertBlock();

	c.setBlockFormat(mTextBlockFormat);
	c.insertText(mText, mTextCharFormat);

	Q_ASSERT(mDoc.blockCount() >= 2);

	resize();
}

void TextPage::resize()
{
	mDoc.setTextWidth(-1);

	// IdealWith is defined by the longest line plus margins
	qreal idealWidth = mDoc.idealWidth();

	// Calculate scale
	if (mMaxWidth > 0 && idealWidth > mMaxWidth) // Scale down
		mDocScale = mMaxWidth / idealWidth;
	else if (mMinWidth > 0 && idealWidth < mMinWidth) // Scale up
		mDocScale = mMinWidth / idealWidth;
	else
		mDocScale = 1;

	emit docScaleChanged();

	// Note: Its crucial to define the TextWidth before accessing the size().height()
	mDoc.setTextWidth(idealWidth);

	// height is defined by document
	qreal itemHeight = mDoc.size().height();

	if ((idealWidth * mDocScale) != width() || (itemHeight * mDocScale) != height())
	{
		/*qDebug() << "New TextPage size:" <<
		         "\n\tidealWidth:" << idealWidth << "*" << mDocScale << "=" << (idealWidth * mDocScale) <<
		         "\n\titemHeight:" << itemHeight << "*" << mDocScale << "=" << (itemHeight * mDocScale);*/

		setWidth(idealWidth * mDocScale);
		setHeight(itemHeight * mDocScale);

		mImage.reset(new QImage(width(), height(), QImage::Format_ARGB32_Premultiplied));

		mImageDirty = true;
	}
}

void TextPage::onWindowChanged(QQuickWindow* window)
{
	if (window)
		connect(window, &QQuickWindow::beforeSynchronizing, this, &TextPage::onBeforeSynchronizing, Qt::DirectConnection);
}

/**
 * Direct connected to QQuickWindow::beforeSynchronizing and called before rendering of next frame.
 * Despite this function is called in render thread it is safe to access members, because the GUI thread
 * is already blocked.
 */
void TextPage::onBeforeSynchronizing()
{
	if (mImageDirty)
	{
		if (!mImage)
			return;

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
			mDoc.drawContents(&p, clipRec);
		}
		else
		{
			mDoc.drawContents(&p);
		}

		mImageDirty = false;
	}
}

QSGNode* TextPage::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* /*updatePaintNodeData*/)
{
	QSGSimpleTextureNode* node = static_cast<QSGSimpleTextureNode*>(oldNode);
	if (nullptr == node)
	{
		node = new QSGSimpleTextureNode();
		node->setFlag(QSGNode::OwnedByParent);

		node->setFiltering(QSGTexture::Linear);
		// XXX: For what its worth, when the old texture isn't deleted when another one is set??
		/*node->setOwnsTexture(true);*/
	}

	if (node->texture() != nullptr)
		delete node->texture();
	QSGTexture* newTexture = window()->createTextureFromImage(*mImage);
	if (newTexture != nullptr)
	{
		//	qDebug() << "Updating texture to width:" << mImage->width() << "height:" << mImage->height();
		node->setRect(QRectF(0, 0, mImage->width(), mImage->height()));
		node->setTexture(newTexture);
	}
	else
	{
		qCritical() << "Unable to create texture";
	}

	return node;
}

} /* namespace qtouch */
