import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0

Item {
    id: listItem

    // Text to show on the label
    property string labelText: "Insert text here ..."
    // Opacity of the text
    property real labelOpacity: 1
    // Source of an optional icon to show infront of the text
    property string iconSource

    signal clicked
    signal doubleClicked

    //    implicitHeight: Math.max(label.height, icon.height)
    implicitHeight: label.height + 8

    state: ListView.isCurrentItem ? "selected" : mouseArea.containsMouse ? "hovered" : "normal"

    Item {
        id: content

        // Ensure the content is above the background
        z: background.z + 1

        anchors {
            fill: parent
            topMargin: 1
            bottomMargin: 1
            leftMargin: 4
            rightMargin: 4
        }

        // Icon
        Image {
            id: icon

            anchors {
                left: parent.left
                leftMargin: 4

                verticalCenter: parent.verticalCenter
            }

            visible: !!iconSource
            source: iconSource

            height: label.height
            width: height
        }

        // Text
        Label {
            id: label

            // Wrap text
            maximumLineCount: 1
            elide: Text.ElideRight

            anchors {
                left: icon.visible ? icon.right : parent.left
                right: parent.right
                leftMargin: 4
                rightMargin: 4

                // FIXME!! Margins are anchor specific!
                // They are only used when the anchores are used!!!
                //                topMargin: 10
                //                bottomMargin: 10
                verticalCenter: parent.verticalCenter
            }

            text: labelText
            opacity: labelOpacity
        }
    }

    Rectangle {
        id: background

        anchors {
            fill: parent
            topMargin: content.anchors.topMargin
            bottomMargin: content.anchors.bottomMargin
            leftMargin: content.anchors.leftMargin
            rightMargin: content.anchors.rightMargin
        }

        color: "lightgrey"
//        border.color: "white"
//        border.width: 1
        radius: 3
        opacity: 1
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true

        onClicked: parent.clicked()
        onDoubleClicked: parent.doubleClicked()
    }

    states: [
        State {
            name: "normal"
            PropertyChanges {
                target: background
                visible: false
            }
        },
        State {
            name: "hovered"
            PropertyChanges {
                target: background
                visible: true
                opacity: 0.4
            }
        },
        State {
            name: "selected"
            PropertyChanges {
                target: background
                visible: true
                opacity: 0.8
            }
        }
    ]

    transitions: [
        Transition {
            from: "normal"
            to: "hovered"
            NumberAnimation {
                target: background
                property: "opacity"
                duration: 200
                easing.type: Easing.OutCubic
            }
        },
        Transition {
            from: "hovered"
            to: "normal"
            NumberAnimation {
                target: background
                property: "opacity"
                duration: 200
                easing.type: Easing.OutCubic
            }
        }
    ]
}
