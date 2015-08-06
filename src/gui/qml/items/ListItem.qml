import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0


/*
ListView item
*/
Item {
    id: root

    property alias text: label.text
    property alias textFormat: label.textFormat
    // Source of an optional icon to show infront of the text
    property alias iconSource: icon.source
    // If enabled each Item knows three states: UNSELECTED, HOVERED, SELECTED
    // If disabled the HOVERED state is ommited and background properties are
    // statically set to theitem.
    property alias hoverable: mouseArea.hoverEnabled

    property real topMargin: 4
    property real bottomMargin: 4

    property alias bgColor: background.color
    property int bgBorderWidth: 0
    property color bgBorderColor: "black"
    property alias bgRadius: background.radius

    property real bgHoveredOpacity: 0.4
    // Note: If a highlighter item is used, simply set bgMaxOpacity to 0
    property real bgMaxOpacity: 1

    property bool isCurrentItem: false

    signal clicked
    signal doubleClicked

    implicitHeight: label.height + topMargin + bottomMargin

    state: {
        if (hoverable)
            isCurrentItem ? "SELECTED" : mouseArea.containsMouse ? "HOVERED" : "UNSELECTED"
        else
            isCurrentItem ? "SELECTED" : "UNSELECTED"
    }

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
            height: label.font.pixelSize * 1.2
            width: height

            fillMode: Image.PreserveAspectFit
        }

        // Text
        Label {
            id: label

            anchors {
                left: icon.visible ? icon.right : parent.left
                right: parent.right
                leftMargin: 4
                rightMargin: 4
                verticalCenter: parent.verticalCenter
            }

            wrapMode: Text.WordWrap

            // Note: text and textFormat is directly set by root item
        }
    }

    Rectangle {
        id: background

        anchors.fill: parent
        opacity: bgMaxOpacity
        border.width: bgBorderWidth
        border.color: bgBorderColor
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: parent.clicked()
        onDoubleClicked: parent.doubleClicked()
    }

    states: [
        State {
            name: "UNSELECTED"
            PropertyChanges {
                target: background
                visible: (hoverable) ? false : true
            }
        },
        State {
            name: "HOVERED"
            PropertyChanges {
                target: background
                visible: true
                opacity: bgHoveredOpacity
            }
        },
        State {
            name: "SELECTED"
            PropertyChanges {
                target: background
                visible: true
                opacity: bgMaxOpacity
            }
        }
    ]

    transitions: [
        Transition {
            from: "UNSELECTED"
            to: "HOVERED"
            NumberAnimation {
                target: background
                property: "opacity"
                duration: 200
                easing.type: Easing.OutCubic
            }
        },
        Transition {
            from: "HOVERED"
            to: "UNSELECTED"
            NumberAnimation {
                target: background
                property: "opacity"
                duration: 200
                easing.type: Easing.OutCubic
            }
        }
    ]
}
