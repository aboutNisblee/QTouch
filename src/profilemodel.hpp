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
 * \file profilemodel.hpp
 *
 * \date 23.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef PROFILEMODEL_HPP_
#define PROFILEMODEL_HPP_

#include <QAbstractListModel>

namespace qtouch
{

class DataModel;

class ProfileModel: public QAbstractListModel
{
	Q_OBJECT

	Q_PROPERTY(int selectedProfileIndex READ getSelectedProfileIndex NOTIFY selectedProfileIndexChanged)

	Q_PROPERTY(QString selectedProfileName READ getSelectedProfileName NOTIFY selectedProfileNameChanged)
	Q_PROPERTY(QString selectedProfileSkillLevel READ getSelectedProfileSkillLevel NOTIFY selectedProfileSkillLevelChanged)

public:
	/** ProfileModelRoles */
	enum ProfileModelRoles
	{
		NameRole = Qt::UserRole + 1,//!< NameRole pName
		SkillRole                  	//!< SkillRole pSkill
	};

	explicit ProfileModel(QObject* parent = 0);
	explicit ProfileModel(DataModel* model, QObject* parent = 0);
	virtual ~ProfileModel();

	int rowCount(const QModelIndex& parent = QModelIndex()) const;
	QVariant data(const QModelIndex& index, int role) const;

	Q_INVOKABLE QVariantMap get(int index);

	Q_INVOKABLE void selectProfile(int index);

	inline int getSelectedProfileIndex() const { return mSelected; }
	inline QString getSelectedProfileName() const { return index(mSelected).data(NameRole).toString(); }
	inline int getSelectedProfileSkillLevel() const { return index(mSelected).data(SkillRole).toInt(); }

signals:
	void selectedProfileIndexChanged();

	void selectedProfileNameChanged();
	void selectedProfileSkillLevelChanged();

protected:
	QHash<int, QByteArray> roleNames() const;

private:
	DataModel* mDm;

	int mSelected;
};

} /* namespace qtouch */

#endif /* PROFILEMODEL_HPP_ */
