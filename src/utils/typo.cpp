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
