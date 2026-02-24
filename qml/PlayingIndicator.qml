// PlayingIndicator.qml
import QtQuick

Item {
    id: root
    property color barColor: "white"
    property bool playing: true
    property real scaleFactor: 1.0

    width: 24 * scaleFactor
    height: 16 * scaleFactor

    Row {
        anchors.centerIn: parent
        spacing: 3 * scaleFactor

        Repeater {
            model: 5
            delegate: Rectangle {
                width: 3 * scaleFactor
                radius: 1.5 * scaleFactor
                color: barColor
                anchors.verticalCenter: parent.verticalCenter

                property int baseHeight: [6, 10, 16, 10, 6][index]
                height: baseHeight * scaleFactor

                SequentialAnimation on height {
                    loops: Animation.Infinite
                    running: root.playing
                    NumberAnimation { to: baseHeight * 2 * scaleFactor; duration: 800; easing.type: Easing.InOutSine }
                    NumberAnimation { to: baseHeight * scaleFactor; duration: 800; easing.type: Easing.InOutSine }
                    PauseAnimation { duration: index * 100 }
                }
            }
        }
    }
}
