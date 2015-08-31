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

public:
	TrainingWidget(QQuickItem* parent = 0);
	virtual ~TrainingWidget();

public slots:
	void reset();

signals:
	void escape();
	void escapeKeyChanged();

protected:
	virtual void keyPressEvent(QKeyEvent*) override;
	virtual QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* updatePaintNodeData) override;

private:
	void resetCursor();
	void updateCursorRectangle(const QTextCursor& cursor);

	Qt::Key mEscKey = Qt::Key_Escape;
	std::unique_ptr<QTextCursor> mCursor;

	QTextCharFormat mCorrectTextCharFormat;
	QTextCharFormat mFailureTextCharFormat;
};

} /* namespace qtouch */

#endif /* TRAININGWIDGET_HPP_ */
