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
        front: HomeScreen {
            id: homeScreen

            anchors.fill: parent

            // Switch focus between front and back
            focus: !trainingScreen.focus

            onLessonStarted: flipper.state = "TRAINING"
        } // homeScreen

        back: TrainingScreen {
            id: trainingScreen

            anchors.fill: parent
            // will be enabled in transition and controls focus
            visible: false

            focus: visible

            onVisibleChanged: {
                if (visible)
                    reset()
            }

            title: courseModel.selectedLessonModel.selectedLessonTitle
            text: courseModel.selectedLessonModel.selectedLessonText

            onQuit: flipper.state = ""
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
                        easing.type: Easing.InOutQuad
                        duration: 500
                    }
                    PropertyAction {
                        target: homeScreen
                        property: "visible"
                        value: false
                    }
                }
            },
            Transition {
                from: "TRAINING"
                SequentialAnimation {
                    PropertyAction {
                        target: homeScreen
                        property: "visible"
                        value: true
                    }
                    NumberAnimation {
                        target: rotation
                        property: "angle"
                        easing.type: Easing.InOutQuad
                        duration: 500
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
