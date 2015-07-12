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
