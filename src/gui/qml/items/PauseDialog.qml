import QtQuick 2.3
import QtQuick.Controls 1.3

FocusScope {
    id: root

    signal unpause
    signal quit

    visible: false
    enabled: visible
    z: -9999

    function show() {
        background.state = "VISIBLE"
        btContinue.focus = true
    }

    function hide() {
        background.state = ""
    }

    Rectangle {
        id: background
        anchors.fill: parent
        color: "#66000000"

        states: State {
            name: "VISIBLE"
            PropertyChanges {
                target: root
                visible: true
                z: 9999
            }
        } // states

        Rectangle {
            id: border
            width: column.width + 40
            height: column.height + 40
            anchors.centerIn: parent
            color: "#ffffff"
            border.width: 1
            radius: 6

            Column {
                anchors.centerIn: parent
                id: column
                spacing: 10

                Label {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Pause")
                }

                Button {
                    id: btContinue
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Continue")
                    isDefault: true
                    focus: true
                    onClicked: root.unpause()
                    Keys.onReturnPressed: root.unpause()
                    Keys.onEnterPressed: root.unpause()

                    KeyNavigation.down: btCancel
                    KeyNavigation.tab: btCancel
                }

                Button {
                    id: btCancel
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: qsTr("Main menu")
                    onClicked: root.quit()
                    Keys.onReturnPressed: root.quit()
                    Keys.onEnterPressed: root.quit()
                }
            }
        }
    }
}
