/**
 * \file document.hpp
 *
 * \date 08.10.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef DOCUMENT_HPP_
#define DOCUMENT_HPP_

#include <memory>

#include <QObject>
#include <QTextDocument>
#include <QTextBlockFormat>
#include <QTextCharFormat>

namespace qtouch
{

class Document: public QTextDocument
{
	Q_OBJECT

	Q_PROPERTY(QString title READ getTitle WRITE setTitle NOTIFY titleChanged)
	Q_PROPERTY(QString text READ getText WRITE setText NOTIFY textChanged)

public:
	Document(QObject* parent = nullptr);
	virtual ~Document();

	inline QString getTitle() const { return mTitle; }
	void setTitle(const QString& title);

	inline QString getText() const { return mText; }
	void setText(const QString& text);

	void setTitleBlockFormat(const QTextBlockFormat& format);
	inline QTextBlockFormat getTitleBlockFormat() const { return mTitleBlockFormat; }
	void setTextBlockFormat(const QTextBlockFormat& format);
	inline QTextBlockFormat getTextBlockFormat() const { return mTextBlockFormat; }

	void setTitleCharFormat(const QTextCharFormat& format);
	inline QTextCharFormat getTitleCharFormat() const { return mTitleCharFormat; }
	void setTextCharFormat(const QTextCharFormat& format);
	inline QTextCharFormat getTextCharFormat() const { return mTextCharFormat; }

	qreal getIdealWidth();

	std::unique_ptr<QTextCursor> getTextCursor();
	QTextBlock getFirstTextBlock();

	void resetText();

signals:
	void titleChanged();
	void textChanged();

protected:
	QString mTitle;
	QString mText;

	QTextBlockFormat mTextBlockFormat;
	QTextCharFormat mTextCharFormat;
	QTextBlockFormat mTitleBlockFormat;
	QTextCharFormat mTitleCharFormat;
};

} /* namespace qtouch */

#endif /* DOCUMENT_HPP_ */
