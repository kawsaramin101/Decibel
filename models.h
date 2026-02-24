#ifndef MODELS_H
#define MODELS_H

#include <QObject>
#include <QString>

class Playlist : public QObject {
  Q_OBJECT
  Q_PROPERTY(int id READ id CONSTANT)
  Q_PROPERTY(QString name READ name CONSTANT)
  Q_PROPERTY(int order READ order CONSTANT)
  Q_PROPERTY(int songCount READ songCount CONSTANT)

public:
  explicit Playlist(int id = 0, const QString &name = "", int order = 0,
                    int songCount = 0, QObject *parent = nullptr)
      : QObject(parent), m_id(id), m_name(name), m_order(order),
        m_songCount(songCount) {}

  int id() const { return m_id; }
  QString name() const { return m_name; }
  int order() const { return m_order; }
  int songCount() const { return m_songCount; }

private:
  int m_id;
  QString m_name;
  int m_order;
  int m_songCount;
};

class PlaylistSong : public QObject {
  Q_OBJECT
  Q_PROPERTY(int id READ id CONSTANT)
  Q_PROPERTY(QString filepath READ filepath CONSTANT)
  Q_PROPERTY(QString formattedDuration READ formattedDuration CONSTANT)
  Q_PROPERTY(int playlistId READ playlistId CONSTANT)
  Q_PROPERTY(int order READ order CONSTANT)

public:
  explicit PlaylistSong(int id = 0, const QString &filepath = "",
                        const QString &formattedDuration = "",
                        int playlistId = 0, int order = 0,
                        QObject *parent = nullptr)
      : QObject(parent), m_id(id), m_filepath(filepath),
        m_formattedDuration(formattedDuration), m_playlistId(playlistId),
        m_order(order) {}

  int id() const { return m_id; }
  QString filepath() const { return m_filepath; }
  QString formattedDuration() const { return m_formattedDuration; }
  int playlistId() const { return m_playlistId; }
  int order() const { return m_order; }

private:
  int m_id;
  QString m_filepath;
  QString m_formattedDuration;
  int m_playlistId;
  int m_order;
};

#endif // MODELS_H