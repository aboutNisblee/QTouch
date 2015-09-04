import QtQuick 2.3
import QtQuick.Controls 1.3
import QtGraphicalEffects 1.0

import de.nisble.qtouch 1.0
import "items" as Items

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
            height: 200

            Items.AnalogStopWatch {
                id: analogStopWatch
                anchors {
                    left: parent.left
                    top: parent.top
                    bottom: progressIndicator.top

                    margins: 10
                }
                width: height
            }

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

                flickableItem.boundsBehavior: Flickable.StopAtBounds

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
                            width: 1
                            color: "black"
                        }

                        TrainingWidget {
                            id: trainingWidget

                            focus: true

                            // A minimum width is needed for the widget to
                            // be able to scale the font size
                            minWidth: widgetContainer.width - 2 * horizontalSheetMargin
                            maxWidth: widgetContainer.width - 2 * horizontalSheetMargin

                            textMargin: 25

                            //                            docClipRect: Qt.rect(
                            //                                          0,
                            //                                          widgetScroller.flickableItem.contentY
                            //                                          - verticalSheetMargin,
                            //                                          widgetScroller.viewport.childrenRect.width,
                            //                                          widgetScroller.viewport.childrenRect.height)

                            // Note: title and text are set by root item via property alias
                            onEscape: root.quit()

                            onCursorRectangleChanged: {
                                // When cursor is not fully visible, scroll the flickable.
                                if (cursorRectangle.y < (widgetScroller.flickableItem.contentY
                                                         - verticalSheetMargin)
                                        || (cursorRectangle.y + cursorRectangle.height
                                            > (widgetScroller.flickableItem.contentY
                                               - verticalSheetMargin
                                               + widgetScroller.viewport.childrenRect.height))) {
                                    scrollAnimation.to = cursorRectangle.y
                                    scrollAnimation.start()
                                }

                                //                                if (cursorRectangle.y < (widgetScroller.flickableItem.contentY
                                //                                                         - verticalSheetMargin)) {
                                //                                    // Scroll down
                                //                                    scrollAnimation.to = cursorRectangle.y
                                //                                    scrollAnimation.start()
                                //                                } else if (cursorRectangle.y + cursorRectangle.height
                                //                                           > (widgetScroller.flickableItem.contentY
                                //                                              - verticalSheetMargin
                                //                                              + widgetScroller.viewport.childrenRect.height)) {
                                //                                    // Scroll up
                                //                                    scrollAnimation.to = cursorRectangle.y + cursorRectangle.height
                                //                                            - widgetScroller.viewport.childrenRect.height + verticalSheetMargin
                                //                                    scrollAnimation.start()
                                //                                }
                            }

                            onCursorPositionChanged: {
                                if (0 == cursorPosition
                                        && 0 == activeLineNumber) {
                                    scrollAnimation.to = 0
                                    scrollAnimation.start()
                                }
                            }
                        } // trainingWidget
                    } // widgetBorder

                    DropShadow {
                        anchors.centerIn: parent
                        width: widgetBorder.width
                        height: widgetBorder.height
                        horizontalOffset: 3
                        verticalOffset: -3
                        radius: 1
                        samples: 2
                        color: widgetBorder.border.color
                        source: widgetBorder
                    }
                } // widgetBackground
            } // widgetScroller
        } // widgetContainer
    } // Column

    NumberAnimation {
        id: scrollAnimation
        target: widgetScroller.flickableItem
        duration: 350
        property: "contentY"
        easing.type: Easing.InOutSine
    }
} // root
