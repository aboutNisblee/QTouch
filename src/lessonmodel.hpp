#ifndef LESSONMODEL_HPP
#define LESSONMODEL_HPP

#include <QAbstractListModel>

namespace qtouch
{

class LessonModel: public QAbstractListModel
{
Q_OBJECT

public:
	LessonModel(QObject* parent = 0);
	virtual ~LessonModel();

	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant data(const QModelIndex& index, int role) const;

private:
	QStringList mLessons;
};

} /* namespace qtouch */

#endif // LESSONMODEL_HPP
