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

#include "textformat.hpp"

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
}

TextPage::~TextPage()
{
}

void TextPage::setTitle(const QString& title)
{
	// Remove line breaks; prevent creation of multiple blocks
	mTitle = title.simplified();
	initializeDoc();
	resize();
	updateImage();

	emit titleChanged();
}

void TextPage::setText(const QString& text)
{
	/* TODO: Should we remove duplicated spaces from the given text? */
	mText = text;
	initializeDoc();
	resize();
	updateImage();

	emit textChanged();
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
	{
		/*qDebug() << "mMaxWidth" << mMaxWidth;*/
		updateImage();
	}

	emit maxWidthChanged();
}

void TextPage::setMinWidth(qreal minWidth)
{
	mMinWidth = minWidth;
	if (minWidth > 0 && resize())
	{
		/*qDebug() << "mMinWidth" << mMinWidth;*/
		updateImage();
	}
	emit minWidthChanged();
}

void TextPage::setViewport(QRectF viewport)
{
	if (viewport.isValid())
	{
		mViewport = viewport;
		/*qDebug() << "mViewport" << mViewport;*/
		updateImage();
	}
	emit viewportChanged();
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

void TextPage::initializeDoc()
{
	mDoc.clear();
	QTextCursor c(&mDoc);

	c.setBlockFormat(mTitleBlockFormat);
	c.insertText(mTitle, mTitleCharFormat);

	c.insertBlock();

	c.setBlockFormat(mTextBlockFormat);
	c.insertText(mText, mTextCharFormat);

	Q_ASSERT(mDoc.blockCount() >= 2);
}

bool TextPage::resize()
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

		return true;
	}

	return false;
}

/* TODO: Minimize calls to updateImage! */
void TextPage::updateImage()
{
	if (!mImage)
		return;

	mImage->fill(Qt::transparent);
	//	mImage->fill(Qt::white);

	QPainter p(mImage.get());
	//	p.setRenderHint(QPainter::TextAntialiasing);

	/* FIXME: Text scaling seems to be quite inefficient, at least in Linux. */
	p.scale(mDocScale, mDocScale);

	if (mViewport.isValid())
	{
		/* XXX: Using the visible part of the item to clip the painting.
		 * But this way we need to repaint on scrolling ... */
		QRectF clipRec(mViewport.x() / mDocScale, mViewport.y() / mDocScale, mViewport.width() / mDocScale, mViewport.height() / mDocScale);
		mDoc.drawContents(&p, clipRec);
	}
	else
	{
		mDoc.drawContents(&p);
	}

	update();
}

QSGNode* TextPage::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* /*updatePaintNodeData*/)
{
	QSGSimpleTextureNode* node = static_cast<QSGSimpleTextureNode*>(oldNode);
	if (!node)
	{
		node = new QSGSimpleTextureNode();
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
