/**
 * \file textpage.hpp
 *
 * \date 06.08.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef TEXTPAGE_HPP_
#define TEXTPAGE_HPP_

#include <memory>
#include <QQuickItem>
#include <QRectF>
#include <QTextDocument>
#include <QTextBlockFormat>
#include <QTextCharFormat>

class QImage;

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
class TextPage: public QQuickItem
{
	Q_OBJECT

	Q_PROPERTY(QString title READ getTitle WRITE setTitle NOTIFY titleChanged)
	Q_PROPERTY(QString text READ getText WRITE setText NOTIFY textChanged)

	Q_PROPERTY(qreal textMargin READ getTextMargin WRITE setTextMargin NOTIFY textMarginChanged)

	Q_PROPERTY(qreal maxWidth READ getMaxWidth WRITE setMaxWidth NOTIFY maxWidthChanged)
	Q_PROPERTY(qreal minWidth READ getMinWidth WRITE setMinWidth NOTIFY minWidthChanged)

	Q_PROPERTY(QRectF viewport READ getViewport WRITE setViewport NOTIFY viewportChanged)

public:
	TextPage(QQuickItem* parent = 0);
	virtual ~TextPage();

	inline QString getTitle() const { return mTitle; }
	void setTitle(const QString& title);

	inline QString getText() const { return mText; }
	void setText(const QString& text);

	inline qreal getTextMargin() const { return mDoc.documentMargin(); }
	void setTextMargin(qreal textMargin);

	inline qreal getMaxWidth() const { return mMaxWidth; }
	void setMaxWidth(qreal maxWidth);

	inline qreal getMinWidth() const { return mMinWidth; }
	void setMinWidth(qreal minWidth);

	inline QRectF getViewport() const { return mViewport; }
	void setViewport(QRectF viewport);

signals:
	void titleChanged();
	void textChanged();
	void autoWrapChanged();
	void textMarginChanged();
	void maxWidthChanged();
	void minWidthChanged();
    void viewportChanged();

protected:
	std::unique_ptr<QTextCursor> getTextCursor();
	QTextBlock getFirstTextBlock();

	virtual void initializeDoc();
	virtual bool resize();
	virtual void updateImage();

	virtual QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;

	QString mTitle;
	QString mText;

	QTextDocument mDoc;
	qreal mDocScale = 1;
	std::unique_ptr<QImage> mImage;

	qreal mMaxWidth = 0;
	qreal mMinWidth = 0;

	QRectF mViewport;

	QTextBlockFormat mTextBlockFormat;
	QTextCharFormat mTextCharFormat;
	QTextBlockFormat mTitleBlockFormat;
	QTextCharFormat mTitleCharFormat;
};

} /* namespace qtouch */

#endif /* TEXTPAGE_HPP_ */
