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
 * \file typo.hpp
 *
 * \date 02.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#ifndef TYPO_HPP
#define TYPO_HPP

#include <QDataStream>

namespace qtouch
{

/**
 * @brief Container that holds a single typo.
 * This class is used to count typos of a single character.
 * The character can come from nearly any language because it is
 * stored as a 16-bit Unicode. The typo count can be incremented
 * and reset in either case a corresponding signal is emitted.
 */
class Typo
{
public:
	Typo();
	explicit Typo(QChar const&);
	Typo(QChar const&, quint16);
	virtual ~Typo();

	QChar const& getChar() const;
	quint16 getCount() const;

	void incrementCount();
	void resetCount();

private:
	QChar mChar;
	quint16 mCount;
};

QDataStream& operator<<(QDataStream& out, Typo const& typo);
QDataStream& operator>>(QDataStream& in, Typo& typo);

} /* namespace qtouch */

#endif // TYPO_HPP
