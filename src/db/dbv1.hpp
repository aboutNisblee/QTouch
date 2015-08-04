/**
 * \file dbv1.hpp
 *
 * \date 31.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef DBV1_HPP_
#define DBV1_HPP_

#include <memory>
#include "dbinterface.hpp"

class QSqlDatabase;

namespace qtouch
{

class DbV1: public DbInterface
{
public:
	const int VERSION = 1;

	static std::unique_ptr<DbV1> create();
	virtual ~DbV1();

	/* Connection handling */
	void open(QString const& path);
	void close();
	inline bool isOpen() { return (db) ? db->isOpen() : false; }

	/* Schema */
	void createSchema();
	void dropSchema();

	/* MetaTable */
	void setMeta(const QString& key, const QVariant& value);
	QVariant getMeta(const QString& key);

	/* TRANSACTION */
	void begin_transaction();
	void end_transaction();
	void rollback();

	/* INSERT */
	void insert(const Profile& profile);
	void insert(const Stats& stats);
	void insert(const Course& course);
	void insert(const Lesson& lesson);
	int insert(const QUuid& courseId, const QUuid& lessonId, int parentId = 0);

	/* UPDATE */
	void update(const Profile& profile);
	void update(const Stats& stats);
	void update(const Course& course);
	void update(const Lesson& lesson);

	/* SELECT */
	QSqlQuery selectProfiles();
	QSqlQuery selectStats(const QString& profileName);
	QSqlQuery selectCourses(Db::CourseType type);
	QSqlQuery selectCourse(const QUuid& courseId);
	QSqlQuery selectLesson(const QUuid& lessonId);
	QSqlQuery selectLessonList(const QUuid& courseId);
	QSqlQuery selectDanglingLesson();

	/* DELETE */
	void deleteProfile(const QString& profileName);
	void deleteStats(const QString& profileName);
	void deleteCourse(const QUuid& courseId);
	void deleteLesson(const QUuid& lessonId);
	void deleteLessonList(const QUuid& courseId);

private:
	DbV1() {}
	Q_DISABLE_COPY(DbV1)

	inline void checkOpen() { if (!isOpen()) throw DbException("Database not open"); }

	std::unique_ptr<QSqlDatabase> db;
};

} /* namespace qtouch */

#endif /* DBV1_HPP_ */
