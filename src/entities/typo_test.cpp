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
 * \file typo_test.cpp
 *
 * \date 02.07.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include <QtTest/QtTest>

#include "typo.hpp"

namespace qtouch
{

class TypoTest: public QObject
{
	Q_OBJECT

private slots:
	void init();

	void initialization();
	void copy();
	void resetCount();
	void serialization();

private:
	QSharedPointer<Typo> uut1;
	QSharedPointer<Typo> uut2;

	void compare(const Typo& uut, const Typo& ref);
	void compare(const Typo& uut, const QChar& refChar, quint16 refCount);
};

void TypoTest::init()
{
	uut1.reset(new Typo(QChar('A')));
	uut2.reset(new Typo(QChar('A'), (quint16) 1));
}

void TypoTest::initialization()
{
	compare(*uut1, QChar('A'), (quint16) 0);
	compare(*uut2, QChar('A'), (quint16) 1);
}

void TypoTest::copy()
{
	Typo t1(*uut1);
	compare(*uut1, t1);

	Typo t2(*uut2);
	compare(*uut2, t2);
}

void TypoTest::resetCount()
{
	uut2->resetCount();
	compare(*uut2, QChar('A'), (quint16) 0);
}

void TypoTest::serialization()
{
	QByteArray data;
	QDataStream out(&data, QIODevice::WriteOnly);
	out << *uut2;

	Typo t1;
	QDataStream in(&data, QIODevice::ReadOnly);
	in >> t1;

	compare(t1, *uut2);
}

void TypoTest::compare(const Typo& uut, const Typo& ref)
{
	QCOMPARE(uut.getChar(), ref.getChar());
	QCOMPARE(uut.getCount(), ref.getCount());
}

void TypoTest::compare(const Typo& uut, const QChar& refChar, quint16 refCount)
{
	QCOMPARE(uut.getChar(), refChar);
	QCOMPARE(uut.getCount(), refCount);
}

} /* namespace qtouch */

QTEST_APPLESS_MAIN(qtouch::TypoTest)
#include "typo_test.moc"
