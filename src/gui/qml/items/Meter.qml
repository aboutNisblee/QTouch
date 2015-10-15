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
import QtGraphicalEffects 1.0

Item {
    id: root

    property bool showBackground: false
    property int current: 0
    property int target: 0
    property int min: 0
    property int max: 100
    property color targetMarkColor: '#50FF3C'

    onTargetChanged: canvas.requestPaint()
    onMinChanged: canvas.requestPaint()
    onMaxChanged: canvas.requestPaint()
    onTargetMarkColorChanged: canvas.requestPaint()

    function getMin() {
        return Math.min(min, max)
    }
    function getMax() {
        return Math.max(min, max)
    }
    function getTarget() {
        return Math.max(Math.min(target, getMax()), getMin())
    }

    Image {
        id: background
        anchors.fill: parent
        z: 0
        visible: showBackground
        sourceSize.width: 1024
        sourceSize.height: 1024
        fillMode: Image.PreserveAspectFit
        source: "image://svgelement/meter.svgz#background"
    }

    Image {
        id: ring
        anchors.fill: parent
        z: 1
        sourceSize.width: 1024
        sourceSize.height: 1024
        fillMode: Image.PreserveAspectFit
        source: "image://svgelement/meter.svgz#ring"
    }

    Image {
        id: scale
        anchors.fill: parent
        z: 3
        sourceSize.width: 1024
        sourceSize.height: 1024
        fillMode: Image.PreserveAspectFit
        source: "image://svgelement/meter.svgz#scale"
        transform: Translate {
            y: ring.height / 5
        }
    }

    DropShadow {
        id: hand
        anchors.fill: parent
        z: 4

        source: Image {
            sourceSize.width: 1024
            sourceSize.height: 1024
            fillMode: Image.PreserveAspectFit
            source: "image://svgelement/meter.svgz#hand"
            visible: false
        }

        color: "black"
        radius: 1.5
        samples: 4
        spread: 0

        rotation: Math.min(current / (getMax() - getMin()), 1.0) * 90 - 45
        transform: Translate {
            y: ring.height / 5
        }
    }

    Text {
        id: txtMin
        anchors {
            left: parent.left
            top: parent.top
            leftMargin: ring.width / 5
            topMargin: ring.height / 2.3
        }
        height: parent.height / 12
        z: 2
        horizontalAlignment: Text.AlignLeft
        maximumLineCount: 1
        minimumPixelSize: 5
        font.pixelSize: 72
        fontSizeMode: Text.VerticalFit
        text: getMin()
    }

    Text {
        id: txtMax
        anchors {
            right: parent.right
            top: parent.top
            rightMargin: ring.width / 5
            topMargin: ring.height / 2.3
        }
        height: parent.height / 12
        z: 2
        horizontalAlignment: Text.AlignRight
        maximumLineCount: 1
        minimumPixelSize: 5
        font.pixelSize: 72
        fontSizeMode: Text.VerticalFit
        text: getMax()
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        z: 2
        onPaint: {
            var ctx = canvas.getContext('2d')
            ctx.reset()

            var centreX = width / 2
            var centreY = height / 2 + ring.height / 5
            var fraction = Math.max(1 - getTarget() / (getMax() - getMin()),
                                    0.0)
            var startAngle = -(45.0 / 360.0) * (2 * Math.PI)
            var endAngle = -((45.0 + fraction * 90) / 360.0) * (2 * Math.PI)

            // Draw target mark
            ctx.beginPath()
            ctx.arc(centreX, centreY, height * 0.85 / 2, startAngle,
                    endAngle, true)
            ctx.lineWidth = height / 20
            ctx.strokeStyle = targetMarkColor
            ctx.stroke()
        }
    }
}
