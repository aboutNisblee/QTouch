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
#include <QQmlListProperty>

#include "wrapper/qmlprofile.hpp"

namespace qtouch
{

class DataModel;

class ProfileModel: public QAbstractListModel
{
	Q_OBJECT

	Q_ENUMS(qtouch::QmlProfile::SkillLevel)
	Q_PROPERTY(int index READ getIndex WRITE selectProfile NOTIFY profileIndexChanged)
	Q_PROPERTY(qtouch::QmlProfile* profile READ getProfile NOTIFY profileChanged)

public:
	/** ProfileModelRoles */
	enum ProfileModelRoles
	{
		NameRole = Qt::UserRole + 1,//!< NameRole pName
		SkillRole                  //!< SkillRole pSkill
	};

	explicit ProfileModel(QObject* parent = nullptr);
	explicit ProfileModel(DataModel* model, QObject* parent = nullptr);
	virtual ~ProfileModel();

	int rowCount(const QModelIndex& parent = QModelIndex()) const Q_DECL_OVERRIDE;
	QVariant data(const QModelIndex& index, int role) const Q_DECL_OVERRIDE;

	Q_INVOKABLE QVariantMap get(int index);

	inline int getIndex() const { return mSelected; }
	void selectProfile(int index);

	/* Objects not-created by QML have CppOwnership by default.
	 * The exception to this are objects returned from C++ method calls;
	 * their ownership will be set to JavaScriptOwnership.
	 * This applies only to explicit invocations of Q_INVOKABLE methods or slots,
	 * but not to property getter invocations. */
	QmlProfile* getProfile() const;

	Q_INVOKABLE bool addProfile(const QString& name, QmlProfile::SkillLevel skill);
	Q_INVOKABLE bool addProfile(QmlProfile* profile);

signals:
	void profileIndexChanged();
	void profileChanged();

protected:
	virtual QHash<int, QByteArray> roleNames() const Q_DECL_OVERRIDE;

private:
	DataModel* mDm;

	int mSelected;
};

} /* namespace qtouch */

#endif /* PROFILEMODEL_HPP_ */
