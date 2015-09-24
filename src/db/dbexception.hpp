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
 * \file dbexception.hpp
 *
 * \date 31.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef DBEXCEPTION_HPP_
#define DBEXCEPTION_HPP_

#include "utils/exceptions.hpp"
#include <QSqlError>

namespace qtouch
{

class DbException: public Exception
{
public:
	explicit DbException(const QString& msg): Exception(msg) {}
	DbException(const QString& msg, const QSqlError& sqlError): Exception(msg), mSqlError(sqlError) {}
	virtual ~DbException() noexcept {}

	virtual void raise() const { throw* this; }
	virtual DbException* clone() const { return new DbException(*this); }

	virtual QString message() const noexcept
	{
		QString msg = mMsg;
		if (!mSqlError.databaseText().isEmpty())
		{
			msg.append("\n\tDatabase message: " % mSqlError.databaseText());
			if (!mSqlError.nativeErrorCode().isEmpty())
				msg.append(" (" % mSqlError.nativeErrorCode() % ")");
		}
		if (!mSqlError.driverText().isEmpty())
			msg.append("\n\tDriver message: " % mSqlError.driverText());
		if (mSqlError.type() != QSqlError::NoError)
			msg.append("\n\tType: " % QString().setNum(mSqlError.type()));
		return msg;
	}

	virtual QString databaseMessage() const noexcept { return mSqlError.databaseText(); }
	virtual QString driverMessage() const noexcept { return mSqlError.driverText(); }

	virtual const QSqlError& sqlError() const noexcept { return mSqlError; }
	virtual int sqlErrorCode() const noexcept { return mSqlError.nativeErrorCode().toInt(); }

protected:
	QSqlError mSqlError;
};

} /* namespace qtouch */

#endif /* DBEXCEPTION_HPP_ */
