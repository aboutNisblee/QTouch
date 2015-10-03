/* Copyright (C) 2015  Moritz Nisblé <moritz.nisble@gmx.de>
 *
 * This file is part of QTouch.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/**
 * \file profile.hpp
 *
 * \date 02.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef PROFILE_HPP_
#define PROFILE_HPP_

#include <memory>
#include <vector>
#include <QDateTime>
#include <QString>
#include <QUuid>

namespace qtouch
{

class Stats
{
public:
	Stats(const QUuid& courseId, const QUuid& lessonId, const QString& profileName,
	      const QDateTime& start = QDateTime::currentDateTime()) :
		mCourseId(courseId), mLessonId(lessonId), mProfileName(profileName), mStart(start), mCharCount(0), mErrorCount(0) {}
	virtual ~Stats() {}

	inline const QUuid& getCourseId() const { return mCourseId; }
	inline const QUuid& getLessonId() const { return mLessonId; }
	inline const QString& getProfileName() const { return mProfileName; }
	inline const QDateTime& getStart() const {	return mStart; }

	inline const QDateTime& getEnd() const { return mEnd; }
	virtual void setEnd(const QDateTime& end) { mEnd = end; }

	inline quint32 getCharCount() const { return mCharCount; }
	virtual void setCharCount(quint32 charCount) { mCharCount = charCount; }

	inline quint32 getErrorCount() const { return mErrorCount;	}
	virtual void setErrorCount(quint32 errorCount) { mErrorCount = errorCount; }

protected:
	QUuid mCourseId;
	QUuid mLessonId;
	QString mProfileName;
	QDateTime mStart;

	QDateTime mEnd;
	quint32 mCharCount;
	quint32 mErrorCount;
};

class Profile
{
public:
	typedef std::vector<Stats>::value_type value_type;
	typedef std::vector<Stats>::size_type size_type;
	typedef std::vector<Stats>::iterator iterator;
	typedef std::vector<Stats>::const_iterator const_iterator;

	/** SkillLevel */
	enum SkillLevel
	{
		Beginner,//!< Beginner
		Advanced //!< Advanced
	};

	explicit Profile(const QString& name, SkillLevel skill = Beginner) : mName(name), mSkillLevel(skill) {}
	virtual ~Profile() {}

	inline const QString& getName() const { return mName; }

	inline SkillLevel getSkillLevel() const { return mSkillLevel; }
	inline void setSkillLevel(SkillLevel skillLevel) { mSkillLevel = skillLevel; }

	inline void push_back(const Stats& stats) { mStats.push_back(stats); }
	inline void clear() { mStats.clear(); }
	inline void replace(std::vector<Stats> stats) { mStats.swap(stats); }
	inline size_type size() const { return mStats.size(); }

	template<typename InputIt>
	iterator insert(const_iterator pos, InputIt first, InputIt last) { return mStats.insert(pos, first, last); }
	inline iterator insert(const_iterator position, const value_type& val) { return mStats.insert(position, val); }
	inline iterator insert(const_iterator position, value_type&& val) { return mStats.insert(position, val); }

	inline const_iterator begin() const { return mStats.begin(); }
	inline const_iterator end() const { return mStats.end(); }
	inline iterator begin() { return mStats.begin(); }
	inline iterator end() { return mStats.end(); }

protected:
	QString mName;
	SkillLevel mSkillLevel;

	std::vector<Stats> mStats;
};

} /* namespace qtouch */

#endif /* PROFILE_HPP_ */
