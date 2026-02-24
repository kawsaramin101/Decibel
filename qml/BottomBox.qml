import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Pane {
    padding: 16

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Label {
            text: {
                return backend.currentSongName ? backend.currentSongName.substring(backend.currentSongName.lastIndexOf('/') + 1) : "No song playing";
            }
            font.pixelSize: 14
            font.bold: backend.currentSongName !== ""
            opacity: backend.currentSongName ? 1.0 : 0.5
            Layout.fillWidth: true
            elide: Text.ElideRight
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Label {
                text: formatTime(backend.position)
                font.pixelSize: 12
                opacity: 0.6
                Layout.preferredWidth: 45
            }

            Slider {
                id: progressSlider
                Layout.fillWidth: true
                from: 0
                to: backend.duration
                value: backend.position

                onMoved: {
                    backend.seek(value);
                }
            }

            Label {
                text: formatTime(backend.duration)
                font.pixelSize: 12
                opacity: 0.6
                Layout.preferredWidth: 45
            }
        }

        // Play/Pause button
        RowLayout {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            
            Button {
                icon.source: "qrc:/assets/icons/play-previous.svg"
                icon.width: 22
                icon.height: 22

                flat: true
                Layout.preferredWidth: implicitHeight
                enabled: backend.currentSongName !== ""
                onClicked: backend.playPreviousSong()
            }
            Button {
                icon.source: "qrc:/assets/icons/seek-backward.svg"
                icon.width: 22
                icon.height: 22

                flat: true
                Layout.preferredWidth: implicitHeight
                enabled: backend.currentSongName !== ""
                onClicked: backend.seekBack()
            }
            Button {
                icon.source: backend.isPlaying ? "qrc:/assets/icons/pause.svg" : "qrc:/assets/icons/play.svg"
                icon.width: 22
                icon.height: 22
                icon.color: palette.buttonText  // ADD THIS
                flat: true
                Layout.preferredWidth: implicitHeight
                enabled: backend.currentSongName !== ""
                onClicked: backend.togglePlayPause()
            }
            Button {
                icon.source: "qrc:/assets/icons/seek-forward.svg"
                icon.width: 22
                icon.height: 22
                icon.color: palette.buttonText  // ADD THIS
                flat: true
                Layout.preferredWidth: implicitHeight
                enabled: backend.currentSongName !== ""
                onClicked: backend.seekNext()
            }
            Button {
                icon.source: "qrc:/assets/icons/play-next.svg"
                icon.width: 22
                icon.height: 22
                icon.color: palette.buttonText  // ADD THIS
                flat: true
                Layout.preferredWidth: implicitHeight
                enabled: backend.currentSongName !== ""
                onClicked: backend.playNextSong()
            }
        }
    }

    function formatTime(milliseconds) {
        let seconds = Math.floor(milliseconds / 1000);
        const minutes = Math.floor(seconds / 60);
        seconds = seconds % 60;
        return minutes + ":" + (seconds < 10 ? "0" : "") + seconds;
    }
}
