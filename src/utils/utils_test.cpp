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
 * \file utils_test.cpp
 *
 * \date 03.08.2015
 * \author Moritz Nisblé moritz.nisble@gmx.de
 */

#include <QtTest/QtTest>

#include "utils.hpp"

namespace qtouch
{

class UtilsTest: public QObject
{
	Q_OBJECT

private slots:
	void smartPtrTraitsTest();
	void getValueTest();
	void scopedFlagTest();
};

void UtilsTest::smartPtrTraitsTest()
{
	// is_unique_ptr
	// value
	QVERIFY(false == is_unique_ptr<int>::value);
	QVERIFY(false == is_unique_ptr<const int>::value);
	QVERIFY(true == is_unique_ptr<std::unique_ptr<void>>::value);
	QVERIFY(true == is_unique_ptr<const std::unique_ptr<void>>::value);

	// pointer
	QVERIFY(false == is_unique_ptr<int*>::value);
	QVERIFY(false == is_unique_ptr<const int*>::value);
	QVERIFY(false == is_unique_ptr<const int* const>::value);

	QVERIFY(true == is_unique_ptr< std::unique_ptr<void>* >::value);
	QVERIFY(true == is_unique_ptr< const std::unique_ptr<void>* >::value);
	QVERIFY(true == is_unique_ptr< const std::unique_ptr<void>* const >::value);

	// reference
	QVERIFY(false == is_unique_ptr< int& >::value);
	QVERIFY(false == is_unique_ptr< const int& >::value);
	QVERIFY(false == is_unique_ptr < int&& >::value);
	QVERIFY(false == is_unique_ptr < const int&& >::value);

	QVERIFY(true == is_unique_ptr< std::unique_ptr<void>& >::value);
	QVERIFY(true == is_unique_ptr< const std::unique_ptr<void>& >::value);
	QVERIFY(true == is_unique_ptr < std::unique_ptr<void> && >::value);
	QVERIFY(true == is_unique_ptr < const std::unique_ptr<void> && >::value);

	// void
	QVERIFY(false == is_unique_ptr<void>::value);
	QVERIFY(false == is_unique_ptr<void*>::value);

	// std::shared_ptr
	QVERIFY(false == is_unique_ptr<std::shared_ptr<void>>::value);


	// is_shared_ptr
	// value
	QVERIFY(false == is_shared_ptr<int>::value);
	QVERIFY(false == is_shared_ptr<const int>::value);
	QVERIFY(true == is_shared_ptr<std::shared_ptr<void>>::value);
	QVERIFY(true == is_shared_ptr<const std::shared_ptr<void>>::value);

	// pointer
	QVERIFY(false == is_shared_ptr<int*>::value);
	QVERIFY(false == is_shared_ptr<const int*>::value);
	QVERIFY(false == is_shared_ptr<const int* const>::value);

	QVERIFY(true == is_shared_ptr< std::shared_ptr<void>* >::value);
	QVERIFY(true == is_shared_ptr< const std::shared_ptr<void>* >::value);
	QVERIFY(true == is_shared_ptr< const std::shared_ptr<void>* const >::value);

	// reference
	QVERIFY(false == is_shared_ptr< int& >::value);
	QVERIFY(false == is_shared_ptr< const int& >::value);
	QVERIFY(false == is_shared_ptr < int&& >::value);
	QVERIFY(false == is_shared_ptr < const int&& >::value);

	QVERIFY(true == is_shared_ptr< std::shared_ptr<void>& >::value);
	QVERIFY(true == is_shared_ptr< const std::shared_ptr<void>& >::value);
	QVERIFY(true == is_shared_ptr < std::shared_ptr<void> && >::value);
	QVERIFY(true == is_shared_ptr < const std::shared_ptr<void> && >::value);

	// void
	QVERIFY(false == is_shared_ptr<void>::value);
	QVERIFY(false == is_shared_ptr<void*>::value);

	// std::unique_ptr
	QVERIFY(false == is_shared_ptr<std::unique_ptr<void>>::value);


	// is_smart_ptr
	// value
	QVERIFY(false == is_smart_ptr<int>::value);
	QVERIFY(false == is_smart_ptr<const int>::value);
	QVERIFY(true == is_smart_ptr<std::unique_ptr<void>>::value);
	QVERIFY(true == is_smart_ptr<const std::shared_ptr<void>>::value);

	// pointer
	QVERIFY(false == is_smart_ptr<int*>::value);
	QVERIFY(false == is_smart_ptr<const int*>::value);
	QVERIFY(false == is_smart_ptr<const int* const>::value);

	QVERIFY(true == is_smart_ptr< std::unique_ptr<void>* >::value);
	QVERIFY(true == is_smart_ptr< const std::shared_ptr<void>* >::value);
	QVERIFY(true == is_smart_ptr< const std::unique_ptr<void>* const >::value);

	// reference
	QVERIFY(false == is_smart_ptr< int& >::value);
	QVERIFY(false == is_smart_ptr< const int& >::value);
	QVERIFY(false == is_smart_ptr < int&& >::value);
	QVERIFY(false == is_smart_ptr < const int&& >::value);

	QVERIFY(true == is_smart_ptr< std::shared_ptr<void>& >::value);
	QVERIFY(true == is_smart_ptr< const std::unique_ptr<void>& >::value);
	QVERIFY(true == is_smart_ptr < std::shared_ptr<void> && >::value);
	QVERIFY(true == is_smart_ptr < const std::unique_ptr<void> && >::value);

	// void
	QVERIFY(false == is_smart_ptr<void>::value);
	QVERIFY(false == is_smart_ptr<void*>::value);
}

void UtilsTest::getValueTest()
{
	int one = 42;
	const int cOne = 42;
	const int& cOneRef = 42;

	auto uP2int = std::unique_ptr<int>(new int(42));
	const auto cUp2int = std::unique_ptr<int>(new int(42));
	const auto& cUpRef2int = std::unique_ptr<int>(new int(42));

	auto sP2int = std::shared_ptr<int>(new int(42));
	const auto cSp2int = std::shared_ptr<int>(new int(42));
	const auto& cSpRef2int = std::shared_ptr<int>(new int(42));

	QVERIFY(42 == value(one));
	QVERIFY(42 == value(cOne));
	QVERIFY(42 == value(&one));
	QVERIFY(42 == value(&cOne));
	QVERIFY(42 == value(cOneRef));
	QVERIFY(42 == value(42)); // rvalue

	QVERIFY(42 == value(uP2int));
	QVERIFY(42 == value(cUp2int));
	QVERIFY(42 == value(cUpRef2int));

	QVERIFY(42 == value(sP2int));
	QVERIFY(42 == value(cSp2int));
	QVERIFY(42 == value(cSpRef2int));
}

void UtilsTest::scopedFlagTest()
{
	bool flag = false;

	{
		ScopedFlag f(flag);
		QVERIFY(true == flag);
	}

	QVERIFY(false == flag);
}

} /* namespace qtouch */

QTEST_GUILESS_MAIN(qtouch::UtilsTest)
#include "utils_test.moc"
