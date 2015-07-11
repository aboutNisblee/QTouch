/**
 * \file db.hpp
 *
 * \date 06.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef DB_HPP
#define DB_HPP

#include <QSharedPointer>
#include <QString>

#include "utils/profile.hpp"
#include "utils/course.hpp"
#include "utils/exceptions.hpp"

class QSqlDatabase;

namespace qtouch
{

/**
 * Database management.
 * @note We forego to implement singleton crap. Simply instantiate only once.
 * A class is only used for easier connection lifetime management.
 */
class Db
{
public:
	/*
	 * XXX: Use QStandardPaths::DataLocation when < 5.4
	 * else QStandardPaths::AppDataLocation
	 */

	enum COURSE_SELECTOR
	{
		ALL, BUILTIN, CUSTOM
	};

	/**
	 * Constructor.
	 * A call to open is optional. The connection is opened when needed,
	 * using the given path.
	 * @param path Path to the database file.
	 */
	explicit Db(QString const& path);
	/**
	 * Destructor.
	 * Closes the connection to the database and free the resources.
	 */
	virtual ~Db();

	/**
	 * Returns the configured user version.
	 * @return The user version of the Db, 0 if Db just created or -1 on error.
	 */
	int getUserVersion() throw (DatabaseException);

	/**
	 * Explicitly open a Db file. If path is empty the path passed to constructor is used.
	 * @param path Path to the database file.
	 * @param force_creation If true, the old database is dropped.
	 * @return true on success.
	 */
	void open(QString const& path = QString(), bool force_creation = false) throw (DatabaseException);

	/**
	 * Explicitly close the current database.
	 * A call to this function is normally not needed. It is called from destructor.
	 */
	void close();

	/**
	 * Insert a profile.
	 * @param profile
	 * @return true on success.
	 */
	void insert(Profile const& profile) throw (DatabaseException);
	/**
	 * Insert a course.
	 * @param course
	 * @return true on success.
	 */
	void insert(const CoursePtr& course) throw (DatabaseException);

	void insert(const CourseList& courses) throw (DatabaseException);

	CourseList courses(COURSE_SELECTOR selector) throw (DatabaseException);
	LessonList lessons(const CoursePtr& course) throw (DatabaseException);

private:
	QSharedPointer<QSqlDatabase> db;
	QString mPath;

	Q_DISABLE_COPY(Db)
};

} /* namespace qtouch */

#endif // DB_HPP
