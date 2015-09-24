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
 * \file typo.cpp
 *
 * \date 02.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include "typo.hpp"

namespace qtouch
{

Typo::Typo() :
	mChar('\0'), mCount(0)
{
}

Typo::Typo(QChar const& c) :
	mChar(c), mCount(0)
{
}

Typo::Typo(QChar const& c, quint16 n) :
	mChar(c), mCount(n)
{

}

Typo::~Typo()
{
}

QChar const& Typo::getChar() const
{
	return mChar;
}

quint16 Typo::getCount() const
{
	return mCount;
}

void Typo::incrementCount()
{
	mCount++;
}

void Typo::resetCount()
{
	mCount = 0;
}

QDataStream& operator<<(QDataStream& out, Typo const& typo)
{
	out << typo.getCount() << typo.getChar();
	return out;
}

QDataStream& operator>>(QDataStream& in, Typo& typo)
{
	quint16 i;
	QChar c;
	in >> i >> c;
	typo = Typo(c, i);
	return in;
}

} /* namespace qtouch */
