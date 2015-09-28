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

    property int currentMillis: 0
    property int previousMillis: 0
    property int clockTextSpacing: 20
    property int textMargins: 2

    implicitWidth: clock.width + Math.max(
                       lblHeading.contentWidth, lblCurrentTime.contentWidth,
                       lblPreviousTime.contentWidth) + clockTextSpacing
    implicitHeight: lblHeading.minimumPixelSize + lblCurrentTime.minimumPixelSize
                    + lblPreviousTime.minimumPixelSize + 4 * textMargins

    Clock {
        id: clock
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
        }
        width: height
        shAnimationTime: 1000 * 0.8
        mhAnimationTime: 1000
        continuousMinuteHand: false
        seconds: currentMillis / 1000
    }

    Label {
        id: lblHeading
        anchors {
            top: parent.top
            left: clock.right
            leftMargin: clockTextSpacing
            topMargin: textMargins
        }
        height: parent.height * 0.2
        text: qsTr("Elapsed Time")
        fontSizeMode: Text.VerticalFit
        minimumPixelSize: 15
        font.pixelSize: 32
        elide: Text.ElideRight
        verticalAlignment: Text.AlignTop
    }

    Label {
        id: lblCurrentTime
        anchors {
            top: lblHeading.bottom
            bottom: lblPreviousTime.top
            left: clock.right
            leftMargin: clockTextSpacing
            topMargin: textMargins
            bottomMargin: textMargins
        }
        text: Qt.formatTime(new Date(currentMillis), "mm:ss.zzz").substr(0, 7)
        fontSizeMode: Text.VerticalFit
        minimumPixelSize: 30
        font.pixelSize: 72
        verticalAlignment: Text.AlignVCenter
    }

    Label {
        id: lblPreviousTime
        anchors {
            bottom: parent.bottom
            left: clock.right
            leftMargin: clockTextSpacing
            bottomMargin: textMargins
        }
        height: parent.height * 0.2
        text: Qt.formatTime(new Date(previousMillis), "mm:ss.zzz").substr(0, 7)
        fontSizeMode: Text.VerticalFit
        minimumPixelSize: 15
        font.pixelSize: 32
        verticalAlignment: Text.AlignBottom
    }
}
