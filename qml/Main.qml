import QtQuick
import QtQuick.Controls

Window {
    width: 750
    height: 500
    visible: true
    title: "Decibel"

    SplitView {
        anchors.fill: parent
        orientation: Qt.Vertical

        TopBox {
            SplitView.preferredHeight: 350
            SplitView.minimumHeight: 100
        }

        BottomBox {
            SplitView.preferredHeight: 150
            SplitView.minimumHeight: 80
        }
    }
}
