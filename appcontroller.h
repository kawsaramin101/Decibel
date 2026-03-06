#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include "database.h"
#include "models.h"
#include <QAudioOutput>
#include <QFileInfo>
#include <QList>
#include <QMediaPlayer>
#include <QObject>
#include <QString>
#include <QVariant>

enum class RepeatMode { None, One, All };
enum class ShuffleMode { Off, On };

class AppController : public QObject {
  Q_OBJECT

  // Properties
  Q_PROPERTY(QList<QObject *> playlists READ playlists NOTIFY playlistsChanged)
  Q_PROPERTY(QList<QObject *> songs READ songs NOTIFY songsChanged)
  Q_PROPERTY(Playlist *selectedPlaylist READ selectedPlaylist NOTIFY
                 selectedPlaylistChanged)
  Q_PROPERTY(int selectedPlaylistId READ selectedPlaylistId NOTIFY
                 selectedPlaylistChanged)
  Q_PROPERTY(QString selectedPlaylistName READ selectedPlaylistName NOTIFY
                 selectedPlaylistChanged)
  Q_PROPERTY(QString currentSongName READ currentSongName NOTIFY
                 currentPlayingSongChanged)
  Q_PROPERTY(Playlist *currentPlayingPlaylist READ currentPlayingPlaylist NOTIFY
                 currentPlayingPlaylistChanged)
  Q_PROPERTY(PlaylistSong *currentPlayingSong READ currentPlayingSong NOTIFY
                 currentPlayingSongChanged)
  Q_PROPERTY(bool isPlaying READ isPlaying NOTIFY playbackStateChanged)
  Q_PROPERTY(qint64 position READ position NOTIFY positionChanged)
  Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)

public:
  explicit AppController(QObject *parent = nullptr);
  ~AppController();

  QList<QObject *> playlists();
  QList<QObject *> songs();
  Playlist *selectedPlaylist();
  int selectedPlaylistId();
  QString selectedPlaylistName();
  QString currentSongName();
  Playlist *currentPlayingPlaylist();
  PlaylistSong *currentPlayingSong();
  bool isPlaying();
  qint64 position();
  qint64 duration();

  QString getFromStorage(const QString &key);
  void setInStorage(const QString &key, const QString &value);

signals:
  void playlistsChanged();
  void songsChanged();
  void selectedPlaylistChanged();
  void currentPlayingSongChanged();
  void currentPlayingPlaylistChanged();
  void playbackStateChanged();
  void positionChanged();
  void durationChanged();

public slots:
  void cleanup();
  void selectPlaylist(Playlist *playlist);
  void playSong(const QString &filepath, int songId = -1);
  void setSongAndPlay(PlaylistSong *song, Playlist *playlist);
  void scanFolder(int playlistId);
  void addSongsFromFilePicker(int playlistId);
  void togglePlayPause();
  void seek(qint64 position);
  void seekBack();
  void seekNext();
  void playPreviousSong();
  void playNextSong();
  void addPlaylist(const QString &name);
  void deletePlaylist(int playlistId);
  void removeSong(int songId);
  void toggleRepeatMode();
  void toggleShuffleMode();

private:
  void loadPlaylists();
  void loadSongs(int playlistId);
  void addSongsToPlaylist(int playlistId, const QList<QFileInfo> &audioFiles);
  void setSongPlayer(const QString &filepath, int songId = -1);
  void onMediaStatusChanged(QMediaPlayer::MediaStatus status);

  // Storage functions
  void saveToLocalStorage(const QString &key, const QVariant &value);
  QVariant getFromLocalStorage(const QString &key,
                               const QVariant &defaultValue = QVariant());
  int getSelectedPlaylist();

  Database m_db;
  QList<Playlist *> m_playlists;
  QList<PlaylistSong *> m_songs;

  Playlist *m_selectedPlaylist;
  int m_selectedPlaylistId;
  QString m_selectedPlaylistName;

  QString m_currentSongName;
  QString m_currentSongPath;

  Playlist *m_currentPlayingPlaylist;
  PlaylistSong *m_currentPlayingSong;

  QMediaPlayer *m_player;
  QAudioOutput *m_audioOutput;

  qint64 m_pendingSeekPosition;
  QString m_appDataFilePath;

  RepeatMode m_repeatMode = RepeatMode::None;
  ShuffleMode m_shuffleMode = ShuffleMode::Off;

  struct StorageField {
    QString key;
    QString value;
  };

  QList<StorageField> m_storage = {
      {"current_playing_song_id", "-1"},
      {"selected_playlist_id", "-1"},
      {"current_playing_song_last_position", "-1"},
      {"current_playing_playlist_id", "-1"},
      {"shuffle", "false"},
      {"repeat_type", "none"},
  };

  void loadLocalStorage();
  void saveLocalStorage();
};

#endif // APPCONTROLLER_H
