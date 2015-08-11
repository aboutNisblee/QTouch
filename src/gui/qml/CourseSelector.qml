import QtQuick 2.3
import QtQuick.Controls 1.3


/*
CourseSelector
Horizontal course chooser sitting at the top of the home screen.
Consists of a PathView and two Buttons to switch between the courses.
*/
Item {
    id: root

    implicitHeight: Math.max(path.height, btPreviousCourse.height)

    // Configuration properties
    // Left right margins
    property real lrMargins: 10
    // Space between the elements
    property real spacer: 8
    // Number of to be shown in the PathView
    property int itemCount: 1

    // Input property interface
    property variant currentCourseModel

    // Output property interface
    property int selectedCourseIndex: 0
    // Propagate whether course details should be displayed or not
    property bool courseDescriptionBottonChecked: false

    PathView {
        id: path

        // FIXME: onCurrentIndexChanged fires two times on each index change! Why?
        property int lastIndex: 0
        property bool bottonChecked: false

        // Size
        anchors {
            top: parent.top
            bottom: parent.bottom

            left: parent.left
            leftMargin: root.lrMargins

            right: btPreviousCourse.left
            rightMargin: root.spacer
        }

        // Settings
        focus: true
        Keys.onLeftPressed: decrementCurrentIndex()
        Keys.onRightPressed: incrementCurrentIndex()

        pathItemCount: root.itemCount

        // Data
        model: currentCourseModel

        delegate: Component {
            Item {
                anchors {
                    top: PathView.view.top
                    bottom: PathView.view.bottom
                }

                width: lblCourseName.width + btCourseDescription.width + root.spacer

                // Label that shows the current course name
                Label {
                    id: lblCourseName
                    anchors {
                        verticalCenter: parent.verticalCenter
                    }
                    font.weight: Font.Bold
                    text: cTitle
                }

                // Button to show the course details
                Button {
                    id: btCourseDescription
                    anchors {
                        verticalCenter: parent.verticalCenter
                        left: lblCourseName.right
                        leftMargin: root.spacer
                    }
                    iconName: "dialog-information"
                    iconSource: "qrc:/icons/64x64/dialog-information.png"
                    checkable: true
                    checked: path.bottonChecked
                    onCheckedChanged: {
                        if (path.bottonChecked != checked) {
                            path.bottonChecked = checked
                            courseDescriptionBottonChecked = checked
                        }
                    }
                }
            } // Item
        } // Component

        // Effects
        path: Path {
            startX: (path.currentItem) ? path.currentItem.width / 2 : 0
            PathLine {
                relativeX: (path.currentItem) ? path.currentItem.width * 2 : 0
            }
        }

        // When course is changed, inform the model
        onCurrentIndexChanged: {
            // FIXME: Workaround (see above)
            if (currentIndex != lastIndex) {
                lastIndex = currentIndex
                selectedCourseIndex = currentIndex
            }
        }
    } // path

    Button {
        id: btPreviousCourse
        anchors {
            right: btNextCourse.left
            rightMargin: root.spacer
        }
        iconName: "arrow-left"
        iconSource: "qrc:/icons/32x32/arrow-left.png"
        onClicked: path.decrementCurrentIndex()
    }

    Button {
        id: btNextCourse
        anchors {
            right: parent.right
            rightMargin: root.lrMargins
        }
        iconName: "arrow-right"
        iconSource: "qrc:/icons/32x32/arrow-right.png"
        onClicked: path.incrementCurrentIndex()
    }
}
