import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0
import QtQuick.Controls.Styles 1.2

import de.nisble.qtouch 1.0

ColumnLayout {
    id: homeScreen

    // This is not nice, because now the lifetime of the controller depends
    // on the view. But its even better than injecting the controller as property.
    HomeScreenController {
        id: controller
    }

    visible: controller.visible

    // Bind to the signals of the Controller
    Component.onCompleted: {
        console.log("HomeScreen loaded")

        controller.onLoaded()
    }

    ToolBar {
        id: toolBar

        // Note: ToolBar does not like the injection of the Layout properties.
        // "QML ToolBar: Binding loop detected for property "width" when parent made visible.
        anchors {
            right: parent.right
            left: parent.left
        }

        RowLayout {
            // Note: ToolBar has no Layout
            anchors.right: parent.right
            anchors.left: parent.left

            ToolButton {
                id: btUser

                iconName: "user-identity"
                iconSource: "qrc:/icons/64x64/user-identity.png"

                //                onClicked: {
                //                    textAreaLeft.text = "btUser clicked"
                //                }
            }

            // Spacer
            Item {
                Layout.fillWidth: true
            }

            ToolButton {
                id: btConfigure

                Layout.alignment: Qt.AlignRight

                iconName: "configure"
                iconSource: "qrc:/icons/64x64/configure.png"

                onClicked: {
                    lessonPreview.text = "btConfigure clicked"

                    lessonSelector.update()
                }
            }
        }
    }

    RowLayout {
        id: courseSelector
        Layout.fillWidth: true

        Label {
            id: textCourseName
            font.weight: Font.Bold
            text: qsTr("Deutsch")
        }

        Button {
            id: btCourseInfo
            iconName: "dialog-information"
            iconSource: "qrc:/icons/64x64/dialog-information.png"
            checkable: true
        }

        Item {
            Layout.fillWidth: true
        }

        Button {
            id: btPreviousCourse
            Layout.alignment: Qt.AlignRight
            iconName: "arrow-left"
            iconSource: "qrc:/icons/32x32/arrow-left.png"
        }

        Button {
            id: btNextCourse
            Layout.alignment: Qt.AlignRight
            iconName: "arrow-right"
            iconSource: "qrc:/icons/32x32/arrow-right.png"
        }
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 6

        ScrollView {
            id: lessonSelector

            Layout.fillHeight: true
            Layout.fillWidth: true

            ListView {
                id: lessonSelectorView

                Layout.fillHeight: true
                Layout.fillWidth: true

                clip: true
            }
        }
        TextArea {
            id: lessonPreview

            Layout.fillHeight: true
            Layout.fillWidth: true

            enabled: false
            text: "Right one"
        }
    }

    // TODO: Add it to MainWindow!
    //    StatusBar {

    //    }
}
