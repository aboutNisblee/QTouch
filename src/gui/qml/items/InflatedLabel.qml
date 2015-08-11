import QtQuick 2.3
import QtQuick.Controls 1.2

Item {
    id: root

    property string text
    property bool inflated: false

    height: label.height
    width: label.width

    Label {
        id: label

        text: root.text

        height: 0
        opacity: 0

        states: State {
            name: "VISIBLE"
            when: inflated
            PropertyChanges {
                target: label
                height: contentHeight
                opacity: 1
            }
        } // states

        transitions: [
            Transition {
                to: "VISIBLE"
                SequentialAnimation {
                    PropertyAnimation {
                        property: "height"
                        easing.type: Easing.Linear
                        duration: 150
                    }
                    PropertyAnimation {
                        property: "opacity"
                        easing.type: Easing.Linear
                        duration: 150
                    }
                }
            },
            Transition {
                from: "VISIBLE"
                SequentialAnimation {
                    PropertyAnimation {
                        property: "opacity"
                        easing.type: Easing.Linear
                        duration: 150
                    }
                    PropertyAnimation {
                        property: "height"
                        easing.type: Easing.Linear
                        duration: 150
                    }
                }
            }
        ] // transitions
    } // Label
} //root
