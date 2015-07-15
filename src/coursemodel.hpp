/**
 * \file coursemodel.hpp
 *
 * \date 12.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef COURSEMODEL_HPP_
#define COURSEMODEL_HPP_

#include <QAbstractListModel>

#include "datamodel.hpp"

namespace qtouch
{

class LessonModel: public QAbstractListModel
{
	Q_OBJECT

	Q_PROPERTY(int selectedLessonIndex READ getSelectedLessonIndex WRITE selectLesson NOTIFY selectedLessonIndexChanged)

public:
	enum LessonModelRoles
	{
		UuidRole = Qt::UserRole + 1,
		TitleRole,
		NewCharsRole,
		BuiltinRole,
		TextRole
	};

	explicit LessonModel(QObject* parent = 0);
	explicit LessonModel(DataModelPtr model, QObject* parent = 0);
	virtual ~LessonModel();

	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant data(const QModelIndex& index, int role) const;

	Q_INVOKABLE QVariantMap get(int index);

	void selectLesson(int index);
	int getSelectedLessonIndex() const;

	void setCourse(const QUuid& courseId);

signals:
	void selectedLessonIndexChanged();

protected:
	QHash<int, QByteArray> roleNames() const;

private:
	DataModelPtr mDm;

	QUuid mCourseId;
	int mSelected;
};

class CourseModel: public QAbstractListModel
{
	Q_OBJECT

	Q_PROPERTY(int selectedCourseIndex READ getSelectedCourseIndex WRITE selectCourse NOTIFY selectedCourseIndexChanged)
	Q_PROPERTY(LessonModel* selectedLessonModel READ getSelectedLessonModel NOTIFY selectedLessonModelChanged)

public:
	enum CourseModelRoles
	{
		UuidRole = Qt::UserRole + 1,
		TitleRole,
		DescriptionRole,
		BuiltinRole,
	};

	explicit CourseModel(DataModelPtr model, QObject* parent = 0);
	virtual ~CourseModel();

	// TODO: Add flags to control enabled items! E.g. different language, never used, ...

	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant data(const QModelIndex& index, int role) const;

	Q_INVOKABLE QVariantMap get(int index);

	/* TODO: Is this really smart?
	 * This could be completely moved to the view when
	 * the selection feature is removed and clients passes the index
	 * of the course they need a LessonModel for.
	 */
	void selectCourse(int index);
	int getSelectedCourseIndex() const;
	LessonModel* getSelectedLessonModel() const;

	//	Q_INVOKABLE LessonModel* getLessonModel(int index) const;

signals:
	void selectedCourseIndexChanged();
	void selectedLessonModelChanged();

protected:
	QHash<int, QByteArray> roleNames() const;

private:
	DataModelPtr mDm;

	// Lifetime managed by Qt
	LessonModel* mLessonModel;
	int mSelected;
};

} /* namespace qtouch */

#endif /* COURSEMODEL_HPP_ */
