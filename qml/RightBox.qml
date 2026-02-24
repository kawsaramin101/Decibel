import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Pane {
    padding: 0

    ColumnLayout {
        anchors.fill: parent

        // Header: Search + Buttons
        RowLayout {
            Layout.fillWidth: true
            Layout.margins: 8
            visible: backend.selectedPlaylistId >= 0

            TextField {
                Layout.fillWidth: true
                placeholderText: "Search songs..."
                visible: backend.songs.length > 0
                Layout.preferredWidth: visible ? -1 : 0
            }

            Button {
                text: "Search"
                visible: backend.songs.length > 0
            }

            Item {
                Layout.fillWidth: backend.songs.length === 0
            }

            Button {
                text: backend.songs.length === 0 ? "Add songs" : ""
                icon.source: Qt.styleHints.colorScheme === Qt.Dark
                        ? "qrc:/assets/icons/add-songs-white.svg"
                        : "qrc:/assets/icons/add-songs-black.svg"
                icon.color: palette.buttonText
                onClicked: backend.addSongsFromFilePicker(backend.selectedPlaylistId)
                ToolTip.visible: hovered && backend.songs.length > 0
                ToolTip.text: "Add songs"
                ToolTip.delay: 500
            }

            Button {
                text: backend.songs.length === 0 ? "Scan Folder" : ""
                icon.source: "qrc:/assets/icons/scan-folder.svg"
                icon.color: palette.buttonText
                onClicked: backend.scanFolder(backend.selectedPlaylistId)
                ToolTip.visible: hovered && backend.songs.length > 0
                ToolTip.text: "Scan Folder"
                ToolTip.delay: 500
            }

            Item {
                Layout.fillWidth: backend.songs.length === 0
            }
        }

        // Song List or Empty State
        StackLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: backend.selectedPlaylistId < 0 ? 0 : (backend.songs.length === 0 ? 1 : 2)

            // No playlist selected
            Item {
                Label {
                    anchors.centerIn: parent
                    text: "Select a playlist to view songs"
                    opacity: 0.4
                    font.pixelSize: 14
                }
            }

            // Playlist selected but empty
            Item {
                Label {
                    anchors.centerIn: parent
                    text: "No songs in this playlist"
                    opacity: 0.4
                    font.pixelSize: 14
                }
            }

            // Song list
            ListView {
                clip: true
                model: backend.songs

                delegate: Item {
                    width: ListView.view.width
                    height: 60

                    MouseArea {
                        id: songMouseArea
                        anchors.fill: parent
                        hoverEnabled: true

                        onClicked: {
                            backend.setSongAndPlay(modelData, backend.selectedPlaylist);
                        }

                        Rectangle {
                            anchors.fill: parent
                            color: songMouseArea.containsMouse ? Qt.rgba(1, 1, 1, 0.05) : "transparent"

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 16
                                anchors.rightMargin: 8
                                spacing: 12

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2

                                    Label {
                                        text: {
                                            const path = modelData.filepath;
                                            return path.substring(path.lastIndexOf('/') + 1);
                                        }
                                        font.pixelSize: 14
                                        Layout.fillWidth: true
                                        elide: Text.ElideRight
                                    }

                                    Label {
                                        text: modelData.formattedDuration || "00:00:00"
                                        font.pixelSize: 12
                                        opacity: 0.6
                                        Layout.fillWidth: true
                                    }
                                }

                                PlayingIndicator {
                                    visible: backend.currentPlayingSong ? modelData.id === backend.currentPlayingSong.id : false
                                    barColor: palette.text
                                    playing: backend.isPlaying
                                    scaleFactor: 0.6
                                    Layout.alignment: Qt.AlignVCenter
                                }

                                ToolButton {
                                    text: "⋮"
                                    font.pixelSize: 18
                                    opacity: songMouseArea.containsMouse ? 1.0 : 0.0
                                    onClicked: songMenu.popup()

                                    Menu {
                                        id: songMenu
                                        MenuItem {
                                            text: "Remove from playlist"
                                            onTriggered: backend.removeSong(modelData.id)
                                        }
                                    }
                                }
                            }
                        }
                    }

                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        anchors.right: parent.right
                        height: 1
                        color: palette.mid
                    }
                }
            }
        }
    }
}
