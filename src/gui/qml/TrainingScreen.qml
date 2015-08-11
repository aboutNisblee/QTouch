import QtQuick 2.3
import QtQuick.Controls 1.3
import QtGraphicalEffects 1.0

import de.nisble.qtouch 1.0

Item {
    id: root

    // Title
    property alias title: trainingWidget.title
    // Text
    property alias text: trainingWidget.text

    signal cancel

    MouseArea {
        anchors.fill: root
        onDoubleClicked: root.cancel()
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
            height: 30

            color: "red"
        }

        Item {
            id: widgetContainer

            anchors {
                left: columnLayout.left
                right: columnLayout.right
            }

            height: root.height - statsPlaceholder.height

            // height: widgetScroller.height
            ScrollView {
                // Centers in parent and adapts its size to its contents
                // as long as its fits into its parent
                id: widgetScroller

                anchors.centerIn: parent
                width: Math.min(widgetBorder.width, widgetContainer.width)
                height: Math.min(widgetBorder.height, widgetContainer.height)

                verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
                horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

                Rectangle {
                    // Only a border that fits its content and centrs in its parent
                    id: widgetBorder

                    anchors.centerIn: parent
                    width: trainingWidget.width
                    height: trainingWidget.height
                    antialiasing: true
                    border {
                        width: 1
                        color: "#000"
                    }

                    TrainingWidget {
                        id: trainingWidget
                        anchors.centerIn: parent

                        autoWrap: false
                        textMargin: 20

                        // Because the text defines the width of
                        // the whole item a manimum is needed for the
                        // layout to know where to wrap the text.
                        //                            maxWidth: widgetContainer.width

                        // Note: title and text are set by root item via property alias
                    } // txtPreview
                } // previewBorder

                InnerShadow {
                    width: widgetBorder.width
                    height: widgetBorder.height
                    anchors.centerIn: parent
                    horizontalOffset: -2
                    verticalOffset: 2
                    radius: 0
                    samples: 16
                    color: "black"
                    source: widgetBorder
                }
            } // widgetScroller
        } // widgetContainer
    } // Column
} // root
