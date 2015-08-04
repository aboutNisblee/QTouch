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
