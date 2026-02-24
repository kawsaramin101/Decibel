#ifndef DATABASE_H
#define DATABASE_H

#include "models.h"
#include <QList>
#include <QMap>
#include <QObject>
#include <QSqlDatabase>
#include <QString>

class Database : public QObject {
  Q_OBJECT

public:
  explicit Database(QObject *parent = nullptr);
  ~Database();

  // Playlist operations
  int createPlaylist(const QString &name);
  Playlist *getPlaylist(int playlistId, QObject *parent = nullptr);
  QList<Playlist *> getPlaylists(QObject *parent = nullptr);
  void deletePlaylist(int playlistId);

  // Song operations
  void addSong(int playlistId, const QString &filepath,
               const QString &formattedDuration = QString());
  PlaylistSong *getSong(int songId, QObject *parent = nullptr);
  QList<PlaylistSong *> getSongs(int playlistId, QObject *parent = nullptr);
  PlaylistSong *getNextSong(int playlistId, int currentSongId,
                            QObject *parent = nullptr);
  PlaylistSong *getPreviousSong(int playlistId, int currentSongId,
                                QObject *parent = nullptr);
  void removeSong(int songId);

private:
  void initSchema();
  QString getDatabasePath();

  QSqlDatabase m_db;
  // playlist_id -> list[PlaylistSong*]
  QMap<int, QList<PlaylistSong *>> m_playlistSongCache;
};

#endif // DATABASE_H