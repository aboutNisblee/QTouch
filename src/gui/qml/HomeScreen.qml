import QtQuick 2.3
import QtQuick.Controls 1.3

Item {
    id: root

    signal lessonStarted

    Component.onCompleted: {
        lessonSelector.onLessonStarted.connect(lessonStarted)
    }

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
            // console.debug("courseSelector.onCourseSelected: " + index)
            courseModel.selectCourse(index)
        }
        onShowCourseDescription: {
            lblCourseDescription.state = (enabled) ? "VISIBLE" : ""
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

        height: 0
        opacity: 0

        states: State {
            name: "VISIBLE"
            PropertyChanges {
                target: lblCourseDescription
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
            // console.debug("lessonSelector.onLessonSelected: " + index)
            courseModel.selectedLessonModel.selectLesson(index)
        }

        //        onLessonStarted: {
        //            console.log("Starting Lesson: " + courseModel.selectedLessonModel.selectedLessonTitle
        //                        + " of Course: " + courseModel.selectedCourseTitle)
        //        }
    }
} // Item
