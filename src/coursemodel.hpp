/**
 * \file coursemodel.hpp
 *
 * \date 12.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef COURSEMODEL_HPP_
#define COURSEMODEL_HPP_

#include <QAbstractListModel>
#include <QUuid>

namespace qtouch
{

class DataModel;
class CourseModel;

class LessonModel: public QAbstractListModel
{
	Q_OBJECT

	Q_PROPERTY(int selectedLessonIndex READ getSelectedLessonIndex NOTIFY selectedLessonIndexChanged)

	Q_PROPERTY(QUuid selectedLessonId READ getSelectedLessonId NOTIFY selectedLessonIdChanged)
	Q_PROPERTY(QString selectedLessonTitle READ getSelectedLessonTitle NOTIFY selectedLessonTitleChanged)
	Q_PROPERTY(QString selectedLessonNewChars READ getSelectedLessonNewChars NOTIFY selectedLessonNewCharsChanged)
	Q_PROPERTY(bool isSelectedLessonBuiltin READ isSelectedLessonBuiltin NOTIFY selectedLessonBuiltinChanged)
	Q_PROPERTY(QString selectedLessonText READ getSelectedLessonText NOTIFY selectedLessonTextChanged)

public:
	/** LessonModelRoles */
	enum LessonModelRoles
	{
		UuidRole = Qt::UserRole + 1,//!< UuidRole lId
		TitleRole,                  //!< TitleRole lTitle
		NewCharsRole,               //!< NewCharsRole lNewChars
		BuiltinRole,                //!< BuiltinRole lBuiltin
		TextRole                    //!< TextRole lText
	};

	explicit LessonModel(QObject* parent = 0);
	explicit LessonModel(DataModel* model, QObject* parent = 0);
	virtual ~LessonModel();

	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant data(const QModelIndex& index, int role) const;

	Q_INVOKABLE QVariantMap get(int index);

	Q_INVOKABLE void selectLesson(int index);

	inline int getSelectedLessonIndex() const { return mSelected; }
	inline QUuid getSelectedLessonId() const { return index(mSelected).data(UuidRole).toUuid(); }
	inline QString getSelectedLessonTitle() const { return index(mSelected).data(TitleRole).toString(); }
	inline QString getSelectedLessonNewChars() const { return index(mSelected).data(NewCharsRole).toString(); }
	inline bool isSelectedLessonBuiltin() const { return index(mSelected).data(BuiltinRole).toBool(); }
	inline QString getSelectedLessonText() const { return index(mSelected).data(TextRole).toString(); }

signals:
	void selectedLessonIndexChanged();

	void selectedLessonIdChanged();
	void selectedLessonTitleChanged();
	void selectedLessonNewCharsChanged();
	void selectedLessonBuiltinChanged();
	void selectedLessonTextChanged();

protected:
	QHash<int, QByteArray> roleNames() const;

private:
	friend class CourseModel;
	inline void setCourse(const QUuid& courseId) { mCourseId = courseId; }

	DataModel* mDm;

	QUuid mCourseId;
	int mSelected;
};

class CourseModel: public QAbstractListModel
{
	Q_OBJECT

	Q_PROPERTY(int selectedCourseIndex READ getSelectedCourseIndex NOTIFY selectedCourseIndexChanged)

	/**
	 * The LessonModel for the lessons of the currently selected course.
	 * @note Since the pointer never changes its value you cannot rebind this property to another one
	 * and catch the onChanged signal. Use property aliasing instead of rebinding.
	 * Binding to this property directly works, because of the explicit emission of selectedLessonModelChanged.
	 */
	Q_PROPERTY(LessonModel* selectedLessonModel READ getSelectedLessonModel NOTIFY selectedLessonModelChanged)

	Q_PROPERTY(QUuid selectedCourseId READ getSelectedCourseId NOTIFY selectedCourseIdChanged)
	Q_PROPERTY(QString selectedCourseTitle READ getSelectedCourseTitle NOTIFY selectedCourseTitleChanged)
	Q_PROPERTY(QString selectedCourseDescription READ getSelectedCourseDescription NOTIFY selectedCourseDescriptionChanged)
	Q_PROPERTY(bool isSelectedCourseBuiltin READ isSelectedCourseBuiltin NOTIFY selectedCourseBuiltinChanged)

public:
	/** CourseModelRoles */
	enum CourseModelRoles
	{
		UuidRole = Qt::UserRole + 1,//!< UuidRole cId
		TitleRole,                  //!< TitleRole cTitle
		DescriptionRole,            //!< DescriptionRole cDescription
		BuiltinRole,                //!< BuiltinRole cBuiltin
	};

	explicit CourseModel(QObject* parent = 0);
	explicit CourseModel(DataModel* model, QObject* parent = 0);
	virtual ~CourseModel();

	// TODO: Add flags to control enabled items! E.g. different language, never used, ...

	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant data(const QModelIndex& index, int role) const;

	Q_INVOKABLE QVariantMap get(int index);

	Q_INVOKABLE void selectCourse(int index);

	inline int getSelectedCourseIndex() const { return mSelected; }
	inline LessonModel* getSelectedLessonModel() const { return mLessonModel; }

	inline QUuid getSelectedCourseId() const { return index(mSelected).data(UuidRole).toUuid(); }
	inline QString getSelectedCourseTitle() const { return index(mSelected).data(TitleRole).toString(); }
	inline QString getSelectedCourseDescription() const { return index(mSelected).data(DescriptionRole).toString(); }
	inline bool isSelectedCourseBuiltin() const { return index(mSelected).data(BuiltinRole).toBool(); }

	//	Q_INVOKABLE LessonModel* getLessonModel(int index) const;

signals:
	void selectedCourseIndexChanged();
	void selectedLessonModelChanged();

	void selectedCourseIdChanged();
	void selectedCourseTitleChanged();
	void selectedCourseDescriptionChanged();
	void selectedCourseBuiltinChanged();

protected:
	QHash<int, QByteArray> roleNames() const;

private:
	DataModel* mDm;

	// Lifetime managed by Qt
	LessonModel* mLessonModel;
	int mSelected;
};

} /* namespace qtouch */

#endif /* COURSEMODEL_HPP_ */
