#include "appcontroller.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QStandardPaths>
#include <QTextStream>
#include <QUrl>
#include <qobject.h>
#include <string>

AppController::AppController(QObject *parent)
    : QObject(parent), m_selectedPlaylist(nullptr), m_selectedPlaylistId(-1),
      m_currentPlayingPlaylist(nullptr), m_currentPlayingSong(nullptr),
      m_player(new QMediaPlayer(this)), m_audioOutput(nullptr),
      m_pendingSeekPosition(-1) {
  // Set up app data file path
  QString appDataPath =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir appDataDir(appDataPath);
  if (!appDataDir.exists()) {
    appDataDir.mkpath(".");
  }
  m_appDataFilePath = appDataPath + "/data.txt";

  // Create the file if it doesn't exist
  // QFile file(m_appDataFilePath);
  // if (!file.exists()) {
  //   file.open(QIODevice::WriteOnly);
  //   file.close();
  // }

  // Connect player signals
  connect(m_player, &QMediaPlayer::positionChanged, this,
          &AppController::positionChanged);
  connect(m_player, &QMediaPlayer::durationChanged, this,
          &AppController::durationChanged);
  connect(m_player, &QMediaPlayer::playbackStateChanged, this,
          &AppController::playbackStateChanged);
  connect(m_player, &QMediaPlayer::mediaStatusChanged, this,
          &AppController::onMediaStatusChanged);

  loadPlaylists();

  loadLocalStorage();

  // Get selected playlist from local storage and set it
  QString selectedPlaylistIdStr = getFromStorage("selected_playlist_id");

  int selectedPlaylistId = std::stoi(selectedPlaylistIdStr.toStdString());
  if (selectedPlaylistId != -1) {
    Playlist *playlist = m_db.getPlaylist(selectedPlaylistId, this);
    if (playlist) {
      selectPlaylist(playlist);
    }
  }

  // Get currently playing song from local storage and set it

  QString currentPlayingSongIdStr = getFromStorage("current_playing_song_id");
  int currentPlayingSongId = std::stoi(currentPlayingSongIdStr.toStdString());
  PlaylistSong *song = m_db.getSong(currentPlayingSongId, this);
  if (song) {
    setSongPlayer(song->filepath(), song->id());

    // Store the position to seek to once media loads
    qint64 lastPosition =
        getFromLocalStorage("current_playing_song_last_position", 0)
            .toLongLong();
    if (lastPosition > 0) {
      m_pendingSeekPosition = lastPosition;
    }
  }
}

AppController::~AppController() { cleanup(); }

void AppController::cleanup() {
  // Save current state
  saveToLocalStorage("current_playing_song_last_position",
                     m_player->position());

  // Stop playback
  m_player->stop();
}

void AppController::loadPlaylists() {
  // Clean up old playlists
  qDeleteAll(m_playlists);
  m_playlists.clear();

  m_playlists = m_db.getPlaylists(this);
  emit playlistsChanged();
}

void AppController::loadSongs(int playlistId) {
  // Don't delete songs here as they're cached by the database
  m_songs = m_db.getSongs(playlistId);
  emit songsChanged();
}

QList<QObject *> AppController::playlists() {
  QList<QObject *> result;
  for (Playlist *p : m_playlists) {
    result.append(p);
  }
  return result;
}

QList<QObject *> AppController::songs() {
  QList<QObject *> result;
  for (PlaylistSong *s : m_songs) {
    result.append(s);
  }
  return result;
}

Playlist *AppController::selectedPlaylist() { return m_selectedPlaylist; }

int AppController::selectedPlaylistId() {
  return m_selectedPlaylist ? m_selectedPlaylist->id() : -1;
}

QString AppController::selectedPlaylistName() {
  return m_selectedPlaylist ? m_selectedPlaylist->name() : "";
}

QString AppController::currentSongName() { return m_currentSongName; }

Playlist *AppController::currentPlayingPlaylist() {
  return m_currentPlayingPlaylist;
}

