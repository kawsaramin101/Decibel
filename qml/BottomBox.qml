import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt5Compat.GraphicalEffects

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
        Item {
            Layout.fillWidth: true
            height: leftButtons.height

            Row {
                id: leftButtons
                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                Button {
                    icon.source: backend.repeatMode === 0 ? themedIcon("repeat-none") : backend.repeatMode === 1 ? themedIcon("repeat-one") : themedIcon("repeat") 
                       icon.width: 22
                       icon.height: 22
                
                    width: height
                    flat: true
                    onClicked: backend.toggleRepeatMode()
                }

                Button {
                   text: "S"
                   width: height
                   flat: true
                   onClicked: backend.toggleShuffleMode()
                }
            }


            Row {
                   anchors.centerIn: parent
                   Button {
                       icon.source: themedIcon("play-previous")
                       icon.width: 22
                       icon.height: 22

                       flat: true
                       Layout.preferredWidth: implicitHeight
                       enabled: backend.currentSongName !== ""
                       onClicked: backend.playPreviousSong()
                   }
                   Button {
                       icon.source: themedIcon("seek-backward")
                       icon.width: 22
                       icon.height: 22

                       flat: true
                       Layout.preferredWidth: implicitHeight
                       enabled: backend.currentSongName !== ""
                       onClicked: backend.seekBack()
                   }
                   Button {
                       icon.source: backend.isPlaying ? themedIcon("pause") : themedIcon("play")
                       icon.width: 22
                       icon.height: 22
                       icon.color: palette.buttonText  // ADD THIS
                       flat: true
                       Layout.preferredWidth: implicitHeight
                       enabled: backend.currentSongName !== ""
                       onClicked: backend.togglePlayPause()
                   }
                   Button {
                       icon.source: themedIcon("seek-forward")
                       icon.width: 22
                       icon.height: 22
                       icon.color: palette.text
                       flat: true
                       Layout.preferredWidth: implicitHeight
                       enabled: backend.currentSongName !== ""
                       onClicked: backend.seekNext()


                   }
                   Button {
                        icon.source: themedIcon("play-next")
                       icon.width: 22
                       icon.height: 22
                       icon.color: palette.text
                       flat: true
                       Layout.preferredWidth: implicitHeight
                       enabled: backend.currentSongName !== ""
                       onClicked: backend.playNextSong()
                   }

                   }


            Row {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter
                    Button {
                        text: "Settings"
                    }
                }


        }
    }

    function formatTime(milliseconds) {
        let seconds = Math.floor(milliseconds / 1000);
        const minutes = Math.floor(seconds / 60);
        seconds = seconds % 60;
        return minutes + ":" + (seconds < 10 ? "0" : "") + seconds;
    }

    function themedIcon(name) {
        if (Qt.styleHints.colorScheme === Qt.Dark) {
            return "qrc:/assets/icons/" + name + "-white.svg"
        } else {
            return "qrc:/assets/icons/" + name + "-black.svg"
        }
    }
}
