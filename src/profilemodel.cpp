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
 * \file profilemodel.cpp
 *
 * \date 23.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "profilemodel.hpp"

#include <algorithm>

#include <QQmlEngine>
#include <QDebug>

#include "datamodel.hpp"

namespace qtouch
{

ProfileModel::ProfileModel(QObject* parent):
	QAbstractListModel(parent), mDm(nullptr), mSelected(-1)
{
}

ProfileModel::ProfileModel(DataModel* model, QObject* parent):
	QAbstractListModel(parent), mDm(model), mSelected(-1)
{
}

ProfileModel::~ProfileModel()
{
}

int ProfileModel::rowCount(const QModelIndex& /*parent*/) const
{
	return mDm->getProfileCount();
}

QVariant ProfileModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

	if (!mDm->isValidProfileIndex(index.row()))
		return QVariant();

	switch (role)
	{
	// TODO: Handle default roles
	case NameRole:
		return mDm->getProfile(index.row()).getName();
		break;
	case SkillRole:
		return static_cast<qtouch::QmlProfile::SkillLevel>(mDm->getProfile(index.row()).getSkillLevel());
		break;
	default:
		return QVariant();
		break;
	}
}

/* TODO: Extract to base class! */
QVariantMap ProfileModel::get(int i)
{
	QVariantMap result;
	QHash<int, QByteArray> names = roleNames();

	QHashIterator<int, QByteArray> it(names);
	while (it.hasNext())
	{
		it.next();
		QModelIndex idx = index(i, 0);
		QVariant data = idx.data(it.key());

		result[it.value()] = data;
	}
	return result;
}

void ProfileModel::selectProfile(int index)
{
	if (!mDm->isValidProfileIndex(index))
	{
		qWarning() << this << "Invalid index:" << index;
		mSelected = -1;
	}
	else /* Do net check for index changes. Simply update and fire! */
	{
		mSelected = index;
		emit profileIndexChanged();
		emit profileChanged();
	}
}

QmlProfile* ProfileModel::getProfile() const
{
	QmlProfile* p = new QmlProfile(mDm->getProfile(mSelected, true));
	QQmlEngine::setObjectOwnership(p, QQmlEngine::JavaScriptOwnership);
	return p;
}

QHash<int, QByteArray> ProfileModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[NameRole] = "pName";
	roles[SkillRole] = "pSkill";
	return roles;
}

bool ProfileModel::addProfile(const QString& name, QmlProfile::SkillLevel skill)
{
	bool result = false;
	beginInsertRows(QModelIndex(), mDm->getProfileCount(), mDm->getProfileCount());
	if (mDm->insertProfile(Profile(name, static_cast<Profile::SkillLevel>(skill))))
		result = true;
	endInsertRows();
	return result;
}

bool ProfileModel::addProfile(QmlProfile* profile)
{
	bool result = false;
	beginInsertRows(QModelIndex(), mDm->getProfileCount(), mDm->getProfileCount());
	if (mDm->insertProfile(*static_cast<Profile*>(profile)))
		result = true;
	endInsertRows();
	return result;
}

} /* namespace qtouch */
