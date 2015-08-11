import QtQuick 2.3
import QtQuick.Controls 1.3

Rectangle {
    id: mainWindow
    width: 1000
    height: 700
    color: "#00000000"
    border.width: 0
    visible: true

    Flipable {
        id: flipper

        anchors {
            top: mainWindow.top
            left: mainWindow.left
            right: mainWindow.right
            bottom: statusBar.top
        }

        // HomeScreen and ProfileScreen
        front: Item {
            id: home

            anchors.fill: parent

            states: State {
                name: "PROFILE"
                when: btProfile.checked
            }

            ToolBar {
                id: toolBar
                anchors {
                    top: home.top
                    left: home.left
                    right: home.right
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
                    //                    onCheckedChanged: home.state = (checked) ? "PROFILE" : ""
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
                    top: toolBar.bottom
                    right: home.right
                    left: home.left
                    bottom: home.bottom
                }

                onLessonStarted: flipper.state = "TRAINING"
            } // homeScreen

            Loader {
                id: profileScreenLoader

                anchors {
                    top: toolBar.bottom
                    right: home.right
                    left: home.left
                }

                height: 0
                active: false

                sourceComponent: ProfileScreen {
                    id: profileScreen

                    height: profileScreenLoader.height
                    width: profileScreenLoader.width
                } // profileScreen

                onLoaded: console.debug("profileScreenLoader loaded")
            } // profileScreenLoader

            transitions: [
                Transition {
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
                            to: home.height - toolBar.height
                            easing.type: Easing.Linear
                            duration: 250
                        }
                    }
                },
                Transition {
                    from: "PROFILE"
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
        } // front

        back: TrainingScreen {
            id: trainingScreen

            anchors.fill: parent
            visible: false // will be enabled in transition

            title: courseModel.selectedLessonModel.selectedLessonTitle
            text: courseModel.selectedLessonModel.selectedLessonText

            onCancel: flipper.state = ""
        } // trainingScreen

        transform: Rotation {
            id: rotation
            origin.x: flipper.width / 2
            origin.y: flipper.height / 2
            axis.x: 1
            axis.y: 0
            axis.z: 0 // set axis.y to 1 to rotate around y-axis
            angle: 0 // the default angle
        } // rotation

        states: State {
            name: "TRAINING"
            PropertyChanges {
                target: rotation
                angle: 180
            }
        }

        transitions: [
            Transition {
                to: "TRAINING"
                SequentialAnimation {
                    PropertyAction {
                        target: trainingScreen
                        property: "visible"
                        value: true
                    }
                    NumberAnimation {
                        target: rotation
                        property: "angle"
                        duration: 1000
                    }
                    PropertyAction {
                        target: home
                        property: "visible"
                        value: false
                    }
                }
            },
            Transition {
                from: "TRAINING"
                SequentialAnimation {
                    PropertyAction {
                        target: home
                        property: "visible"
                        value: true
                    }
                    NumberAnimation {
                        target: rotation
                        property: "angle"
                        duration: 1000
                    }
                    PropertyAction {
                        target: trainingScreen
                        property: "visible"
                        value: false
                    }
                }
            }
        ] // flipper transitions
    } // flipper

    StatusBar {
        id: statusBar
        anchors {
            right: mainWindow.right
            left: mainWindow.left
            bottom: mainWindow.bottom
        }

        Row {
            anchors.fill: parent
            Label {
                text: "Read Only"
            }
        }
    } // statusBar
} // mainWindow
