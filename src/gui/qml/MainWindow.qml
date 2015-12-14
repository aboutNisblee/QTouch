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
import QtQuick.Window 2.1
import QtQuick.Controls 1.3

import de.nisble.qtouch 1.0

Window {
    //    id: mainWindow
    width: 1250
    height: 700
    //    color: "transparent"
    //    color: "#E1E1E1"
    visible: true

    onActiveFocusItemChanged: console.debug(
                                  "Active focus given to: " + activeFocusItem)

    minimumWidth: trainingScreen.implicitWidth
    minimumHeight: 500

    Flipable {
        id: flipper

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: statusBar.top
        }

        // HomeScreen and ProfileScreen
        front: HomeScreen {
            id: homeScreen

            anchors.fill: parent

            // Controlled by transition
            visible: true
            enabled: visible
            focus: !trainingScreen.focus

            courseModel: $courseModel
            profileModel: $profileModel
            document {
                title: $courseModel.lessonModel.lesson.title
                text: $courseModel.lessonModel.lesson.text
                documentMargin: 30
            }

            onLessonStarted: flipper.state = "TRAINING"
        } // homeScreen

        back: TrainingScreen {
            id: trainingScreen

            anchors.fill: parent

            // Controlled by transition
            visible: false
            enabled: visible
            focus: visible

            onVisibleChanged: reset()

            document {
                title: $courseModel.lessonModel.lesson.title
                text: $courseModel.lessonModel.lesson.text
                documentMargin: 40
            }

            stats {
                course: $courseModel.course.id
                lesson: $courseModel.lessonModel.lesson.id
                profile: $profileModel.profile.name
                chars: $courseModel.lessonModel.lesson.text.length
            }

            onQuit: flipper.state = ""

            onFinished: {
                var profile = $profileModel.profile
                profile.pushStats(stats)
                $profileModel.save(profile)
            }
        } // trainingScreen

        transform: Rotation {
            id: rotation
            origin.x: flipper.width / 2
            origin.y: flipper.height / 2
            axis.x: 1
            axis.y: 0
            axis.z: 0 // set axis.y to 1 to rotate around y-axis
            angle: 0 // the default angle
        } // rotation

        states: State {
            name: "TRAINING"
            PropertyChanges {
                target: rotation
                angle: 180
            }
        }

        transitions: [
            Transition {
                to: "TRAINING"
                SequentialAnimation {
                    PropertyAction {
                        target: trainingScreen
                        property: "visible"
                        value: true
                    }
                    NumberAnimation {
                        target: rotation
                        property: "angle"
                        easing.type: Easing.InOutQuad
                        duration: 500
                    }
                    PropertyAction {
                        target: homeScreen
                        property: "visible"
                        value: false
                    }
                }
            },
            Transition {
                from: "TRAINING"
                SequentialAnimation {
                    PropertyAction {
                        target: homeScreen
                        property: "visible"
                        value: true
                    }
                    NumberAnimation {
                        target: rotation
                        property: "angle"
                        easing.type: Easing.InOutQuad
                        duration: 500
                    }
                    PropertyAction {
                        target: trainingScreen
                        property: "visible"
                        value: false
                    }
                }
            }
        ] // flipper transitions
    } // flipper

    StatusBar {
        id: statusBar
        anchors {
            right: parent.right
            left: parent.left
            bottom: parent.bottom
        }

        Row {
            anchors.fill: parent
            Label {
                text: "Read Only"
            }
        }
    } // statusBar
} // mainWindow
