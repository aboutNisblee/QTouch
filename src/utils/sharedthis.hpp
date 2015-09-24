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
 * \file sharedthis.hpp
 *
 * \date 09.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef SHAREDTHIS_HPP_
#define SHAREDTHIS_HPP_

#include <QSharedPointer>
#include <QWeakPointer>

// *INDENT-OFF*
/* XXX: Since 5.4 there is a base class for this purpose
 * called QEnableSharedFromThis. */
class SharedThis
{
protected:
	SharedThis() {}

	/* Prevent coping the weak pointer when children are copied. */
	SharedThis(const SharedThis&) {}
	virtual SharedThis& operator=(const SharedThis&) { return *this; }
	virtual ~SharedThis() {}

	void initWeakThis(const QSharedPointer<SharedThis>& thiz) { mWeakThis = thiz; }

	/**
	 * Get a shared this pointer, increasing the reference count of other
	 * shared pointer to this object.
	 * @return
	 */
	QSharedPointer<SharedThis> sharedFromWeakThis()
	{
		QSharedPointer<SharedThis> strong = mWeakThis.lock();
		return strong;
	}

	QWeakPointer<SharedThis> mWeakThis;
};
// *INDENT-ON*

#endif /* SHAREDTHIS_HPP_ */
