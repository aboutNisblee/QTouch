/* Copyright (C) 2015  Moritz Nisblé <moritz.nisble@gmx.de>
**
** This file is part of QTouch.
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU Lesser General Public License as published
** by the Free Software Foundation, either version 3 of the License,
** or (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
**/
import QtQuick 2.0
import QtQuick.Controls 1.3

Item {
    id: root

    property int current: 0
    property int min: 0
    property int max: 360
    property int target: 180
    property int previous: 0
    property string heading: qsTr("Heading")
    property string postifx
    property int meterTextSpacing: 20
    property int textMargins: 2

    implicitWidth: meter.width + Math.max(
                       lblHeading.contentWidth, lblCurrent.contentWidth,
                       lblPrevious.contentWidth) + meterTextSpacing
    implicitHeight: lblHeading.minimumPixelSize + lblCurrent.minimumPixelSize
                    + lblPrevious.minimumPixelSize + 4 * textMargins

    Meter {
        id: meter
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
        }
        width: height
        current: root.current
        target: root.target
        min: root.min
        max: root.max
    }

    Label {
        id: lblHeading
        anchors {
            top: parent.top
            left: meter.right
            leftMargin: meterTextSpacing
            topMargin: textMargins
        }
        height: parent.height * 0.2
        text: heading
        fontSizeMode: Text.VerticalFit
        minimumPixelSize: 15
        font.pixelSize: 32
        elide: Text.ElideRight
        verticalAlignment: Text.AlignTop
    }

    Label {
        id: lblCurrent
        anchors {
            top: lblHeading.bottom
            bottom: lblPrevious.top
            left: meter.right
            leftMargin: meterTextSpacing
            topMargin: textMargins
            bottomMargin: textMargins
        }
        text: current.toString() + postifx
        fontSizeMode: Text.VerticalFit
        minimumPixelSize: 30
        font.pixelSize: 72
        verticalAlignment: Text.AlignVCenter
    }

    Label {
        id: lblPrevious
        anchors {
            bottom: parent.bottom
            left: meter.right
            leftMargin: meterTextSpacing
            bottomMargin: textMargins
        }
        height: parent.height * 0.2
        text: previous.toString() + postifx
        fontSizeMode: Text.VerticalFit
        minimumPixelSize: 15
        font.pixelSize: 32
        verticalAlignment: Text.AlignBottom
    }
}
