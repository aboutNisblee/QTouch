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
import QtQuick.Controls.Styles 1.3
import QtQml.Models 2.1
import QtGraphicalEffects 1.0

import de.nisble.qtouch 1.0
import "items" as Items


/*
LessonSelector
Vertical lesson chooser that fills the most of the home screen,
A ListView is placed on the left half that displays the lessons of the currently selected course.
The lesson text is displayed as preview on the right half.

Note: To keep the CourseModel as simple as possible, each course has is own lesson model.
CourseModel produces it when its needed, by wrapping the lessons of a course in a LessonModel.
A DelegateModel is used here to supply the delagate and LessonModel together to the ListView.
*/
FocusScope {
    id: root

    // Configuration properties
    property real scrollbarHeight: 26
    property real scrollbarWidth: 16

    // Input property interface
    // The lesson model of the currently selected course
    property alias currentLessonModel: delegateModel.model
    // Title for the preview panel
    property alias previewTitle: txtPreview.title
    // Text for the preview panel
    property alias previewText: txtPreview.text

    // Output property interface
    // The index of the currently selected lesson
    property int selectedLessonIndex: 0

    signal lessonStarted

    // Control selection by keys when focus is disabled (Keys.forwardTo)
    Keys.onUpPressed: list.decrementCurrentIndex()
    Keys.onDownPressed: list.incrementCurrentIndex()
    Keys.onReturnPressed: lessonStarted()
    Keys.onEnterPressed: lessonStarted()

    VisualDataModel {
        id: delegateModel

        delegate: Items.ListItem {
            // Get the width from ListView
            anchors {
                left: parent.left
                right: parent.right
                leftMargin: 4
                rightMargin: 4
            }

            text: lTitle
            iconSource: index > 10 ? "qrc:/icons/32x32/object-locked.png" : ""

            hoverable: true
            bgHoveredOpacity: 0.5
            bgMaxOpacity: 0
            bgColor: "lightgray"
            bgRadius: 5

            onClicked: list.currentIndex = index
            onDoubleClicked: lessonStarted()
        } // delegate
    } // delegateModel

    Row {
        id: rowLayout

        anchors.fill: parent
        spacing: 5

        ScrollView {
            anchors {
                top: rowLayout.top
                bottom: rowLayout.bottom
            }
            width: rowLayout.width / 2

            focus: root.focus

            style: ScrollViewStyle {
                // Transient and inc/dec controls or not transient and no controls
                // transientScrollBars: true
                incrementControl: Item {
                }
                decrementControl: Item {
                }

                handle: Item {
                    implicitHeight: root.scrollbarHeight
                    implicitWidth: root.scrollbarWidth
                    Rectangle {
                        anchors {
                            fill: parent
                            margins: 2
                        }
                        color: "lightgray"
                        radius: 3
                    } // Rectangle
                } // handle

                scrollBarBackground: Item {
                    implicitHeight: root.scrollbarHeight
                    implicitWidth: root.scrollbarWidth
                }
            } // style

            ListView {
                id: list

                // Implicitly anchored to ScrollView

                // Settings
                focus: true
                highlightFollowsCurrentItem: false

                Keys.onUpPressed: decrementCurrentIndex()
                Keys.onDownPressed: incrementCurrentIndex()

                // Data
                model: delegateModel

                highlight: Component {
                    Rectangle {
                        width: list.currentItem.width
                        height: list.currentItem.height
                        color: list.currentItem.bgColor
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

                onCurrentIndexChanged: selectedLessonIndex = currentIndex
            } // list
        } //ScrollView

        Item {
            id: preview

            // Size
            anchors {
                top: rowLayout.top
                bottom: rowLayout.bottom
            }
            width: rowLayout.width / 2

            Item {
                // Reserves the space above the button
                id: previewContainer
                anchors {
                    top: preview.top
                    left: preview.left
                    right: preview.right

                    bottom: btStart.top

                    topMargin: 30
                    leftMargin: 30
                    rightMargin: 30
                    bottomMargin: 30
                }

                ScrollView {
                    // Centers in parent and adapts its size to its contents
                    // as long as its fits into its parent
                    id: previewScroller

                    anchors.centerIn: parent
                    width: Math.min(previewBorder.width, previewContainer.width)
                    height: Math.min(previewBorder.height,
                                     previewContainer.height)

                    verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                    horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

                    Rectangle {
                        // Only a border that fits its content and centrs in its parent
                        id: previewBorder

                        anchors.centerIn: parent
                        width: txtPreview.width
                        height: txtPreview.height
                        antialiasing: true
                        border {
                            width: 1
                            color: "#000"
                        }

                        TextPage {
                            id: txtPreview
                            anchors.centerIn: parent

                            textMargin: 20

                            maxWidth: previewContainer.width

                            // Note: title and text are set by root item via property alias
                        } // txtPreview
                    } // previewBorder

                    // FIXME: Fails when opacity is faded out
                    //                    InnerShadow {
                    //                        width: previewBorder.width
                    //                        height: previewBorder.height
                    //                        anchors.centerIn: parent
                    //                        horizontalOffset: -2
                    //                        verticalOffset: 2
                    //                        radius: 0
                    //                        samples: 16
                    //                        color: "black"
                    //                        source: previewBorder
                    //                    }
                } // previewScroller
            } // previewContainer

            Button {
                id: btStart

                text: qsTr("Start Lesson")

                anchors {
                    horizontalCenter: parent.horizontalCenter
                    bottom: parent.bottom
                    bottomMargin: 10
                }

                onClicked: lessonStarted()
            } // btStart
        } // preview
    } // RowLayout
} // root
