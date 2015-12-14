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
 * \file qmlprofile.hpp
 *
 * \date 03.10.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef QMLPROFILE_HPP_
#define QMLPROFILE_HPP_

#include <QObject>
#include <QQmlListProperty>

#include "entities/profile.hpp"

namespace qtouch
{

class QmlStats: public QObject, public Stats
{
	Q_OBJECT

	Q_PROPERTY(QUuid course READ getCourseId WRITE setCourseId NOTIFY courseIdChanged)
	Q_PROPERTY(QUuid lesson READ getLessonId WRITE setLessonId NOTIFY lessonIdChanged)
	Q_PROPERTY(QString profile READ getProfileName WRITE setProfileName NOTIFY profileNameChanged)
	Q_PROPERTY(QDateTime start READ getStart WRITE setStart NOTIFY startChanged)
	Q_PROPERTY(quint32 time READ getTime WRITE setTime NOTIFY timeChanged)
	Q_PROPERTY(quint32 chars READ getCharCount WRITE setCharCount NOTIFY charCountChanged)
	Q_PROPERTY(quint32 errors READ getErrorCount WRITE setErrorCount NOTIFY errorCountChanged)

public:
	QmlStats(QObject* parent = nullptr);
	explicit QmlStats(const Stats& rhs);
	virtual ~QmlStats() {}

	Stats unwrap();

	void setCourseId(const QUuid& lessonId);
	void setLessonId(const QUuid& lessonId);
	void setProfileName(const QString& profileName);
	void setStart(const QDateTime& start);

	void setTime(quint32 time) Q_DECL_OVERRIDE;
	void setCharCount(quint32 charCount) Q_DECL_OVERRIDE;
	void setErrorCount(quint32 errorCount) Q_DECL_OVERRIDE;

signals:
	void courseIdChanged();
	void lessonIdChanged();
	void profileNameChanged();
	void startChanged();
	void timeChanged();
	void charCountChanged();
	void errorCountChanged();
};

class QmlSkillLevel: public QObject
{
	Q_GADGET
	Q_ENUMS(SkillLevel)
public:
	/* FIXME: This enum shadows Profile::SkillLevel!
	 * Find another solution. Profile should not be derived from QObject,
	 * but this is a requirement for enums to be accessible from QML.
	 * See: qthelp://org.qt-project.qtcore.542/qtcore/qobject.html#Q_ENUMS
	 * BUT: It isn't possible to refer to an enum from QML anyway.
	 * In QML enums are always of type int.
	 * I don't understand it anymore :)
	 * This class isn't registered and the values ase accessible from QML
	 * as e.g. Profile.Beginner! */
	enum SkillLevel
	{
		Beginner = Profile::Beginner, Advanced = Profile::Advanced
	};
};

class QmlProfile: public QObject, public Profile
{
	Q_OBJECT

	Q_ENUMS(qtouch::QmlSkillLevel::SkillLevel)
	Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(qtouch::QmlSkillLevel::SkillLevel skill READ getSkill WRITE setSkill NOTIFY skillLevelChanged)
	Q_PROPERTY(QQmlListProperty<qtouch::QmlStats> stats READ getStats NOTIFY statsChanged)

public:
	explicit QmlProfile(QObject* parent = nullptr);
	explicit QmlProfile(const Profile& rhs, QObject* parent = nullptr);
	virtual ~QmlProfile() {}

	void setName(const QString& name);
	void setSkill(QmlSkillLevel::SkillLevel skill);
	inline QmlSkillLevel::SkillLevel getSkill() const { return static_cast<QmlSkillLevel::SkillLevel>(mSkillLevel); }

	inline QQmlListProperty<QmlStats> getStats()
	{
		return QQmlListProperty<QmlStats>(this, 0, &QmlProfile::appendStats, &QmlProfile::countStats, &QmlProfile::atStats,
		                                  &QmlProfile::clearStats);
	}

	Q_INVOKABLE void pushStats(QmlStats* stats);

signals:
	void nameChanged();
	void skillLevelChanged();
	void statsChanged();

private:
	static void appendStats(QQmlListProperty<QmlStats>* list, QmlStats* value);
	static int countStats(QQmlListProperty<QmlStats>* list);
	static QmlStats* atStats(QQmlListProperty<QmlStats>* list, int index);
	static void clearStats(QQmlListProperty<QmlStats>* list);
};

} /* namespace qtouch */

#endif /* QMLPROFILE_HPP_ */
