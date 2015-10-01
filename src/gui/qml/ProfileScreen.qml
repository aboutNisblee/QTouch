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
import QtQuick.Controls 1.3

import "items" as Items

FocusScope {
    id: root

    // Input property interface
    // Enable to open
    property bool open: false
    // Set current profile model
    property alias profileModel: list.model

    // Output property interface
    // The index of the currently selected course
    property int selectedProfileIndex: 0

    enabled: open
    visible: (background.height === -height) ? false : true
    clip: true

    states: State {
        name: "OPEN"
        when: open
        PropertyChanges {
            target: background
            y: 0
        }
    } // states

    Rectangle {
        id: background
        width: parent.width
        height: parent.height
        y: -parent.height

        Row {
            anchors.fill: parent

            ListView {
                id: list
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                    margins: 20
                }
                width: parent.width / 2

                focus: true

                delegate: Items.ListItem {
                    anchors {
                        left: parent.left
                        right: parent.right
                        margins: 20
                    }

                    text: name
                    iconSource: "qrc:/icons/32x32/user-identity.png"

                    topMargin: 6
                    bottomMargin: 6
                    hoverable: true
                    bgRadius: 5
                    bgBorderWidth: 1
                    bgBorderColor: "black"
                    bgMaxOpacity: 0

                    isCurrentItem: ListView.isCurrentItem

                    onClicked: {
                        ListView.view.currentIndex = index
                        console.debug("Profile changed!")
                    }
                } // delegate

                highlight: Component {
                    Rectangle {
                        anchors {
                            left: parent.left
                            right: parent.right
                            margins: 20
                        }

                        height: list.currentItem.height
                        width: list.currentItem.width

                        border.width: list.currentItem.bgBorderWidth
                        border.color: list.currentItem.bgBorderColor
                        radius: list.currentItem.bgRadius

                        x: list.currentItem.x
                        y: list.currentItem.y
                        Behavior on y {
                            SpringAnimation {
                                spring: 10
                                damping: 0.8
                            }
                        }
                    }
                } // highlight

                footer: Items.ListItem {
                    anchors {
                        left: parent.left
                        right: parent.right
                        margins: 20
                    }

                    topMargin: 12

                    text: qsTr("New profile")
                    iconSource: "qrc:/icons/32x32/list-add.png"

                    onClicked: {
                        console.debug("New Profile!")
                    }
                } // footer
            } // profileSelectorView

            Rectangle {
                id: rhsBackground
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                }

                width: parent.width / 2
                color: "beige"

                Item {
                    id: newProfileContainer

                    anchors {
                        fill: parent
                        bottomMargin: parent.height / 4
                        topMargin: parent.height / 4
                        leftMargin: 10
                        rightMargin: 10
                    }

                    TextField {
                        id: txtProfileName
                        anchors {
                            top: parent.top
                            right: parent.right
                            left: parent.left
                        }
                        placeholderText: qsTr("Text Field")
                    }

                    GroupBox {
                        id: gbSkillLevel
                        anchors {
                            top: txtProfileName.bottom
                            right: parent.right
                            left: parent.left
                        }
                        height: 200
                        title: qsTr("Group Box")
                    }

                    Button {
                        id: btCreateProfile
                        anchors {
                            top: gbSkillLevel.bottom
                            horizontalCenter: parent.horizontalCenter
                        }
                        text: qsTr("Create Profile")
                    }
                }
            } // rhsBackground
        } // Row

        Behavior on y {
            PropertyAnimation {
                easing.type: Easing.InOutQuad
                duration: 350
            }
        }
    } // background
} // root
