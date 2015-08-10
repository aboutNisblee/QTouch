import QtQuick 2.3
import QtQuick.Controls 1.3

Item {
    id: root

    CourseSelector {
        id: courseSelector
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right

            topMargin: 5
        }

        currentCourseModel: courseModel

        // React to output signals
        onCourseSelected: {
            console.debug("courseSelector.onCourseSelected: " + index)
            courseModel.selectCourse(index)
        }
        onShowCourseDescription: {
            lblCourseDescription.state = (enabled) ? "VISIBLE" : "INVISIBLE"
        }
    }

    // Label that can be inflated by clicking the btCourseDescription button that
    // is placed on the PathView delegate of CourseSelector.
    Label {
        id: lblCourseDescription

        anchors {
            top: courseSelector.bottom
            topMargin: 5
            right: parent.right
            rightMargin: 5
            left: parent.left
            leftMargin: 5

            bottomMargin: 10
        }

        // Access course model at the currently selected index and get the description
        text: courseModel.selectedCourseDescription

        visible: height > 0
        state: "INVISIBLE"

        states: [
            State {
                name: "INVISIBLE"
                PropertyChanges {
                    target: lblCourseDescription
                    height: 0
                    opacity: 0
                }
            },
            State {
                name: "VISIBLE"
                PropertyChanges {
                    target: lblCourseDescription
                    height: contentHeight
                    opacity: 1
                }
            }
        ] // states

        transitions: [
            Transition {
                from: "INVISIBLE"
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
                to: "INVISIBLE"
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

        Behavior on height {
            PropertyAnimation {
                easing.type: Easing.Linear
                duration: 150
            }
        }
        Behavior on opacity {
            PropertyAnimation {
                easing.type: Easing.Linear
                duration: 150
            }
        }
    } // Label

    LessonSelector {
        id: lessonSelector
        anchors {
            top: lblCourseDescription.bottom
            bottom: parent.bottom
            right: parent.right
            left: parent.left

            topMargin: 5
        }
        // Fill the column
        height: parent.height - courseSelector.height - lblCourseDescription.height

        currentLessonModel: courseModel.selectedLessonModel
        previewTitle: courseModel.selectedLessonModel.selectedLessonTitle
        previewText: courseModel.selectedLessonModel.selectedLessonText

        // React to output signals
        onLessonSelected: {
            console.debug("lessonSelector.onLessonSelected: " + index)
            courseModel.selectedLessonModel.selectLesson(index)
        }
    }
} // Item
