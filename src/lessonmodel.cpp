#include "lessonmodel.hpp"

LessonModel::LessonModel(QObject* parent):
    QAbstractListModel(parent)
{
    for (int i = 0; i < 100; ++i)
    {
        mLessons.append(QString("Test lesson %1").arg(i));
    }
}

LessonModel::~LessonModel()
{

}

int LessonModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return mLessons.size();
}

QVariant LessonModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() < 0 || index.row() >= mLessons.size())
        return QVariant();

    if (role == Qt::DisplayRole)
        return mLessons.at(index.row());
    else
        return QVariant();
}
