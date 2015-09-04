import QtQuick 2.3

/*
Analog stopwatch
TODO: Add the possibility to bypass the internal timer
and trigger the clock from outside.
 */
Item {
    id: root

    // Configuration
    property bool bgVisible: false
    property int sourceWidth: 512
    property int sourceHeight: 512

    // Timer properties
    property alias interval: timer.interval
    property alias repeat: timer.repeat
    property bool running: timer.running

    // Output
    property int seconds: timer.seconds
    signal fire(int milliseconds)

    // Control
    function start() {
        if (!timer.running) {
            timer.millisecs = 0
            timer.seconds = 0
            secondhand.rotation = 0
            minutehand.rotation = 0
            timer.start()
        }
    }
    function stop() {
        timer.stop()
    }
    function resume() {
        timer.start()
    }

    Timer {
        id: timer
        interval: 1000
        repeat: true
        running: false
        property int millisecs: 0
        property int seconds: 0
        onTriggered: {
            millisecs += interval

            // Only fire on custom interval (else onSecondsChanged can be used)
            if (interval != 1000)
                fire(millisecs)

            if (0 === (millisecs % 1000)) {
                seconds++
                secondhand.rotation += 6
                if (0 === (seconds % 60)) {
                    minutehand.rotation += 6
                }
            }
        }
    }

    Image {
        id: background
        anchors.fill: parent
        sourceSize.width: root.sourceWidth
        sourceSize.height: root.sourceHeight
        fillMode: Image.PreserveAspectFit
        visible: root.bgVisible
        source: "image://svgelement/elapsedtimemeter.svgz#analog-center"
    }

    Image {
        id: dial
        anchors.fill: parent
        sourceSize.width: root.sourceWidth
        sourceSize.height: root.sourceHeight
        fillMode: Image.PreserveAspectFit
        source: "image://svgelement/elapsedtimemeter.svgz#background"
    }

    Image {
        id: minutehand
        anchors.centerIn: dial
        sourceSize.width: root.sourceWidth
        sourceSize.height: root.sourceHeight
        height: dial.height * 0.9
        width: dial.width * 0.9
        fillMode: Image.PreserveAspectFit
        source: "image://svgelement/elapsedtimemeter.svgz#minute-hand"
        rotation: 0

        Behavior on rotation {
            NumberAnimation {
                easing.type: Easing.InQuad
                duration: 1000
            }
        }
    }

    Image {
        id: secondhand
        anchors.centerIn: dial
        sourceSize.width: root.sourceWidth
        sourceSize.height: root.sourceHeight
        height: dial.height * 0.9
        width: dial.width * 0.9
        fillMode: Image.PreserveAspectFit
        source: "image://svgelement/elapsedtimemeter.svgz#second-hand"
        rotation: 0

        Behavior on rotation {
            NumberAnimation {
                easing.type: Easing.InQuad
                duration: 1000
            }
        }
    }
}
