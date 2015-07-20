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
