import QtQuick 2.3
import QtQuick.Controls 1.3
import QtGraphicalEffects 1.0

import de.nisble.qtouch 1.0

FocusScope {
    id: root

    // Configuration properties
    property int horizontalSheetMargin: 60
    property int verticalSheetMargin: 60

    // Input property interface
    // Lesson title
    property alias title: trainingWidget.title
    // Lesson text
    property alias text: trainingWidget.text

    signal quit

    function reset() {
        trainingWidget.reset()
    }

    Column {
        id: columnLayout

        anchors.fill: root

        Rectangle {
            id: statsPlaceholder
            anchors {
                left: columnLayout.left
                right: columnLayout.right
            }

            // XXX: DEBUGGING
            height: 150

            color: "red"

            ProgressBar {
                id: progressIndicator
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
                value: trainingWidget.progress
            }
        }

        Item {
            id: widgetContainer

            anchors {
                left: columnLayout.left
                right: columnLayout.right
            }
            height: root.height - statsPlaceholder.height

            ScrollView {
                id: widgetScroller

                anchors.fill: parent

                verticalScrollBarPolicy: Qt.ScrollBarAsNeeded
                horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

                focus: true
                clip: true

                Rectangle {
                    id: widgetBackground

                    anchors.centerIn: parent
                    width: widgetBorder.width + 2 * horizontalSheetMargin
                    height: widgetBorder.height + 2 * verticalSheetMargin

                    color: "lightgray"

                    // Only a border that fits its content and centers in its parent
                    Rectangle {
                        id: widgetBorder

                        anchors.centerIn: parent
                        width: trainingWidget.width
                        height: trainingWidget.height

                        border {
                            width: 2
                            color: "black"
                        }

                        TrainingWidget {
                            id: trainingWidget

                            focus: true

                            // A minimum width is needed for the widget to
                            // be able to scale the font size
                            minWidth: widgetContainer.width - 2 * horizontalSheetMargin

                            textMargin: 25

                            //                            viewport: Qt.rect(
                            //                                          0,
                            //                                          widgetScroller.flickableItem.contentY
                            //                                          - verticalSheetMargin,
                            //                                          widgetScroller.viewport.childrenRect.width,
                            //                                          widgetScroller.viewport.childrenRect.height)

                            // Note: title and text are set by root item via property alias
                            onEscape: root.quit()
                        } // trainingWidget
                    } // widgetBorder
                } // widgetBackground

                //                InnerShadow {
                //                    width: widgetBorder.width
                //                    height: widgetBorder.height
                //                    anchors.centerIn: parent
                //                    horizontalOffset: -2
                //                    verticalOffset: 2
                //                    radius: 0
                //                    samples: 16
                //                    color: "black"
                //                    source: widgetBorder
                //                }
            } // widgetScroller
        } // widgetContainer
    } // Column
} // root
