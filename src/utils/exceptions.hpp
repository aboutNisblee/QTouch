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
	explicit Exception(const QString& msg): mMsg(msg) {}
	virtual ~Exception() throw () {}

	virtual void raise() const { throw* this; }
	virtual Exception* clone() const { return new Exception(*this); }

	virtual const char* what() const throw () { return mMsg.toLatin1().data(); }
	virtual QString message() const throw () { return mMsg; }

protected:
	QString mMsg;
};

class FileException: public Exception
{
public:
	FileException(const QString& msg, const QString& filename): Exception(msg), mFileName(filename) {}
	virtual ~FileException() throw () {}

	virtual void raise() const { throw* this; }
	virtual FileException* clone() const { return new FileException(*this); }

	virtual QString message() const throw () { return mMsg % " in " % mFileName; }
	virtual const QString& filename() const throw() { return mFileName; }

protected:
	QString mFileName;
};

class XmlException: public Exception
{
public:
	XmlException(const QString& msg, const QString& filename): Exception(msg), mFileName(filename) {}
	virtual ~XmlException() throw () {}

	virtual void raise() const { throw* this; }
	virtual XmlException* clone() const { return new XmlException(*this); }

	virtual QString message() const throw () { return mMsg % " in " % mFileName; }
	virtual const QString& filename() const throw() { return mFileName; }

protected:
	QString mFileName;
};

} /* namespace qtouch */

#endif /* EXCEPTIONS_HPP_ */
