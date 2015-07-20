/**
 * \file course.hpp
 *
 * \date 02.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef COURSE_HPP_
#define COURSE_HPP_

#include <memory>
#include <vector>

#include <QString>
#include <QUuid>
#include <QDataStream>
#include <QByteArray>
#include <QCryptographicHash>

#include "utils/utils.hpp"

namespace qtouch
{

class Course;

class CourseLessonBase
{
public:
	virtual ~CourseLessonBase() {}

	virtual const QUuid& getId() const { return mId; }
	// TODO: Remove this setter! Try to set PK in constructor to make a manipulation impossible!
	virtual bool setId(const QUuid& id);

	virtual const QString& getTitle() const { return mTitle; }
	virtual void setTitle(const QString& title) { mTitle = title; }

	virtual bool isBuiltin() const { return mBuiltin; }
	virtual void setBuiltin(bool builtin) { mBuiltin = builtin; }

	virtual QDataStream& serialize(QDataStream& out) const = 0;
	QByteArray hash() const;

	inline bool operator==(const CourseLessonBase& rhs) const { return hash() == rhs.hash(); }
	inline bool operator!=(const CourseLessonBase& rhs) const { return hash() != rhs.hash(); }

protected:
	CourseLessonBase() : mBuiltin(false) {}

	QUuid mId;
	QString mTitle;
	bool mBuiltin;
};

class Lesson: public CourseLessonBase
{
	friend class Course;
public:
	virtual ~Lesson() {}

	inline const QString& getNewChars() const { return mNewChars; }
	inline void setNewChars(const QString& newChars) { mNewChars = newChars; }

	inline const QString& getText() const { return mText; }
	inline void setText(const QString& text) { mText = text; }

	std::shared_ptr<Course> getCourse() const;

	virtual QDataStream& serialize(QDataStream& out) const;

private:
	/* Store the back pointer to the owning object. */
	inline void setCourse(std::shared_ptr<Course>& parent) { mCourse = parent; }

	std::weak_ptr<Course> mCourse;

	QString mNewChars;
	QString mText;
};

/**
 * Class that is able to manage a Course and its Lessons.
 */
class Course: public std::enable_shared_from_this<Course>, public CourseLessonBase
{
public:
	typedef std::vector<std::shared_ptr<const Lesson>>::size_type size_type;
	typedef std::vector<std::shared_ptr<const Lesson>>::const_iterator const_iterator;

	static std::shared_ptr<Course> create();
	static std::shared_ptr<Course> clone(const Course& org);

	virtual ~Course() {}

	inline const QString& getDescription() const { return mDescription; }
	inline void setDescription(const QString& description) { mDescription = description; }

	void push_back(const Lesson& lesson);

	template<typename Iterator>
	const_iterator insert(const_iterator position, Iterator first, Iterator last);

	inline int size() const { return mLessons.size(); }
	inline void clear() { mLessons.clear(); }

	inline std::shared_ptr<const Lesson> at(size_type i) const throw(std::out_of_range) { return mLessons.at(i); }
	inline std::shared_ptr<const Lesson> operator[](size_type i) const throw(std::out_of_range) { return mLessons.at(i); }

	bool contains(const QUuid& id) const;
	std::shared_ptr<const Lesson> get(const QUuid& lessonId) const;

	inline const_iterator begin() const {return mLessons.begin(); }
	inline const_iterator end() const { return mLessons.end(); }

	virtual QDataStream& serialize(QDataStream& out) const;

private:
	Course() {}
	Course(const Course& org);

	QString mDescription;
	std::vector<std::shared_ptr<const Lesson>> mLessons;
};

/**
 * Serialize all members.
 * @param out An output stream.
 * @param object A Course or Lesson.
 * @return The output stream.
 */
template<class T>
inline QDataStream& operator<<(QDataStream& out, const T& object)
{
	return object.serialize(out);
}

/**
 * Calculate the MD5 hash over a given range of Courses or Lessons.
 * @param first Iterator to the first Course.
 * @param last Iterator behind the last Course.
 * @return The MD5 hash.
 */
template<typename Iterator>
QByteArray hash(Iterator first, Iterator last)
{
	QByteArray buffer;
	QDataStream stream(&buffer, QIODevice::WriteOnly);
	while (first != last)
	{
		stream << value(*first);
		++first;
	}
	return QCryptographicHash::hash(buffer, QCryptographicHash::Md5);
}

template<typename Iterator>
Course::const_iterator Course::insert(const_iterator position, Iterator first, Iterator last)
{
	ptrdiff_t offset = position - begin();
	for (; first != last; ++first)
	{
		/* Make a "deep" copy of the given lesson
		 * to not alter the parent ptr of the passed one.
		 * Its not really deep because its members are implicitly shared. */
		auto l = std::make_shared<Lesson>(**first);
		auto thiz = shared_from_this();
		l->setCourse(thiz);

		position = mLessons.insert(position, l);
		++position;
	}
	return begin() + offset;
}

struct CourseListAscTitle
{
	bool operator()(const std::shared_ptr<const Course>& lhs, const std::shared_ptr<const Course>& rhs)
	{
		return (lhs->getTitle() < rhs->getTitle());
	}
};

} /* namespace qtouch */


#endif /* COURSE_HPP_ */
