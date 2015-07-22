/**
 * \file db.hpp
 *
 * \date 06.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef DB_HPP
#define DB_HPP

#include <QString>
#include <QSqlError>

#include "entities/profile.hpp"
#include "entities/stats.hpp"
#include "entities/course.hpp"
#include "utils/exceptions.hpp"

//class QSqlDatabase;
#include <QtSql/QSqlDatabase>

namespace qtouch
{

class DbException: public Exception
{
public:
	explicit DbException(const QString& msg): Exception(msg) {}
	DbException(const QString& msg, const QSqlError& sqlError): Exception(msg), mSqlError(sqlError) {}
	virtual ~DbException() throw () {}

	virtual void raise() const { throw* this; }
	virtual DbException* clone() const { return new DbException(*this); }

	virtual QString message() const throw()
	{
		QString msg = mMsg;
		if (!mSqlError.databaseText().isEmpty())
		{
			msg.append("\n\tDatabase message: " % mSqlError.databaseText());
			if (!mSqlError.nativeErrorCode().isEmpty())
				msg.append(" (" % mSqlError.nativeErrorCode() % ")");
		}
		if (!mSqlError.driverText().isEmpty())
			msg.append("\n\tDriver message: " % mSqlError.driverText());
		return msg;
	}

	virtual QString databaseMessage() const throw() { return mSqlError.databaseText(); }
	virtual QString driverMessage() const throw() { return mSqlError.driverText(); }

	virtual const QSqlError& sqlError() const throw() { return mSqlError; }
	virtual int sqlErrorCode() const throw() { return mSqlError.nativeErrorCode().toInt(); }

protected:
	QSqlError mSqlError;
};

/**
 * Database management.
 * @note We forego to implement singleton crap. Simply instantiate only once.
 * A class is only used for easier connection lifetime management.
 */
class Db
{
public:
	// TODO: Move me to entities when needed anywhere else.
	enum CourseType
	{
		All, BuiltIn, Custom
	};

	explicit Db(QString const& path);
	virtual ~Db();

	void open(QString const& path = QString(), bool force_creation = false) throw (DbException);
	void close();

	int getUserVersion() throw (DbException);

	void setCourseHash(const QByteArray& hash) throw (DbException);
	QByteArray getCourseHash() throw (DbException);

	/* SELECT */

	std::vector<std::shared_ptr<const Profile>> profiles() throw (DbException);

	std::vector<std::shared_ptr<const Stats>> stats(const QString& profileName) throw (DbException);
	void replaceStats(Profile& profile) throw (DbException);

	std::unique_ptr<Lesson> lesson(const QUuid& lessonId) throw (DbException);
	std::vector<std::shared_ptr<Lesson>> lessons(const QUuid& courseId) throw (DbException);

	std::shared_ptr<Course> course(const QUuid& courseId) throw (DbException);
	std::vector<std::shared_ptr<Course>> courses(CourseType type = All) throw (DbException);
	void replaceLessons(Course& course) throw (DbException);


	/* INSERT */

	void insert(const Profile& profile) throw (DbException);
	void insert(const Course& course) throw (DbException);
	void insert(const Stats& stats) throw (DbException);

	/* TODO: Its a pity! To not force the client to use a specific container,
	 * we should use templates. But a template must be implemented in a file that
	 * is included by the client while it must access the functions that should
	 * be hidden at the same time.
	 * TODO: Simply add a little interface for the templates to access the hidden functions.
	 */
//	template<typename Iter>
//	void insert(Iter first, Iter last) throw (DbException);

	void insert(const std::vector<std::shared_ptr<const Course>>& courses) throw (DbException);

	/* DELETE */

	void deleteCourse(const QUuid& courseId) throw (DbException);
	void deleteLessonList(const QUuid& courseId) throw (DbException);

	/* UPDATE */

	bool update(const Course& course) throw (DbException);

private:
	std::unique_ptr<QSqlDatabase> db;
	QString mPath;

	Q_DISABLE_COPY(Db)
};

} /* namespace qtouch */

#endif // DB_HPP