PlaylistSong *AppController::currentPlayingSong() {
  return m_currentPlayingSong;
}

bool AppController::isPlaying() {
  if (!m_player) {
    return false;
  }
  return m_player->playbackState() == QMediaPlayer::PlayingState;
}

qint64 AppController::position() {
  if (!m_player) {
    return 0;
  }
  return m_player->position();
}

qint64 AppController::duration() {
  if (!m_player) {
    return 0;
  }
  return m_player->duration();
}

void AppController::selectPlaylist(Playlist *playlist) {
  if (!playlist)
    return;

  m_selectedPlaylistId = playlist->id();
  m_selectedPlaylistName = playlist->name();
  qDebug() << playlist->id();

  loadSongs(playlist->id());
  m_selectedPlaylist = playlist;

  emit selectedPlaylistChanged();
  setInStorage("selected_playlist_id", QString::number(playlist->id()));
  // saveToLocalStorage("selected_playlist_id", playlist->id());
}

void AppController::setSongPlayer(const QString &filepath, int songId) {
  if (!m_audioOutput) {
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
  }

  m_currentSongName = filepath;
  m_player->setSource(QUrl::fromLocalFile(filepath));
}

void AppController::playSong(const QString &filepath, int songId) {
  m_player->play();

  emit currentPlayingSongChanged();
  qDebug() << songId;
  saveToLocalStorage("current_playing_song_id", songId);
}

void AppController::setSongAndPlay(PlaylistSong *song, Playlist *playlist) {
  if (!song || !playlist)
    return;

  setSongPlayer(song->filepath(), song->id());
  playSong(song->filepath(), song->id());

  m_currentPlayingPlaylist = playlist;
  m_currentPlayingSong = song;

  emit currentPlayingSongChanged();
  emit currentPlayingPlaylistChanged();

  qDebug() << m_currentPlayingPlaylist;
}

void AppController::addSongsToPlaylist(int playlistId,
                                       const QList<QFileInfo> &audioFiles) {
  QMediaPlayer *tempPlayer = new QMediaPlayer();
  QAudioOutput *tempAudio = new QAudioOutput();
  tempPlayer->setAudioOutput(tempAudio);

  for (const QFileInfo &audioFile : audioFiles) {
    tempPlayer->stop();
    tempPlayer->setSource(QUrl()); // fully release previous
    QCoreApplication::processEvents();

    tempPlayer->setSource(QUrl::fromLocalFile(audioFile.absoluteFilePath()));

    // Add a timeout to avoid infinite loop
    int timeout = 0;
    while (tempPlayer->mediaStatus() != QMediaPlayer::LoadedMedia &&
           tempPlayer->mediaStatus() != QMediaPlayer::BufferedMedia &&
           tempPlayer->mediaStatus() != QMediaPlayer::InvalidMedia &&
           timeout < 5000) {
      QCoreApplication::processEvents();
      timeout++;
    }

    qint64 durationMs = tempPlayer->duration();
    qint64 totalSeconds = durationMs / 1000;
    qint64 hours = totalSeconds / 3600;
    qint64 minutes = (totalSeconds % 3600) / 60;
    qint64 seconds = totalSeconds % 60;
    QString formattedDuration = QString("%1:%2:%3")
                                    .arg(hours, 2, 10, QChar('0'))
                                    .arg(minutes, 2, 10, QChar('0'))
                                    .arg(seconds, 2, 10, QChar('0'));
    m_db.addSong(playlistId, audioFile.absoluteFilePath(), formattedDuration);
  }

  tempPlayer->stop();
  tempPlayer->setSource(QUrl());
  delete tempAudio;
  delete tempPlayer;
}

