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


/*
CourseSelector
Horizontal course chooser sitting at the top of the home screen.
Consists of a PathView and two Buttons to switch between the courses.
*/
FocusScope {
    id: root

    implicitHeight: Math.max(path.height, btPreviousCourse.height)

    // Configuration properties
    // Left right margins
    property real lrMargins: 10
    // Space between the elements
    property real spacer: 8
    // Number of items to be shown in the PathView
    property int itemCount: 1

    // Input property interface
    // Set course model
    property alias courseModel: path.model

    // Output property interface
    // Propagates whether course details should be displayed or not
    property bool courseDescriptionBottonChecked: path.bottonChecked

    // Control selection by keys when focus is disabled (Keys.forwardTo)
    Keys.onLeftPressed: path.decrementCurrentIndex()
    Keys.onRightPressed: path.incrementCurrentIndex()
    Keys.onSpacePressed: path.bottonChecked = !path.bottonChecked

    PathView {
        id: path

        // FIXME: onCurrentIndexChanged fires two times on each index change! Why?
        property int lastIndex: 0
        property bool bottonChecked: false

        anchors {
            top: parent.top
            bottom: parent.bottom

            left: parent.left
            leftMargin: root.lrMargins

            right: btPreviousCourse.left
            rightMargin: root.spacer
        }

        focus: true
        Keys.onLeftPressed: decrementCurrentIndex()
        Keys.onRightPressed: incrementCurrentIndex()

        pathItemCount: root.itemCount

        delegate: Component {
            Item {
                anchors {
                    top: PathView.view.top
                    bottom: PathView.view.bottom
                }

                width: lblCourseName.width + btCourseDescription.width + root.spacer

                // Label that shows the current course name
                Label {
                    id: lblCourseName
                    anchors {
                        verticalCenter: parent.verticalCenter
                    }
                    font.weight: Font.Bold
                    text: cTitle
                }

                // Button to show the course details
                Button {
                    id: btCourseDescription
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: lblCourseName.right
                        leftMargin: root.spacer
                    }
                    width: height
                    iconName: "dialog-information"
                    iconSource: "qrc:/icons/16x16/dialog-information.png"
                    checkable: true
                    checked: path.bottonChecked
                    onCheckedChanged: {
                        if (path.bottonChecked != checked) {
                            path.bottonChecked = checked
                        }
                    }
                } // btCourseDescription
            } // Item
        } // Component

        // Effects
        path: Path {
            startX: (path.currentItem) ? path.currentItem.width / 2 : 0
            PathLine {
                relativeX: (path.currentItem) ? path.currentItem.width * 2 : 0
            }
        }

        // When course is changed, inform the model
        onCurrentIndexChanged: {
            // FIXME: Workaround (see above)
            if (currentIndex != lastIndex) {
                lastIndex = currentIndex
                model.index = currentIndex
            }
        }
    } // path

    Button {
        id: btPreviousCourse
        anchors {
            right: btNextCourse.left
            rightMargin: root.spacer
        }
        iconName: "arrow-left"
        iconSource: "qrc:/icons/16x16/arrow-left.png"
        onClicked: path.decrementCurrentIndex()
    }

    Button {
        id: btNextCourse
        anchors {
            right: parent.right
            rightMargin: root.lrMargins
        }
        iconName: "arrow-right"
        iconSource: "qrc:/icons/16x16/arrow-right.png"
        onClicked: path.incrementCurrentIndex()
    }
}
