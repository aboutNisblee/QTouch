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
#include <QTextDocument>

#include <QTextBlockFormat>
#include <QTextCharFormat>

class QImage;

namespace qtouch
{

/** Qt Quick item that is able to draw text.
 * This item is mimics the behavior of a TextArea but allows full control
 * over the internal QTextDocument. It is used in the LessonPreview item to
 * show the preview of the Lesson text and serves as base class for the view
 * of the TrainingWidget.\n
 * There are two distinct layout modes this item can operate in.\n
 * When the autoWrap property is disabled (default), the given text defines
 * the line wrapping ('\\n'). The width of the item is then set to the ideal
 * width of the text (i.e. the width of the longest line plus margins).
 * The width property of Item can then be used to determine the minimum needed
 * width.\n
 * If autoWrap is enabled all newline characters are deleted from the given
 * string and the text is automatically wrapped at word boundaries. Note that
 * in this mode a maximum width is needed for the document to be able to wrap
 * the text. This is specified by the maxWidth property.
 */
class TextPage: public QQuickItem
{
	Q_OBJECT

	Q_PROPERTY(QString title READ getTitle WRITE setTitle NOTIFY titleChanged)
	Q_PROPERTY(QString text READ getText WRITE setText NOTIFY textChanged)

	Q_PROPERTY(bool autoWrap READ isAutoWrap WRITE setAutoWrap NOTIFY autoWrapChanged)
	Q_PROPERTY(qreal textMargin READ getTextMargin WRITE setTextMargin NOTIFY textMarginChanged)
	Q_PROPERTY(qreal maxWidth READ getMaxWidth WRITE setMaxWidth NOTIFY maxWidthChanged)
	Q_PROPERTY(qreal maxHeight READ getMaxHeight WRITE setMaxHeight NOTIFY maxHeightChanged)

public:
	TextPage(QQuickItem* parent = 0);
	virtual ~TextPage();

	inline QString getTitle() const { return mTitle; }
	void setTitle(const QString& title);

	inline QString getText() const { return mText; }
	void setText(const QString& text);

	inline bool isAutoWrap() const { return mAutoWrap; }
	void setAutoWrap(bool enable);

	inline qreal getTextMargin() const { return mDoc.documentMargin(); }
	void setTextMargin(qreal textMargin);

	inline qreal getMaxWidth() const { return mMaxWidth; }
	void setMaxWidth(qreal maxWidth);

	inline qreal getMaxHeight() const { return mMaxHeight; }
	void setMaxHeight(qreal maxHeight);

signals:
	void titleChanged();
	void textChanged();
	void autoWrapChanged();
	void textMarginChanged();
	void maxWidthChanged();
	void maxHeightChanged();

protected:
	virtual void updateDoc();
	virtual bool resize();
	virtual void updateImage();

	virtual QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;

	QString mTitle;
	QString mText;

	QTextDocument mDoc;
	std::unique_ptr<QImage> mImage;

	bool mAutoWrap = false;

	qreal mMaxWidth = 0;
	qreal mMaxHeight = 0;

	QTextBlockFormat mTitleBlockFormat;
	QTextBlockFormat mTextBlockFormat;

	QTextCharFormat mTitleCharFormat;
	QTextCharFormat mTextCharFormat;
};

} /* namespace qtouch */

#endif /* TEXTPAGE_HPP_ */