void AppController::scanFolder(int playlistId) {
  QString folderPath =
      QFileDialog::getExistingDirectory(nullptr, "Select Folder to Scan", "");
  if (folderPath.isEmpty()) {
    return;
  }

  QStringList audioExtensions = {".mp3", ".wav", ".flac", ".m4a",
                                 ".aac", ".ogg", ".wma",  ".opus"};
  QDir folder(folderPath);
  QFileInfoList fileList = folder.entryInfoList(QDir::Files);
  QList<QFileInfo> audioFiles;

  for (const QFileInfo &fileInfo : fileList) {
    if (audioExtensions.contains(fileInfo.suffix().toLower().prepend("."))) {
      audioFiles.append(fileInfo);
    }
  }

  std::sort(audioFiles.begin(), audioFiles.end(),
            [](const QFileInfo &a, const QFileInfo &b) {
              return a.fileName() < b.fileName();
            });

  addSongsToPlaylist(playlistId, audioFiles);

  if (m_selectedPlaylistId == playlistId) {
    loadSongs(playlistId);
  }
  loadPlaylists();
}

void AppController::addSongsFromFilePicker(int playlistId) {
  QStringList audioExtensions = {".mp3", ".wav", ".flac", ".m4a",
                                 ".aac", ".ogg", ".wma",  ".opus"};
  // Build a filter string for the dialog, e.g. "Audio Files (*.mp3 *.wav ...)"
  QStringList filters;
  for (const QString &ext : audioExtensions) {
    filters << "*" + ext;
  }
  QString filterString = "Audio Files (" + filters.join(" ") + ")";

  QStringList filePaths = QFileDialog::getOpenFileNames(
      nullptr, "Select Audio Files", "", filterString);

  if (filePaths.isEmpty()) {
    return;
  }

  QList<QFileInfo> audioFiles;
  for (const QString &path : filePaths) {
    audioFiles.append(QFileInfo(path));
  }

  addSongsToPlaylist(playlistId, audioFiles);

  if (m_selectedPlaylistId == playlistId) {
    loadSongs(playlistId);
  }
  loadPlaylists();
}

void AppController::togglePlayPause() {
  if (m_player->playbackState() == QMediaPlayer::PlayingState) {
    m_player->pause();
  } else {
    m_player->play();
  }
}

void AppController::seek(qint64 position) { m_player->setPosition(position); }

void AppController::seekBack() {
  m_player->setPosition(m_player->position() - 5000);
  emit positionChanged();
}

void AppController::seekNext() {
  m_player->setPosition(m_player->position() + 5000);
  emit positionChanged();
}

void AppController::playPreviousSong() {
  if (!m_selectedPlaylist || !m_currentPlayingSong)
    return;

  PlaylistSong *previousSong = m_db.getPreviousSong(
      m_selectedPlaylist->id(), m_currentPlayingSong->id(), this);

  if (previousSong) {
    setSongAndPlay(previousSong, m_selectedPlaylist);
  }
}

void AppController::playNextSong() {
  if (!m_selectedPlaylist || !m_currentPlayingSong)
    return;

  PlaylistSong *nextSong = m_db.getNextSong(m_selectedPlaylist->id(),
                                            m_currentPlayingSong->id(), this);

  if (nextSong) {
    setSongAndPlay(nextSong, m_selectedPlaylist);
  }
}

void AppController::addPlaylist(const QString &name) {
  if (name.trimmed().isEmpty()) {
    return;
  }

  int newPlaylistId = m_db.createPlaylist(name);
  Playlist *newPlaylist = m_db.getPlaylist(newPlaylistId, this);

  if (newPlaylist) {
    selectPlaylist(newPlaylist);
  }

  loadPlaylists();
}

void AppController::deletePlaylist(int playlistId) {
  if (playlistId == m_selectedPlaylistId) {
    m_selectedPlaylistId = -1;
    m_selectedPlaylistName = "";
    m_selectedPlaylist = nullptr;
    m_songs.clear();
    emit selectedPlaylistChanged();
    emit songsChanged();
  }

  m_db.deletePlaylist(playlistId);
  loadPlaylists();
}

