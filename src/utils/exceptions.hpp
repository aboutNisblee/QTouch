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

/**
 * Base class for all exceptions in QTouch.
 * Each subclass initializes a standard message. This basic message can be
 * retrieved by the what() function. Some subclasses add some additional
 * details. Those details should be always accessible via additional getters.
 * For convenience each subclass should overwrite the message method and return
 * a formatted QString that adds its details to the standard message.
 *
 * @note To be able to throw exceptions over thread boundaries they must
 * be thrown by value and caught by reference!
 */
class Exception: public QException
{
public:
	explicit Exception(QString msg): msg(msg) {}
	virtual ~Exception() throw () {}

	virtual void raise() const { throw* this; }
	virtual Exception* clone() const { return new Exception(*this); }

	virtual const char* what() const throw () { return msg.toLatin1().data(); }
	virtual QString message() const throw () { return msg; }

protected:
	QString msg;
};

struct FileException: public Exception
{
public:
	FileException(QString msg, QString file): Exception(msg), file(file) {}
	virtual ~FileException() throw () {}

	virtual void raise() const { throw* this; }
	virtual FileException* clone() const { return new FileException(*this); }

	virtual QString message() const throw () { return msg % " in " % file; }
	virtual const QString& filename() const throw() { return file; }

protected:
	QString file;
};

struct XmlException: public Exception
{
public:
	XmlException(QString msg, QString file): Exception(msg), file(file) {}
	virtual ~XmlException() throw () {}

	virtual void raise() const { throw* this; }
	virtual XmlException* clone() const { return new XmlException(*this); }

	virtual QString message() const throw () { return msg % " in " % file; }
	virtual const QString& filename() const throw() { return file; }

protected:
	QString file;
};

struct DatabaseException: public Exception
{
public:
	DatabaseException(QString msg): Exception(msg) {}
	virtual ~DatabaseException() throw () {}

	virtual void raise() const { throw* this; }
	virtual DatabaseException* clone() const { return new DatabaseException(*this); }

	virtual QString message() const throw() { return QStringLiteral(""); }
};

struct SqlDriverException: public DatabaseException
{
public:
	SqlDriverException(QString msg, QString drvMsg): DatabaseException(msg), drvMsg(drvMsg) {}
	virtual ~SqlDriverException() throw () {}

	virtual void raise() const { throw* this; }
	virtual SqlDriverException* clone() const { return new SqlDriverException(*this); }

	virtual QString message() const throw() { return msg % " -- Driver message: " % drvMsg; }
	virtual const QString& driverMessage() const throw() { return drvMsg; }

protected:
	QString drvMsg;
};

struct SqlException: public DatabaseException
{
public:
	SqlException(QString msg, QString dbMsg): DatabaseException(msg), dbMsg(dbMsg) {}
	virtual ~SqlException() throw () {}

	virtual void raise() const { throw* this; }
	virtual SqlException* clone() const { return new SqlException(*this); }

	virtual QString message() const throw() { return msg % " -- Database message: " % dbMsg; }
	virtual const QString& databaseMessage() const throw() { return dbMsg; }

protected:
	QString dbMsg;
};


} /* namespace qtouch */

#endif /* EXCEPTIONS_HPP_ */
