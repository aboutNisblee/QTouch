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
import QtQuick.Controls 1.2

Item {
    id: root

    property string text
    property bool inflated: false

    height: label.height
    width: label.width

    Label {
        id: label

        text: root.text

        height: 0
        opacity: 0

        states: State {
            name: "VISIBLE"
            when: inflated
            PropertyChanges {
                target: label
                height: contentHeight
                opacity: 1
            }
        } // states

        transitions: [
            Transition {
                to: "VISIBLE"
                SequentialAnimation {
                    PropertyAnimation {
                        property: "height"
                        easing.type: Easing.Linear
                        duration: 150
                    }
                    PropertyAnimation {
                        property: "opacity"
                        easing.type: Easing.Linear
                        duration: 150
                    }
                }
            },
            Transition {
                from: "VISIBLE"
                SequentialAnimation {
                    PropertyAnimation {
                        property: "opacity"
                        easing.type: Easing.Linear
                        duration: 150
                    }
                    PropertyAnimation {
                        property: "height"
                        easing.type: Easing.Linear
                        duration: 150
                    }
                }
            }
        ] // transitions
    } // Label
} //root
