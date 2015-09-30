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

int ProfileModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
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
		return mDm->getProfileName(index.row());
		break;
	case SkillRole:
		return mDm->getProfileSkill(index.row());
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
		//	else if(mSelected != index)
	{
		mSelected = index;
		emit selectedProfileIndexChanged();

		emit selectedProfileNameChanged();
		emit selectedProfileSkillLevelChanged();
	}
}

QHash<int, QByteArray> ProfileModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[NameRole] = "name";
	roles[SkillRole] = "skill";
	return roles;
}

} /* namespace qtouch */
