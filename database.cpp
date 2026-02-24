#include "database.h"
#include <QDebug>
#include <QDir>
#include <QSqlError>
#include <QSqlQuery>
#include <QStandardPaths>
#include <QVariant>

Database::Database(QObject *parent) : QObject(parent) {
  QString dbPath = getDatabasePath();

  m_db = QSqlDatabase::addDatabase("QSQLITE");
  m_db.setDatabaseName(dbPath);

  if (!m_db.open()) {
    qWarning() << "Failed to open database:" << m_db.lastError().text();
    return;
  }

  // Enable foreign keys
  QSqlQuery query(m_db);
  query.exec("PRAGMA foreign_keys = ON");

  initSchema();
}

Database::~Database() {
  // Clean up all cached objects
  for (auto &songList : m_playlistSongCache) {
    qDeleteAll(songList);
  }
  m_playlistSongCache.clear();

  if (m_db.isOpen()) {
    m_db.close();
  }
}

QString Database::getDatabasePath() {
  QString appDataPath =
      QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  QDir appDataDir(appDataPath);

  if (!appDataDir.exists()) {
    appDataDir.mkpath(".");
  }

  return appDataPath + "/app.db";
}

void Database::initSchema() {
  QSqlQuery query(m_db);

  QString table1 = R"(
        CREATE TABLE IF NOT EXISTS playlist (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            "order" INTEGER NOT NULL
        );
        
    )";

  if (!query.exec(table1)) {
    qWarning() << "Failed to initialize schema:" << query.lastError().text();
  }

  QString table2 = R"(
        CREATE TABLE IF NOT EXISTS playlistsong (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            filepath TEXT NOT NULL,
            "order" INTEGER NOT NULL,
            formatted_duration TEXT,
            playlist_id INTEGER NOT NULL,
            FOREIGN KEY (playlist_id)
                REFERENCES playlist (id)
                ON DELETE CASCADE
        );
    )";

  if (!query.exec(table2)) {
    qWarning() << "Failed to initialize schema:" << query.lastError().text();
  }
}

int Database::createPlaylist(const QString &name) {
  QSqlQuery query(m_db);

  // Get next order number
  query.prepare("SELECT COALESCE(MAX(\"order\"), 0) + 1 FROM playlist");
  if (!query.exec() || !query.next()) {
    qWarning() << "Failed to get next order:" << query.lastError().text();
    return -1;
  }
  int nextOrder = query.value(0).toInt();

  // Insert new playlist
  query.prepare("INSERT INTO playlist (name, \"order\") VALUES (?, ?)");
  query.addBindValue(name);
  query.addBindValue(nextOrder);

  if (!query.exec()) {
    qWarning() << "Failed to create playlist:" << query.lastError().text();
    return -1;
  }

  return query.lastInsertId().toInt();
}

Playlist *Database::getPlaylist(int playlistId, QObject *parent) {
  QSqlQuery query(m_db);
  query.prepare(R"(
        SELECT p.id, p.name, p."order", COUNT(ps.id) as song_count
        FROM playlist p
        LEFT JOIN playlistsong ps ON p.id = ps.playlist_id
        WHERE p.id = ?
        GROUP BY p.id
    )");
  query.addBindValue(playlistId);

  if (!query.exec()) {
    qWarning() << "Failed to get playlist:" << query.lastError().text();
    return nullptr;
  }

  if (query.next()) {
    return new Playlist(query.value("id").toInt(),
                        query.value("name").toString(),
                        query.value("order").toInt(),
                        query.value("song_count").toInt(), parent);
  }

  return nullptr;
}

QList<Playlist *> Database::getPlaylists(QObject *parent) {
  QList<Playlist *> playlists;

  QSqlQuery query(m_db);
  query.prepare(R"(
        SELECT p.id, p.name, p."order", COUNT(ps.id) as song_count
        FROM playlist p
        LEFT JOIN playlistsong ps ON p.id = ps.playlist_id
        GROUP BY p.id
        ORDER BY p."order" DESC
    )");

  if (!query.exec()) {
    qWarning() << "Failed to get playlists:" << query.lastError().text();
    return playlists;
  }

  while (query.next()) {
    playlists.append(new Playlist(query.value("id").toInt(),
                                  query.value("name").toString(),
                                  query.value("order").toInt(),
                                  query.value("song_count").toInt(), parent));
  }

  return playlists;
}

void Database::deletePlaylist(int playlistId) {
  QSqlQuery query(m_db);
  query.prepare("DELETE FROM playlist WHERE id = ?");
  query.addBindValue(playlistId);

  if (!query.exec()) {
    qWarning() << "Failed to delete playlist:" << query.lastError().text();
    return;
  }

  // Clean up cached songs if they exist
  if (m_playlistSongCache.contains(playlistId)) {
    qDeleteAll(m_playlistSongCache[playlistId]);
    m_playlistSongCache.remove(playlistId);
  }
}