void AppController::removeSong(int songId) {
  m_db.removeSong(songId);

  if (m_selectedPlaylistId != -1) {
    loadSongs(m_selectedPlaylistId);
    loadPlaylists(); // Update song count
  }
}

void AppController::toggleRepeatMode() {
  if (m_repeatMode == RepeatMode::None)
    m_repeatMode = RepeatMode::All;
  else if (m_repeatMode == RepeatMode::All)
    m_repeatMode = RepeatMode::One;
  else
    m_repeatMode = RepeatMode::None;

  QString value = m_repeatMode == RepeatMode::None  ? "none"
                  : m_repeatMode == RepeatMode::All ? "all"
                                                    : "one";
  saveToLocalStorage("repeat_type", value);
}

void AppController::toggleShuffleMode() {
  m_shuffleMode =
      m_shuffleMode == ShuffleMode::Off ? ShuffleMode::On : ShuffleMode::Off;
  saveToLocalStorage("shuffle",
                     m_shuffleMode == ShuffleMode::On ? "true" : "false");
}

void AppController::onMediaStatusChanged(QMediaPlayer::MediaStatus status) {
  if (status == QMediaPlayer::LoadedMedia) {
    if (m_pendingSeekPosition >= 0) {
      qDebug() << "Seeking to" << m_pendingSeekPosition;
      m_player->setPosition(m_pendingSeekPosition);
      m_pendingSeekPosition = -1;
    }
  }

  if (status == QMediaPlayer::EndOfMedia) {
    playNextSong();
  }
}

void AppController::saveToLocalStorage(const QString &key,
                                       const QVariant &value) {
  QFile file(m_appDataFilePath);

  QStringList lines;
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream in(&file);
    while (!in.atEnd()) {
      lines.append(in.readLine());
    }
    file.close();
  }

  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QTextStream out(&file);
    bool found = false;

    for (const QString &line : lines) {
      if (line.trimmed().startsWith(key + " ")) {
        out << key << " = " << value.toString() << "\n";
        found = true;
      } else {
        out << line << "\n";
      }
    }

    if (!found) {
      out << key << " = " << value.toString() << "\n";
    }

    file.close();
  }
}

QVariant AppController::getFromLocalStorage(const QString &key,
                                            const QVariant &defaultValue) {
  QFile file(m_appDataFilePath);

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return defaultValue;
  }

  QTextStream in(&file);
  while (!in.atEnd()) {
    QString line = in.readLine().trimmed();
    if (line.startsWith(key + " ")) {
      QStringList parts = line.split("=");
      if (parts.size() >= 2) {
        file.close();
        return parts[1].trimmed();
      }
    }
  }

  file.close();
  return defaultValue;
}

int AppController::getSelectedPlaylist() {
  return getFromLocalStorage("selected_playlist_id", -1).toInt();
}

void AppController::loadLocalStorage() {
  QFile file(m_appDataFilePath);
  if (!file.exists()) {
    saveLocalStorage();
    return;
  }
  QMap<QString, QString> fileData;
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return;
  QTextStream in(&file);
  while (!in.atEnd()) {
    QString line = in.readLine().trimmed();
    int eqIndex = line.indexOf('=');
    if (eqIndex != -1) {
      fileData[line.left(eqIndex).trimmed()] = line.mid(eqIndex + 1).trimmed();
    }
  }
  file.close();
  for (auto &field : m_storage) {
    if (fileData.contains(field.key))
      field.value = fileData[field.key];
  }
}

void AppController::saveLocalStorage() {
  QFile file(m_appDataFilePath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return;
  QTextStream out(&file);
  for (const auto &field : m_storage)
    out << field.key << " = " << field.value << "\n";
  file.close();
}

QString AppController::getFromStorage(const QString &key) {
  for (const auto &field : m_storage)
    if (field.key == key)
      return field.value;
  return {};
}

void AppController::setInStorage(const QString &key, const QString &value) {
  for (auto &field : m_storage) {
    if (field.key == key) {
      field.value = value;
      saveLocalStorage();
      return;
    }
  }
}
