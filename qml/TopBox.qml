import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    implicitHeight: 200
    padding: 0

    SplitView {
    anchors.fill: parent
    orientation: Qt.Horizontal

    LeftBox {
        SplitView.preferredWidth: parent.width * 0.35
        SplitView.minimumWidth: 120
    }

    RightBox {
        SplitView.preferredWidth: parent.width * 0.65
        SplitView.minimumWidth: 200
    }
}
}
