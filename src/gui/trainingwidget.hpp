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

public:
	TrainingWidget(QQuickItem* parent = 0);
	virtual ~TrainingWidget();

	inline qreal getProgress() const { return mProgress; }

public slots:
	void reset();

signals:
	void escape();
	void escapeKeyChanged();
	void progressChanged();

protected:
	virtual void keyPressEvent(QKeyEvent*) override;
	virtual QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;

private:
	void resetCursor();
	void updateCursorRectangle(const QTextCursor& cursor);
	void updateProgress(qreal percent);

	Qt::Key mEscKey = Qt::Key_Escape;
	std::unique_ptr<QTextCursor> mCursor;

	QTextCharFormat mCorrectTextCharFormat;
	QTextCharFormat mFailureTextCharFormat;

	qreal mProgress = 0;
};

} /* namespace qtouch */

#endif /* TRAININGWIDGET_HPP_ */
