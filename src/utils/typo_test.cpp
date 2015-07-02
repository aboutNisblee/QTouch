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
