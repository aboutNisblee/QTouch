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
	virtual ~Exception() noexcept {}

	virtual void raise() const { throw* this; }
	virtual Exception* clone() const { return new Exception(*this); }

	virtual const char* what() const noexcept { return mMsg.toLatin1().data(); }
	virtual QString message() const noexcept { return mMsg; }

protected:
	QString mMsg;
};

class FileException: public Exception
{
public:
	FileException(const QString& msg, const QString& filename): Exception(msg), mFileName(filename) {}
	virtual ~FileException() noexcept {}

	virtual void raise() const { throw* this; }
	virtual FileException* clone() const { return new FileException(*this); }

	virtual QString message() const noexcept { return mMsg % " in " % mFileName; }
	virtual const QString& filename() const noexcept { return mFileName; }

protected:
	QString mFileName;
};

class XmlException: public Exception
{
public:
	XmlException(const QString& msg, const QString& filename): Exception(msg), mFileName(filename) {}
	virtual ~XmlException() noexcept {}

	virtual void raise() const { throw* this; }
	virtual XmlException* clone() const { return new XmlException(*this); }

	virtual QString message() const noexcept { return mMsg % " in " % mFileName; }
	virtual const QString& filename() const noexcept { return mFileName; }

protected:
	QString mFileName;
};

} /* namespace qtouch */

#endif /* EXCEPTIONS_HPP_ */
