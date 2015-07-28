import QtQuick 2.3
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3
import QtQml.Models 2.1

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
Item {
    id: root

    // Configuration properties
    property real scrollbarHeight: 26
    property real scrollbarWidth: 16

    // Input property interface
    // The lesson model of the currently selected course
    property alias currentLessonModel: delegateModel.model
    // Text for the preview panel
    property string previewText

    signal // Output signal interface
    lessonSelected(int index)

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

            onClicked: {
                // Selection
                list.currentIndex = index
            }
            onDoubleClicked: {
                // TODO
                console.log("delegateModel.onDoubleClicked: StartLesson")
            }
        } // delegate
    } // delegateModel

    Row {
        anchors.fill: parent

        ScrollView {
            anchors {
                top: parent.top
                bottom: parent.bottom
            }
            width: parent.width / 2

            focus: true

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

                // Size redundant: ListView is implicitly anchored

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

                onCurrentIndexChanged: {
                    lessonSelected(currentIndex)
                }
            } // list
        } //ScrollView

        Item {
            id: preview

            // Size
            anchors {
                top: parent.top
                bottom: parent.bottom
            }
            width: parent.width / 2

            // Children
            TextArea {
                id: txtPreview
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    bottom: btStart.top
                    bottomMargin: 10
                }
                backgroundVisible: true
                frameVisible: true

                text: root.previewText
            } // txtLessonPreview

            Button {
                id: btStart

                text: qsTr("Start Lesson")

                anchors {
                    horizontalCenter: parent.horizontalCenter
                    bottom: parent.bottom
                    bottomMargin: 10
                }

                onClicked: {
                    // TODO
                    console.log("btLessonStart.onClicked: StartLesson")
                }
            } // btLessonStart
        } // lessonPreview
    } // RowLayout
} // root
