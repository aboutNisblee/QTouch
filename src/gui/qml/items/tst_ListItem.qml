import QtQuick 2.3
import QtTest 1.0

TestCase {
    name: "ListItemTests"

    property string testText: "TestText"
    property string testIconSource: "qrc:/icons/64x64/user-identity.png"

    ListItem {
        id: uut

        text: testText
        iconSource: testIconSource
    }

    function test_props() {
        compare(uut.text, testText)
        compare(uut.iconSource, testIconSource)
    }
}
