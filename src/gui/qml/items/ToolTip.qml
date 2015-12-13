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

// The tooltip component is based on an implementation from Xander84
// found on https://wiki.qt.io/QtQuick_ToolTip_Component
import QtQuick 2.2

Rectangle {
    id: tooltip

    property alias text: tooltipText.text
    property alias textItem: tooltipText
    property int fadeInDelay: 500
    property int fadeOutDelay: 500

    function show() {
        state = "VISIBLE"
    }

    function hide() {
        state = "INVISIBLE"
    }

    width: tooltipText.width + 20
    height: tooltipText.height + 10
    color: "#dd000000"
    radius: 6
    opacity: 0

    Text {
        id: tooltipText
        anchors.centerIn: parent
        horizontalAlignment: Text.AlignHCenter
        color: "white"
        font.pointSize: 10
        font.bold: true
    }

    MouseArea {
        x: -8
        y: -8
        width: 16
        height: 16
        hoverEnabled: true
        onEntered: show()
        onExited: hide()
    }

    states: [
        State {
            name: "VISIBLE"
            PropertyChanges {
                target: tooltip
                opacity: 1
            }
        },
        State {
            name: "INVISIBLE"
            PropertyChanges {
                target: tooltip
                opacity: 0
            }
        }
    ]

    transitions: [
        Transition {
            to: "VISIBLE"
            NumberAnimation {
                target: tooltip
                property: "opacity"
                duration: fadeInDelay
            }
        },
        Transition {
            to: "INVISIBLE"
            NumberAnimation {
                target: tooltip
                property: "opacity"
                duration: fadeOutDelay
            }
        }
    ]
}
