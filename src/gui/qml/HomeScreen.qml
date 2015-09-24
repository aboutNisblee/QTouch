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

    signal lessonStarted
    Component.onCompleted: {
        lessonSelector.onLessonStarted.connect(lessonStarted)
    }

    states: State {
        name: "PROFILE"
        when: btProfile.checked
    }

    // Forward keys to children
    Keys.forwardTo: [courseSelector, lessonSelector]

    ToolBar {
        id: toolBar
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        ToolButton {
            id: btProfile
            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
                leftMargin: 10
            }
            iconSource: "qrc:/icons/64x64/user-identity.png"
            iconName: "user-identity"
            checkable: true
            //                    onCheckedChanged: home.state = (checked) ? "PROFILE" : ""
        } // btProfile

        ToolButton {
            id: btConfig
            anchors {
                right: parent.right
                verticalCenter: parent.verticalCenter
                rightMargin: 10
            }
            iconSource: "qrc:/icons/64x64/configure.png"
            iconName: "configure"
        } // btConfig
    } // toolBar

    Item {
        id: container

        anchors {
            top: toolBar.bottom
            left: root.left
            right: root.right
            bottom: root.bottom
        }

        CourseSelector {
            id: courseSelector

            anchors {
                top: container.top
                left: container.left
                right: container.right

                topMargin: 5
            }

            currentCourseModel: courseModel

            onSelectedCourseIndexChanged: courseModel.selectCourse(
                                              selectedCourseIndex)
        } // courseSelector

        // Label that is inflated the button that is placed
        // on the PathView delegate of CourseSelector.
        Items.InflatedLabel {
            id: lblCourseDescription

            anchors {
                top: courseSelector.bottom
                topMargin: 5
                right: container.right
                rightMargin: 5
                left: container.left
                leftMargin: 5

                bottomMargin: 10
            }

            // Access course model at the currently selected index and get the description
            text: courseModel.selectedCourseDescription

            inflated: courseSelector.courseDescriptionBottonChecked
        } // lblCourseDescription

        LessonSelector {
            id: lessonSelector
            anchors {
                top: lblCourseDescription.bottom
                right: container.right
                left: container.left
                bottom: container.bottom

                topMargin: 5
            }
            // Fill the column
            height: container.height - courseSelector.height - lblCourseDescription.height

            currentLessonModel: courseModel.selectedLessonModel
            previewTitle: courseModel.selectedLessonModel.selectedLessonTitle
            previewText: courseModel.selectedLessonModel.selectedLessonText

            // React to output signals
            onSelectedLessonIndexChanged: {
                // console.debug("lessonSelector.onLessonSelected: " + index)
                courseModel.selectedLessonModel.selectLesson(
                            selectedLessonIndex)
            }
        } // lessonSelector
    } // container

    Loader {
        id: profileScreenLoader

        anchors {
            top: toolBar.bottom
            right: root.right
            left: root.left
        }

        height: 0
        active: false

        sourceComponent: ProfileScreen {
            id: profileScreen

            height: profileScreenLoader.height
            width: profileScreenLoader.width
        } // profileScreen

        onLoaded: console.debug("profileScreenLoader loaded")
    } // profileScreenLoader

    transitions: [
        Transition {
            to: "PROFILE"
            SequentialAnimation {
                // Ensure profileScreen is loaded
                ScriptAction {
                    script: profileScreenLoader.active = true
                }
                // Dim homeScreen
                PropertyAnimation {
                    target: container
                    property: "opacity"
                    to: 0
                    easing.type: Easing.Linear
                    duration: 150
                }
                // Inflate profileScreen
                PropertyAnimation {
                    target: profileScreenLoader
                    property: "height"
                    to: root.height - toolBar.height
                    easing.type: Easing.Linear
                    duration: 250
                }
            }
        },
        Transition {
            from: "PROFILE"
            SequentialAnimation {
                // Roll profileScreen in
                PropertyAnimation {
                    target: profileScreenLoader
                    property: "height"
                    to: 0
                    easing.type: Easing.Linear
                    duration: 250
                }
                // Show homeScreen
                PropertyAnimation {
                    target: container
                    property: "opacity"
                    to: 1
                    easing.type: Easing.Linear
                    duration: 350
                }
            }
        }
    ] // transitions
} // Item
