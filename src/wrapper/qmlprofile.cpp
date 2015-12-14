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
 * \file qmlprofile.cpp
 *
 * \date 03.10.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "qmlprofile.hpp"

#include <QQmlEngine>

namespace qtouch
{

QmlStats::QmlStats(QObject* parent) :
	QObject(parent), Stats(QUuid(), QUuid(), "")
{
}

QmlStats::QmlStats(const Stats& rhs) :
	Stats(rhs)
{
}

Stats QmlStats::unwrap()
{
	Stats lhs(mCourseId, mLessonId, mProfileName, mStart);
	lhs.setTime(mTime);
	lhs.setCharCount(mCharCount);
	lhs.setErrorCount(mErrorCount);
	return lhs;
}

void QmlStats::setCourseId(const QUuid& courseId)
{
	if (courseId == mCourseId)
		return;

	mCourseId = courseId;
	emit courseIdChanged();
}

void QmlStats::setLessonId(const QUuid& lessonId)
{
	if (lessonId == mLessonId)
		return;

	mLessonId = lessonId;
	emit lessonIdChanged();
}

void QmlStats::setProfileName(const QString& profileName)
{
	if (profileName == mProfileName)
		return;

	mProfileName = profileName;
	emit profileNameChanged();
}

void QmlStats::setStart(const QDateTime& start)
{
	if (start == mStart)
		return;

	mStart = start;
	emit startChanged();
}

void QmlStats::setTime(quint32 time)
{
	if (time == mTime)
		return;

	mTime = time;
	emit timeChanged();
}

void QmlStats::setCharCount(quint32 charCount)
{
	if (charCount == mCharCount)
		return;

	mCharCount = charCount;
	emit charCountChanged();
}

void QmlStats::setErrorCount(quint32 errorCount)
{
	if (errorCount == mErrorCount)
		return;

	mErrorCount = errorCount;
	emit errorCountChanged();
}

QmlProfile::QmlProfile(QObject* parent) :
	QObject(parent), Profile(QString())
{
}

QmlProfile::QmlProfile(const Profile& rhs, QObject* parent) :
	QObject(parent), Profile(rhs)
{
	emit nameChanged();
	emit skillLevelChanged();
}

void QmlProfile::setName(const QString& name)
{
	if (name == mName)
		return;

	mName = name;
	emit nameChanged();
}

void QmlProfile::setSkill(QmlSkillLevel::SkillLevel skill)
{
	if (static_cast<Profile::SkillLevel>(skill) == mSkillLevel)
		return;

	mSkillLevel = static_cast<Profile::SkillLevel>(skill);
	emit skillLevelChanged();
}

void QmlProfile::pushStats(QmlStats* stats)
{
	mStats.push_back(stats->unwrap());
	emit statsChanged();
}

void QmlProfile::appendStats(QQmlListProperty<QmlStats>* list, QmlStats* value)
{
	QmlProfile* profile = qobject_cast<QmlProfile*>(list->object);
	if (profile)
	{
		profile->mStats.push_back(value->unwrap());
		emit profile->statsChanged();
	}
}

int QmlProfile::countStats(QQmlListProperty<QmlStats>* list)
{
	QmlProfile* profile = qobject_cast<QmlProfile*>(list->object);
	if (profile)
		return static_cast<int>(profile->mStats.size());
	else
		return 0;
}

QmlStats* QmlProfile::atStats(QQmlListProperty<QmlStats>* list, int index)
{
	QmlProfile* profile = qobject_cast<QmlProfile*>(list->object);
	if (profile && index >= 0 && index < static_cast<int>(profile->mStats.size()))
	{
		QmlStats* s = new QmlStats(profile->mStats.at(index));
		QQmlEngine::setObjectOwnership(s, QQmlEngine::JavaScriptOwnership);
		return s;
	}
	else
		return nullptr;
}

void QmlProfile::clearStats(QQmlListProperty<QmlStats>* /*list*/)
{
	// TODO: IMPLEMENT ME!!
	//	QmlProfile* profile = qobject_cast<QmlProfile*>(list->object);
}

} /* namespace qtouch */