void Database::addSong(int playlistId, const QString &filepath,
                       const QString &formattedDuration) {
  QSqlQuery query(m_db);

  // Get next order number for this playlist
  query.prepare("SELECT COALESCE(MAX(\"order\"), 0) + 1 FROM playlistsong "
                "WHERE playlist_id = ?");
  query.addBindValue(playlistId);

  if (!query.exec() || !query.next()) {
    qWarning() << "Failed to get next song order:" << query.lastError().text();
    return;
  }
  int nextOrder = query.value(0).toInt();

  // Insert song
  query.prepare("INSERT INTO playlistsong (filepath, \"order\", playlist_id, "
                "formatted_duration) VALUES (?, ?, ?, ?)");
  query.addBindValue(filepath);
  query.addBindValue(nextOrder);
  query.addBindValue(playlistId);
  query.addBindValue(formattedDuration.isEmpty() ? QVariant(QVariant::String)
                                                 : formattedDuration);

  if (!query.exec()) {
    qWarning() << "Failed to add song:" << query.lastError().text();
    return;
  }

  // Invalidate cache for this playlist
  if (m_playlistSongCache.contains(playlistId)) {
    qDeleteAll(m_playlistSongCache[playlistId]);
    m_playlistSongCache.remove(playlistId);
  }
}

PlaylistSong *Database::getSong(int songId, QObject *parent) {
  QSqlQuery query(m_db);
  query.prepare("SELECT filepath, formatted_duration, playlist_id, \"order\" "
                "FROM playlistsong WHERE id = ?");
  query.addBindValue(songId);

  if (!query.exec()) {
    qWarning() << "Failed to get song:" << query.lastError().text();
    return nullptr;
  }

  if (query.next()) {
    return new PlaylistSong(songId, query.value("filepath").toString(),
                            query.value("formatted_duration").toString(),
                            query.value("playlist_id").toInt(),
                            query.value("order").toInt(), parent);
  }

  return nullptr;
}

QList<PlaylistSong *> Database::getSongs(int playlistId, QObject *parent) {
  // Return cached songs if present
  if (m_playlistSongCache.contains(playlistId)) {
    qDebug() << "Using cached songs";
    return m_playlistSongCache[playlistId];
  }

  qDebug() << "Fetching songs from DB";

  QList<PlaylistSong *> songs;
  QSqlQuery query(m_db);
  query.prepare(R"(
        SELECT id, filepath, formatted_duration, "order"
        FROM playlistsong
        WHERE playlist_id = ?
        ORDER BY "order" DESC
    )");
  query.addBindValue(playlistId);

  if (!query.exec()) {
    qWarning() << "Failed to get songs:" << query.lastError().text();
    return songs;
  }

  while (query.next()) {
    songs.append(new PlaylistSong(
        query.value("id").toInt(), query.value("filepath").toString(),
        query.value("formatted_duration").toString(), playlistId,
        query.value("order").toInt(), parent));
  }

  // Cache the results
  m_playlistSongCache[playlistId] = songs;

  return songs;
}

PlaylistSong *Database::getNextSong(int playlistId, int currentSongId,
                                    QObject *parent) {
  QList<PlaylistSong *> songs = getSongs(playlistId);

  int currentIndex = -1;
  for (int i = 0; i < songs.size(); ++i) {
    if (songs[i]->id() == currentSongId) {
      currentIndex = i;
      break;
    }
  }

  if (currentIndex != -1 && currentIndex + 1 < songs.size()) {
    PlaylistSong *nextSong = songs[currentIndex + 1];
    // Create a new instance with the specified parent
    return new PlaylistSong(nextSong->id(), nextSong->filepath(),
                            nextSong->formattedDuration(),
                            nextSong->playlistId(), nextSong->order(), parent);
  }

  return nullptr;
}

PlaylistSong *Database::getPreviousSong(int playlistId, int currentSongId,
                                        QObject *parent) {
  QList<PlaylistSong *> songs = getSongs(playlistId);

  int currentIndex = -1;
  for (int i = 0; i < songs.size(); ++i) {
    if (songs[i]->id() == currentSongId) {
      currentIndex = i;
      break;
    }
  }

  // Note: There's a bug in the Python code - it checks "current_index - 1 <
  // len(songs)" which is almost always true. The correct check should be
  // "current_index - 1 >= 0" I've fixed it here, but if you want the exact same
  // behavior as Python, use: if (currentIndex != -1 && currentIndex - 1 <
  // songs.size()) {

  if (currentIndex != -1 && currentIndex - 1 >= 0) {
    PlaylistSong *prevSong = songs[currentIndex - 1];
    // Create a new instance with the specified parent
    return new PlaylistSong(prevSong->id(), prevSong->filepath(),
                            prevSong->formattedDuration(),
                            prevSong->playlistId(), prevSong->order(), parent);
  }

  return nullptr;
}

void Database::removeSong(int songId) {
  QSqlQuery query(m_db);

  // Get playlist_id before deleting
  query.prepare("SELECT playlist_id FROM playlistsong WHERE id = ?");
  query.addBindValue(songId);

  int playlistId = -1;
  if (query.exec() && query.next()) {
    playlistId = query.value("playlist_id").toInt();
  }

  // Delete the song
  query.prepare("DELETE FROM playlistsong WHERE id = ?");
  query.addBindValue(songId);

  if (!query.exec()) {
    qWarning() << "Failed to remove song:" << query.lastError().text();
    return;
  }

  // Invalidate cache
  if (playlistId != -1 && m_playlistSongCache.contains(playlistId)) {
    qDeleteAll(m_playlistSongCache[playlistId]);
    m_playlistSongCache.remove(playlistId);
  }
}