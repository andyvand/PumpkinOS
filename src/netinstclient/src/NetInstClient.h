#pragma once

#include <QObject>
#include <QList>
#include <QFile>
#include <QTimer>
#include <QTcpSocket>

#include "PrcInfo.h"

// One database installed on the device, as reported by LIST.
struct DbEntry {
  QString name;
  quint32 type = 0;
  quint32 creator = 0;
  quint16 attributes = 0;
  quint16 version = 0;
  quint32 size = 0;

  QString typeString() const { return PrcInfo::idToString(type); }
  QString creatorString() const { return PrcInfo::idToString(creator); }
  bool isApplication() const { return type == 0x6170706C; }
  bool isResourceDb() const { return attributes & 0x0001; }
};

// One entry of a directory on the device's card, as reported by LS.
struct FileEntry {
  QString name;
  bool isDir = false;
  quint32 size = 0;
};

// Client side of the Net Install protocol (see src/netinstserver/netinst.h).
//
// Commands are queued and executed one after the other; every command
// reports its outcome through a signal, and queueDrained() is emitted when
// the queue becomes empty.
class NetInstClient : public QObject {
  Q_OBJECT
public:
  static const quint16 DefaultPort = 5150;

  explicit NetInstClient(QObject *parent = nullptr);

  bool isConnected() const;
  bool isBusy() const;              // a command is being executed
  int pendingCount() const;         // queued commands (excluding the current one)
  QString deviceName() const { return m_deviceName; }
  quint32 maxSize() const { return m_maxSize; }
  QString peerAddress() const;

  void setTimeout(int ms) { m_timeoutMs = ms; }

public slots:
  void connectToHost(const QString &host, quint16 port);
  void disconnectFromHost();
  void ping();
  void list();
  void put(const QString &path);
  void remove(const QString &dbName);
  // file commands (paths are absolute on the device, inside its file root)
  void listDir(const QString &path);                                  // empty path: the file root
  void makeDir(const QString &path);
  void uploadFile(const QString &localPath, const QString &remotePath);
  void removePath(const QString &path);
  void quit();
  void clearQueue();

signals:
  void connected(const QString &deviceName, quint32 maxSize);
  void disconnected();
  void errorOccurred(const QString &message);
  void logMessage(const QString &message);
  void pongReceived();
  void listReceived(const QList<DbEntry> &entries);
  void putProgress(const QString &path, qint64 sent, qint64 total);
  void putFinished(const QString &path, bool ok, const QString &message);
  void removeFinished(const QString &dbName, bool ok, const QString &message);
  void dirListed(const QString &path, const QList<FileEntry> &entries);
  void dirListFailed(const QString &path, const QString &message);
  void makeDirFinished(const QString &path, bool ok, const QString &message);
  void uploadFinished(const QString &localPath, const QString &remotePath, bool ok, const QString &message);
  void removePathFinished(const QString &path, bool ok, const QString &message);
  void queueDrained();

private:
  enum class Kind { Ping, List, Put, Del, Ls, Mkdir, Fput, Fdel, Quit };
  struct Command {
    Kind kind = Kind::Ping;
    QString arg;    // file path, database name or remote path
    QString arg2;   // Fput: remote path
  };
  enum class State { Disconnected, Greeting, Idle, WaitReply, ListBody, DirBody, WaitGo, Sending, WaitPutResult };

  void onConnected();
  void onDisconnected();
  void onSocketError(QAbstractSocket::SocketError error);
  void onReadyRead();
  void onBytesWritten(qint64 bytes);
  void onTimeout();

  void handleLine(const QByteArray &line);
  void startNext();
  void finishCurrent(bool ok, const QString &message);
  void sendLine(const QByteArray &line);
  void pumpFile();
  void abortAll(const QString &reason);
  void resetTimeout();

  QTcpSocket *m_socket;
  QTimer m_timer;
  int m_timeoutMs = 30000;
  State m_state = State::Disconnected;
  QList<Command> m_queue;
  Command m_current;
  bool m_hasCurrent = false;
  QByteArray m_lineBuf;

  QString m_deviceName;
  quint32 m_maxSize = 0;

  // LIST / LS
  QList<DbEntry> m_list;
  QList<FileEntry> m_dir;
  QString m_dirPath;
  int m_listRemaining = 0;

  // PUT
  QFile m_file;
  qint64 m_fileSize = 0;
  qint64 m_fileQueued = 0;   // bytes handed to the socket
};
