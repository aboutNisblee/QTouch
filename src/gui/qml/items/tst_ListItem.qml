/* Copyright (C) 2015  Moritz Nisblé <moritz.nisble@gmx.de>
**
** This file is part of QTouch.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License along
** with this program; if not, write to the Free Software Foundation, Inc.,
** 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
**/
import QtQuick 2.3
import QtTest 1.0

TestCase {
    name: "ListItemTests"

    property string testText: "TestText"
    property string testIconSource: "qrc:/icons/64x64/user-identity.png"

    ListItem {
        id: uut

        text: testText
        iconSource: testIconSource
    }

    function test_props() {
        compare(uut.text, testText)
        compare(uut.iconSource, testIconSource)
    }
}
