import QtQuick 2.3
import QtQuick.Controls 1.3

Rectangle {
    id: mainWindow
    width: 1000
    height: 700
    color: "#00000000"
    border.width: 0
    visible: true

    state: "HOME"

    states: [
        State {
            name: "HOME"
        },
        State {
            name: "PROFILE"
        }
    ] // states

    ToolBar {
        id: toolBar
        anchors {
            top: parent.top
            right: parent.right
            left: parent.left
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
            onCheckedChanged: {
                mainWindow.state = (checked) ? "PROFILE" : "HOME"
            }
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

    HomeScreen {
        id: homeScreen

        anchors {
            right: parent.right
            left: parent.left
            top: toolBar.bottom
            bottom: statusBar.top
        }
    } // homeScreen

    Loader {
        id: profileScreenLoader

        anchors {
            right: parent.right
            left: parent.left
            top: toolBar.bottom
        }

        height: 0
        active: false

        sourceComponent: ProfileScreen {
            id: profileScreen

            height: profileScreenLoader.height
            width: profileScreenLoader.width
        } // profileScreen

        onLoaded: {
            console.debug("profileScreenLoader loaded")
        }
    } // profileScreenLoader

    StatusBar {
        id: statusBar
        anchors {
            right: parent.right
            left: parent.left
            bottom: parent.bottom
        }

        Row {
            anchors.fill: parent
            Label {
                text: "Read Only"
            }
        }
    } // statusBar

    transitions: [
        Transition {
            from: "HOME"
            to: "PROFILE"
            SequentialAnimation {
                // Ensure profileScreen is loaded
                ScriptAction {
                    script: profileScreenLoader.active = true
                }

                // Dim homeScreen
                PropertyAnimation {
                    target: homeScreen
                    property: "opacity"
                    to: 0
                    easing.type: Easing.Linear
                    duration: 150
                }
                // Inflate profileScreen
                PropertyAnimation {
                    target: profileScreenLoader
                    property: "height"
                    to: mainWindow.height - toolBar.height - statusBar.height
                    easing.type: Easing.Linear
                    duration: 250
                }
            }
        },
        Transition {
            from: "PROFILE"
            to: "HOME"
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
                    target: homeScreen
                    property: "opacity"
                    to: 1
                    easing.type: Easing.Linear
                    duration: 350
                }
            }
        }
    ] // transitions
} // mainWindow
