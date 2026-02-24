import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

Pane {
    padding: 0

    ColumnLayout {
        anchors.fill: parent

        // Add Playlist Button
        RowLayout {
            Layout.fillWidth: true

            Label {
                text: "Playlists"
                font.pixelSize: 16
                padding: 16
            }


            Item {
                Layout.fillWidth: true
            }

            Button {
                text: "+"
                Layout.preferredWidth: implicitHeight
                onClicked: addPlaylistWindow.show()
            }
            Item { width: 8 }
        }


        // Playlist List
        ListView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            // spacing: 8
            clip: true

            model: backend.playlists

            delegate: Item {
                width: ListView.view.width
                height: 56

                MouseArea {
                    id: delegateMouseArea
                    anchors.fill: parent
                    hoverEnabled: true

                    onClicked: {
                        backend.selectPlaylist(modelData);
                    }

                    Rectangle {
                        anchors.fill: parent
                        color: backend.selectedPlaylistId === modelData.id
                               ? Qt.rgba(1, 1, 1, 0.12)        // active/selected color
                               : delegateMouseArea.containsMouse
                                 ? Qt.rgba(1, 1, 1, 0.05)      // hover color
                                 : "transparent"
                        Rectangle {
                            anchors.fill: parent
                            color: backend.selectedPlaylistId === modelData.id
                                   ? Qt.rgba(1, 1, 1, 0.12)        // active/selected color
                                   : delegateMouseArea.containsMouse
                                     ? Qt.rgba(1, 1, 1, 0.05)      // hover color
                                     : "transparent"

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 16
                                anchors.rightMargin: 8
                                // spacing: 12

                                ColumnLayout {
                                    Layout.fillWidth: true
                                    // spacing: 2

                                    Label {
                                        text: modelData.name
                                        font.pixelSize: 15
                                        Layout.fillWidth: true
                                        elide: Text.ElideRight
                                    }

                                    Label {
                                        text: modelData.songCount + " song" + (modelData.songCount > 1 ? "s" : "")
                                        font.pixelSize: 13
                                        opacity: 0.5
                                        Layout.fillWidth: true
                                    }
                                }


                                PlayingIndicator {
                                    visible: backend.currentPlayingPlaylist ? modelData.id === backend.currentPlayingPlaylist.id : false
                                    barColor: palette.text
                                    playing: backend.isPlaying
                                    scaleFactor: 0.6
                                    Layout.alignment: Qt.AlignVCenter
                                }

                                ToolButton {
                                    text: "⋮"
                                    opacity: delegateMouseArea.containsMouse ? 1.0 : 0.0
                                    // Remove visible property, keep it always in layout

                                    onClicked: contextMenu.popup()

                                    Menu {
                                        id: contextMenu
                                        MenuItem {
                                            text: "Delete Playlist"
                                            onTriggered: backend.deletePlaylist(modelData.id)
                                        }
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

            // Empty state
            Item {
                anchors.fill: parent
                visible: backend.playlists.length === 0

                Label {
                    anchors.centerIn: parent
                    text: "No playlists yet"
                    opacity: 0.4
                    font.pixelSize: 14
                }
            }
        }
    }


    Window {
        id: addPlaylistWindow
        title: "New Playlist"
        width: 400
        height: 150
        minimumWidth: 300
        minimumHeight: 150
        modality: Qt.ApplicationModal
        flags: Qt.Dialog
        color: palette.window

        onClosing: {
            playlistNameField.text = "";
        }

        Component.onCompleted: {

            x = parent.Window.window.x + (parent.Window.window.width - width) / 2
            y = parent.Window.window.y + (parent.Window.window.height - height) / 2
        }

        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 16
            spacing: 16

            Label {
                text: "Enter playlist name:"
                font.pixelSize: 14
            }

            TextField {
                id: playlistNameField
                Layout.fillWidth: true
                placeholderText: "Playlist name"
                focus: true

                Keys.onReturnPressed: {
                    if (text.trim() !== "") {
                        backend.addPlaylist(text);
                        text = "";
                        addPlaylistWindow.close();
                    }
                }

                Keys.onEscapePressed: {
                    text = "";
                    addPlaylistWindow.close();
                }
            }

            Item {
                Layout.fillHeight: true
            }

            RowLayout {
                Layout.alignment: Qt.AlignRight
                spacing: 8

                Button {
                    text: "Cancel"
                    onClicked: {
                        playlistNameField.text = "";
                        addPlaylistWindow.close();
                    }
                }

                Button {
                    text: "OK"
                    highlighted: true
                    enabled: playlistNameField.text.trim() !== ""
                    onClicked: {
                        if (playlistNameField.text.trim() !== "") {
                            backend.addPlaylist(playlistNameField.text);
                            playlistNameField.text = "";
                            addPlaylistWindow.close();
                        }
                    }
                }
            }
        }
    }
}
