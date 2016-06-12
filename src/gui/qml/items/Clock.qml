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
import QtQuick 2.5
import QtGraphicalEffects 1.0

Item {
    id: root

    property int seconds: 0
    property int shAnimationTime: 1000
    property int mhAnimationTime: 0
    property bool continuousMinuteHand: false
    property bool showBackground: false

    Image {
        id: background
        anchors.fill: parent
        visible: showBackground
        sourceSize.width: 1024
        sourceSize.height: 1024
        fillMode: Image.PreserveAspectFit
        source: "image://svgelement/clock.svgz#background"
    }

    Image {
        id: face
        anchors.fill: parent
        sourceSize.width: 1024
        sourceSize.height: 1024
        fillMode: Image.PreserveAspectFit
        source: "image://svgelement/clock.svgz#face"
    }

    Image {
        id: minutehand_image
        anchors.centerIn: face
        width: face.width * 0.85
        height: face.height * 0.85
        sourceSize.width: 1024
        sourceSize.height: 1024
        fillMode: Image.PreserveAspectFit
        source: "image://svgelement/clock.svgz#minute-hand"
        visible: false
    }

    DropShadow {
        id: minutehand
        anchors.fill: minutehand_image

        source: minutehand_image

        color: "black"
        radius: 1.5
        samples: 4
        spread: 0

        property int min: seconds / 60
        onMinChanged: {
            if (!continuousMinuteHand)
                rotation = min * 6
        }

        Behavior on rotation {
            NumberAnimation {
                easing.type: Easing.InQuad
                duration: mhAnimationTime
            }
        }
    }

    onSecondsChanged: {
        if (continuousMinuteHand)
            minutehand.rotation = seconds / 60 * 6
    }

    Image {
        id: secondhand_image
        anchors.centerIn: face
        width: face.width * 0.85
        height: face.height * 0.85
        sourceSize.width: 1024
        sourceSize.height: 1024
        fillMode: Image.PreserveAspectFit
        source: "image://svgelement/clock.svgz#second-hand"
        visible: false
    }

    DropShadow {
        id: secondhand
        anchors.fill: secondhand_image

        source: secondhand_image

        color: "black"
        radius: 1.5
        samples: 4
        spread: 0

        rotation: seconds * 6

        Behavior on rotation {
            NumberAnimation {
                easing.type: Easing.InQuad
                duration: shAnimationTime
            }
        }
    }
}
