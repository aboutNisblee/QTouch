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
 * \file stats.hpp
 *
 * \date 24.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef STATS_HPP_
#define STATS_HPP_

#include <memory>
#include <QDateTime>
#include <QString>
#include <QUuid>

namespace qtouch
{

class Stats
{
public:
	explicit Stats(const QUuid& lessonId, const QString& profileName,
	               const QDateTime& start = QDateTime::currentDateTime()) :
		mLessonId(lessonId), mProfileName(profileName), mStart(start), mCharCount(0), mErrorCount(0) {}
	virtual ~Stats() {}

	const QUuid& getLessonId() const { return mLessonId; }
	const QString& getProfileName() const { return mProfileName; }

	inline const QDateTime& getStart() const {	return mStart; }
	void setStart(const QDateTime& start) { mStart = start; }

	const QDateTime& getEnd() const { return mEnd; }
	void setEnd(const QDateTime& end) { mEnd = end; }

	quint32 getCharCount() const { return mCharCount; }
	void setCharCount(quint32 charCount) { mCharCount = charCount; }

	quint32 getErrorCount() const { return mErrorCount;	}
	void setErrorCount(quint32 errorCount) { mErrorCount = errorCount; }

private:
	QUuid mLessonId;
	QString mProfileName;

	QDateTime mStart;
	QDateTime mEnd;
	quint32 mCharCount;
	quint32 mErrorCount;
};

} /* namespace qtouch */

#endif /* STATS_HPP_ */
