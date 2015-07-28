import QtQuick 2.3
import QtQuick.Controls 1.3

import "items" as Items

Item {
    id: root

    Row {
        anchors.fill: parent

        ListView {
            id: list

            anchors {
                top: parent.top
                bottom: parent.bottom
                margins: 20
            }
            width: parent.width / 2

            model: ListModel {
                id: testModel
                ListElement {
                    profile: "MrMoe"
                }
                ListElement {
                    profile: "Mone"
                }
                ListElement {
                    profile: "Moritz"
                }
                ListElement {
                    profile: "TestUser1"
                }
                ListElement {
                    profile: "Default"
                }
            }

            delegate: Items.ListItem {
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: 20
                }

                text: profile
                iconSource: "qrc:/icons/64x64/user-identity.png"

                topMargin: 6
                bottomMargin: 6
                hoverable: true
                bgRadius: 5
                bgBorderWidth: 1
                bgBorderColor: "black"
                bgMaxOpacity: 0

                isCurrentItem: ListView.isCurrentItem

                onClicked: {
                    ListView.view.currentIndex = index
                    console.debug("Profile changed!")
                }
            } // delegate

            highlight: Component {
                Rectangle {
                    anchors {
                        left: parent.left
                        right: parent.right
                        margins: 20
                    }

                    height: list.currentItem.height
                    width: list.currentItem.width
                    visible: list.height > 0

                    border.width: list.currentItem.bgBorderWidth
                    border.color: list.currentItem.bgBorderColor
                    radius: list.currentItem.bgRadius

                    x: list.currentItem.x
                    y: list.currentItem.y
                    Behavior on y {
                        SpringAnimation {
                            spring: 10
                            damping: 0.8
                        }
                    }
                }
            } // highlight

            footer: Items.ListItem {
                anchors {
                    left: parent.left
                    right: parent.right
                    margins: 20
                }

                visible: list.height > 0

                topMargin: 12

                text: qsTr("New profile")
                iconSource: "qrc:/icons/32x32/list-add.png"

                onClicked: {
                    console.debug("New Profile!")
                }
            } // footer
        } // profileSelectorView

        Rectangle {
            anchors {
                top: parent.top
                bottom: parent.bottom
            }
            width: parent.width / 2
            color: "beige"
        }
    } // Row
} // root
