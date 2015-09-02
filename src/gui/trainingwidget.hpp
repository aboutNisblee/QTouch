/**
 * \file trainingwidget.hpp
 *
 * \date 11.08.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef TRAININGWIDGET_HPP_
#define TRAININGWIDGET_HPP_

#include <QTextCursor>

#include "textpage.hpp"

namespace qtouch
{

class TrainingWidget: public TextPage
{
	Q_OBJECT

	Q_PROPERTY(Qt::Key escapeKey MEMBER mEscKey NOTIFY escapeKeyChanged)
	Q_PROPERTY(qreal progress READ getProgress NOTIFY progressChanged)
	Q_PROPERTY(QRectF cursorRectangle READ getCursorRectangle NOTIFY cursorRectangleChanged)

	/* The number of the line the cursor is currently in starting at 0. */
	Q_PROPERTY(int activeLineNumber READ getActiveLineNumber NOTIFY activeLineNumberChanged)
	/* Current position of the cursor in the currently active line starting at 0. */
	Q_PROPERTY(int cursorPosition READ getCursorPosition NOTIFY cursorPositionChanged)

public:
	TrainingWidget(QQuickItem* parent = 0);
	virtual ~TrainingWidget();

	inline qreal getProgress() const { return mProgress; }
	inline QRectF getCursorRectangle() const { return mCursorRectangle; }
	int getActiveLineNumber() const;
	int getCursorPosition() const;

public slots:
	void reset();

signals:
	void escape();
	void escapeKeyChanged();
	void progressChanged();
	void cursorRectangleChanged();
	void activeLineNumberChanged();
	void cursorPositionChanged();

protected:
	virtual void keyPressEvent(QKeyEvent*) override;

	virtual void onBeforeSynchronizing() override;
	virtual QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;

private:
	void resetCursor();
	void updateProgress(qreal percent);

	Qt::Key mEscKey = Qt::Key_Escape;
	std::unique_ptr<QTextCursor> mCursor;

	QTextCharFormat mCorrectTextCharFormat;
	QTextCharFormat mFailureTextCharFormat;

	QRectF mCursorRectangle;
	qreal mProgress = 0;
	bool mCursorDirty = false;
};

} /* namespace qtouch */

#endif /* TRAININGWIDGET_HPP_ */
