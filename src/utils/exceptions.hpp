/**
 * \file exceptions.hpp
 *
 * \date 02.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef EXCEPTIONS_HPP_
#define EXCEPTIONS_HPP_

#include <QException>
#include <QString>

namespace qtouch
{

/*
 * NOTE: QException subclasses must be thrown by value and caught by reference!
 */

struct Exception: public QException
{
	explicit Exception(QString msg): message(msg) {}
	virtual ~Exception() throw () {}

	virtual void raise() const { throw *this; }
	virtual Exception *clone() const { return new Exception(*this); }

	virtual const char* what() const throw () { return message.toLatin1().data(); }

	QString message;
};

struct FileException: public Exception
{
	FileException(QString msg, QString file): Exception(msg), filename(file) {}
	virtual ~FileException() throw () {}

	virtual void raise() const { throw *this; }
	virtual FileException *clone() const { return new FileException(*this); }

	QString filename;
};

struct XmlException: public Exception
{
	XmlException(QString msg, QString file): Exception(msg), filename(file) {}
	virtual ~XmlException() throw () {}

	virtual void raise() const { throw *this; }
	virtual XmlException *clone() const { return new XmlException(*this); }

	QString filename;
};

struct DatabaseException: public Exception
{
	DatabaseException(QString msg): Exception(msg) {}
	virtual ~DatabaseException() throw () {}

	virtual void raise() const { throw *this; }
	virtual DatabaseException *clone() const { return new DatabaseException(*this); }

	virtual QString const& dbErrMessage() const { return QStringLiteral(""); }
};

struct SqlDriverException: public DatabaseException
{
	SqlDriverException(QString msg, QString drvMsg): DatabaseException(msg), driverMessage(drvMsg) {}
	virtual ~SqlDriverException() throw () {}

	virtual void raise() const { throw *this; }
	virtual SqlDriverException *clone() const { return new SqlDriverException(*this); }

	virtual QString const& dbErrMessage() const { return driverMessage; }

	QString driverMessage;
};

struct SqlException: public DatabaseException
{
	SqlException(QString msg, QString dbMsg): DatabaseException(msg), databaseMessage(dbMsg) {}
	virtual ~SqlException() throw () {}

	virtual void raise() const { throw *this; }
	virtual SqlException *clone() const { return new SqlException(*this); }

	virtual QString const& dbErrMessage() const { return databaseMessage; }

	QString databaseMessage;
};


} /* namespace qtouch */

#endif /* EXCEPTIONS_HPP_ */
