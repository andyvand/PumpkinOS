#include "NetInstClient.h"

#include <QFileInfo>
#include <QHostAddress>

static const qint64 ChunkSize = 64 * 1024;
static const qint64 MaxBuffered = 512 * 1024;

NetInstClient::NetInstClient(QObject *parent)
  : QObject(parent), m_socket(new QTcpSocket(this)) {
  connect(m_socket, &QTcpSocket::connected, this, &NetInstClient::onConnected);
  connect(m_socket, &QTcpSocket::disconnected, this, &NetInstClient::onDisconnected);
  connect(m_socket, &QTcpSocket::readyRead, this, &NetInstClient::onReadyRead);
  connect(m_socket, &QTcpSocket::bytesWritten, this, &NetInstClient::onBytesWritten);
  connect(m_socket, &QAbstractSocket::errorOccurred, this, &NetInstClient::onSocketError);

  m_timer.setSingleShot(true);
  connect(&m_timer, &QTimer::timeout, this, &NetInstClient::onTimeout);
}

bool NetInstClient::isConnected() const {
  return m_socket->state() == QAbstractSocket::ConnectedState && m_state != State::Disconnected && m_state != State::Greeting;
}

bool NetInstClient::isBusy() const {
  return m_hasCurrent;
}

int NetInstClient::pendingCount() const {
  return m_queue.size();
}

QString NetInstClient::peerAddress() const {
  return m_socket->peerAddress().toString();
}

void NetInstClient::connectToHost(const QString &host, quint16 port) {
  if (m_socket->state() != QAbstractSocket::UnconnectedState) {
    m_socket->abort();
  }
  m_lineBuf.clear();
  m_queue.clear();
  m_hasCurrent = false;
  m_deviceName.clear();
  m_maxSize = 0;
  m_state = State::Greeting;
  emit logMessage(tr("Connecting to %1:%2...").arg(host).arg(port));
  m_socket->connectToHost(host, port);
  resetTimeout();
}

void NetInstClient::disconnectFromHost() {
  if (m_socket->state() == QAbstractSocket::UnconnectedState) return;
  m_timer.stop();
  m_socket->disconnectFromHost();
  if (m_socket->state() != QAbstractSocket::UnconnectedState) {
    m_socket->waitForDisconnected(1000);
  }
}

void NetInstClient::ping() {
  m_queue.append({Kind::Ping, QString(), QString()});
  startNext();
}

void NetInstClient::list() {
  m_queue.append({Kind::List, QString(), QString()});
  startNext();
}

void NetInstClient::put(const QString &path) {
  m_queue.append({Kind::Put, path, QString()});
  startNext();
}

void NetInstClient::remove(const QString &dbName) {
  m_queue.append({Kind::Del, dbName, QString()});
  startNext();
}

void NetInstClient::listDir(const QString &path) {
  m_queue.append({Kind::Ls, path, QString()});
  startNext();
}

void NetInstClient::makeDir(const QString &path) {
  m_queue.append({Kind::Mkdir, path, QString()});
  startNext();
}

void NetInstClient::uploadFile(const QString &localPath, const QString &remotePath) {
  m_queue.append({Kind::Fput, localPath, remotePath});
  startNext();
}

void NetInstClient::removePath(const QString &path) {
  m_queue.append({Kind::Fdel, path, QString()});
  startNext();
}

void NetInstClient::quit() {
  m_queue.append({Kind::Quit, QString(), QString()});
  startNext();
}

void NetInstClient::clearQueue() {
  m_queue.clear();
}

void NetInstClient::resetTimeout() {
  if (m_timeoutMs > 0) m_timer.start(m_timeoutMs);
}

void NetInstClient::onConnected() {
  // wait for the greeting line before reporting the connection
  resetTimeout();
}

void NetInstClient::onDisconnected() {
  m_timer.stop();
  const bool wasConnected = m_state != State::Disconnected;
  m_state = State::Disconnected;
  if (m_hasCurrent) {
    finishCurrent(false, tr("connection closed"));
  }
  if (!m_queue.isEmpty()) {
    m_queue.clear();
    emit queueDrained();
  }
  if (m_file.isOpen()) m_file.close();
  if (wasConnected) {
    emit logMessage(tr("Disconnected"));
    emit disconnected();
  }
}

void NetInstClient::onSocketError(QAbstractSocket::SocketError error) {
  Q_UNUSED(error);
  if (error == QAbstractSocket::RemoteHostClosedError) return;  // reported by disconnected()
  abortAll(m_socket->errorString());
}

void NetInstClient::onTimeout() {
  abortAll(tr("timeout waiting for the device"));
}

void NetInstClient::abortAll(const QString &reason) {
  m_timer.stop();
  const bool active = m_state != State::Disconnected;
  m_state = State::Disconnected;
  emit errorOccurred(reason);
  if (m_hasCurrent) finishCurrent(false, reason);
  if (!m_queue.isEmpty()) {
    m_queue.clear();
    emit queueDrained();
  }
  if (m_file.isOpen()) m_file.close();
  if (m_socket->state() != QAbstractSocket::UnconnectedState) {
    m_socket->abort();
  }
  if (active) {
    emit logMessage(tr("Disconnected: %1").arg(reason));
    emit disconnected();
  }
}

void NetInstClient::sendLine(const QByteArray &line) {
  m_socket->write(line + '\n');
  resetTimeout();
}

void NetInstClient::onReadyRead() {
  m_lineBuf += m_socket->readAll();
  int pos;
  while ((pos = m_lineBuf.indexOf('\n')) >= 0) {
    QByteArray line = m_lineBuf.left(pos);
    m_lineBuf.remove(0, pos + 1);
    if (line.endsWith('\r')) line.chop(1);
    handleLine(line);
    if (m_state == State::Disconnected) break;
  }
  if (m_lineBuf.size() > 65536) {
    abortAll(tr("protocol error: line too long"));
  }
}

void NetInstClient::handleLine(const QByteArray &line) {
  resetTimeout();

  if (m_state == State::ListBody) {
    // <type-hex8> <creator-hex8> <attr-hex4> <version> <size> <name>
    const QList<QByteArray> parts = line.split(' ');
    if (parts.size() >= 6) {
      DbEntry e;
      e.type = parts[0].toUInt(nullptr, 16);
      e.creator = parts[1].toUInt(nullptr, 16);
      e.attributes = quint16(parts[2].toUInt(nullptr, 16));
      e.version = quint16(parts[3].toUInt());
      e.size = parts[4].toUInt();
      int nameStart = 0;
      for (int i = 0; i < 5; i++) nameStart = line.indexOf(' ', nameStart) + 1;
      e.name = QString::fromLatin1(line.mid(nameStart));
      m_list.append(e);
    }
    if (--m_listRemaining <= 0) {
      m_state = State::Idle;
      emit listReceived(m_list);
      finishCurrent(true, tr("%1 databases").arg(m_list.size()));
    }
    return;
  }

  if (m_state == State::DirBody) {
    // <D|F> <size> <name>
    if (line.size() >= 4) {
      FileEntry e;
      e.isDir = line[0] == 'D';
      const int sp = line.indexOf(' ', 2);
      e.size = line.mid(2, sp - 2).toUInt();
      e.name = QString::fromLatin1(line.mid(sp + 1));
      m_dir.append(e);
    }
    if (--m_listRemaining <= 0) {
      m_state = State::Idle;
      emit dirListed(m_dirPath, m_dir);
      finishCurrent(true, tr("%1 entries").arg(m_dir.size()));
    }
    return;
  }

  const bool ok = line.startsWith('+');
  const bool err = line.startsWith('-');
  if (!ok && !err) {
    abortAll(tr("protocol error: %1").arg(QString::fromLatin1(line)));
    return;
  }
  const QString full = QString::fromLatin1(line.mid(1));
  const QString word = full.section(' ', 0, 0);
  const QString rest = full.section(' ', 1);
  // "-ERR <message>": report only the message
  const QString text = (err && word == QLatin1String("ERR")) ? rest : full;

  switch (m_state) {
    case State::Greeting:
      if (ok && word == QLatin1String("NETINST")) {
        // +NETINST <version> <maxsize> <device name>
        m_maxSize = rest.section(' ', 1, 1).toUInt();
        m_deviceName = rest.section(' ', 2);
        m_state = State::Idle;
        emit logMessage(tr("Connected to %1 (%2)").arg(m_deviceName, peerAddress()));
        emit connected(m_deviceName, m_maxSize);
        startNext();
      } else {
        abortAll(err ? text : tr("unexpected greeting: %1").arg(text));
      }
      break;

    case State::WaitReply:
      m_state = State::Idle;
      if (!m_hasCurrent) break;
      if (m_current.kind == Kind::List && ok && word == QLatin1String("LIST")) {
        m_list.clear();
        m_listRemaining = rest.toInt();
        if (m_listRemaining > 0) {
          m_state = State::ListBody;
        } else {
          emit listReceived(m_list);
          finishCurrent(true, tr("0 databases"));
        }
      } else if (m_current.kind == Kind::Ls && ok && word == QLatin1String("LS")) {
        // +LS <n> <path>
        m_dir.clear();
        m_listRemaining = rest.section(' ', 0, 0).toInt();
        m_dirPath = rest.section(' ', 1);
        if (m_listRemaining > 0) {
          m_state = State::DirBody;
        } else {
          emit dirListed(m_dirPath, m_dir);
          finishCurrent(true, tr("0 entries"));
        }
      } else if (m_current.kind == Kind::Ping) {
        if (ok) emit pongReceived();
        finishCurrent(ok, ok ? tr("pong") : text);
      } else if (m_current.kind == Kind::Quit) {
        finishCurrent(ok, text);
        m_socket->disconnectFromHost();
      } else {
        finishCurrent(ok, ok ? rest : text);
      }
      break;

    case State::WaitGo:
      if (ok && word == QLatin1String("GO")) {
        m_state = State::Sending;
        m_fileQueued = 0;
        emit putProgress(m_current.arg, 0, m_fileSize);
        pumpFile();   /* a zero byte file goes straight to WaitPutResult */
      } else {
        m_state = State::Idle;
        m_file.close();
        finishCurrent(false, text);
      }
      break;

    case State::Sending:
    case State::WaitPutResult:
      m_state = State::Idle;
      m_file.close();
      if (ok) emit putProgress(m_current.arg, m_fileSize, m_fileSize);
      finishCurrent(ok, ok ? rest : text);
      break;

    case State::Idle:
      // an unsolicited line (for example "-ERR busy"); ignore
      if (err) emit logMessage(text);
      break;

    case State::Disconnected:
    case State::ListBody:
    case State::DirBody:
      break;
  }
}

void NetInstClient::onBytesWritten(qint64 bytes) {
  Q_UNUSED(bytes);
  if (m_state == State::Sending || m_state == State::WaitPutResult) {
    resetTimeout();
    const qint64 sent = m_fileQueued - m_socket->bytesToWrite();
    emit putProgress(m_current.arg, qBound<qint64>(0, sent, m_fileSize), m_fileSize);
  }
  if (m_state == State::Sending) pumpFile();
}

void NetInstClient::pumpFile() {
  while (m_state == State::Sending && m_fileQueued < m_fileSize && m_socket->bytesToWrite() < MaxBuffered) {
    const QByteArray chunk = m_file.read(qMin(ChunkSize, m_fileSize - m_fileQueued));
    if (chunk.isEmpty()) {
      abortAll(tr("could not read %1").arg(m_current.arg));
      return;
    }
    m_socket->write(chunk);
    m_fileQueued += chunk.size();
  }
  if (m_state == State::Sending && m_fileQueued >= m_fileSize) {
    m_state = State::WaitPutResult;
  }
}

void NetInstClient::startNext() {
  if (m_hasCurrent || m_state != State::Idle) return;
  if (m_queue.isEmpty()) {
    emit queueDrained();
    return;
  }

  m_current = m_queue.takeFirst();
  m_hasCurrent = true;

  switch (m_current.kind) {
    case Kind::Ping:
      m_state = State::WaitReply;
      sendLine("PING");
      break;

    case Kind::List:
      m_state = State::WaitReply;
      sendLine("LIST");
      break;

    case Kind::Del:
      m_state = State::WaitReply;
      sendLine("DEL " + m_current.arg.toLatin1());
      break;

    case Kind::Quit:
      m_state = State::WaitReply;
      sendLine("QUIT");
      break;

    case Kind::Ls:
      m_state = State::WaitReply;
      sendLine(m_current.arg.isEmpty() ? QByteArray("LS") : "LS " + m_current.arg.toLatin1());
      break;

    case Kind::Mkdir:
      m_state = State::WaitReply;
      sendLine("MKDIR " + m_current.arg.toLatin1());
      break;

    case Kind::Fdel:
      m_state = State::WaitReply;
      sendLine("FDEL " + m_current.arg.toLatin1());
      break;

    case Kind::Fput: {
      m_file.close();
      m_file.setFileName(m_current.arg);
      if (!m_file.open(QIODevice::ReadOnly)) {
        finishCurrent(false, tr("cannot open file"));
        return;
      }
      m_fileSize = m_file.size();
      if (m_maxSize && m_fileSize > qint64(m_maxSize)) {
        m_file.close();
        finishCurrent(false, tr("file is larger than the device accepts (%1 bytes)").arg(m_maxSize));
        return;
      }
      m_fileQueued = 0;
      m_state = State::WaitGo;
      emit logMessage(tr("Uploading %1 to %2 (%3 bytes)...").arg(QFileInfo(m_current.arg).fileName(), m_current.arg2).arg(m_fileSize));
      sendLine("FPUT " + QByteArray::number(m_fileSize) + ' ' + m_current.arg2.toLatin1());
      break;
    }

    case Kind::Put: {
      const QFileInfo fi(m_current.arg);
      const PrcInfo info = PrcInfo::fromFile(m_current.arg);
      if (!info.valid) {
        finishCurrent(false, tr("not a PalmOS database file"));
        return;
      }
      if (m_maxSize && info.size > qint64(m_maxSize)) {
        finishCurrent(false, tr("file is larger than the device accepts (%1 bytes)").arg(m_maxSize));
        return;
      }
      m_file.close();
      m_file.setFileName(m_current.arg);
      if (!m_file.open(QIODevice::ReadOnly)) {
        finishCurrent(false, tr("cannot open file"));
        return;
      }
      m_fileSize = m_file.size();
      m_fileQueued = 0;
      m_state = State::WaitGo;
      emit logMessage(tr("Sending %1 (%2, %3 bytes)...").arg(fi.fileName(), info.name).arg(m_fileSize));
      sendLine("PUT " + QByteArray::number(m_fileSize) + ' ' + fi.fileName().toLatin1());
      break;
    }
  }
}

void NetInstClient::finishCurrent(bool ok, const QString &message) {
  if (!m_hasCurrent) return;
  const Command cmd = m_current;
  m_hasCurrent = false;

  switch (cmd.kind) {
    case Kind::Put:
      if (ok) {
        emit logMessage(tr("Installed %1 as \"%2\"").arg(QFileInfo(cmd.arg).fileName(), message));
      } else {
        emit logMessage(tr("Install of %1 failed: %2").arg(QFileInfo(cmd.arg).fileName(), message));
      }
      emit putFinished(cmd.arg, ok, message);
      break;
    case Kind::Del:
      emit logMessage(ok ? tr("Deleted \"%1\"").arg(cmd.arg) : tr("Delete of \"%1\" failed: %2").arg(cmd.arg, message));
      emit removeFinished(cmd.arg, ok, message);
      break;
    case Kind::List:
      if (!ok) emit logMessage(tr("List failed: %1").arg(message));
      break;
    case Kind::Ls:
      if (!ok) {
        emit logMessage(tr("Listing %1 failed: %2").arg(cmd.arg.isEmpty() ? tr("the file root") : cmd.arg, message));
        emit dirListFailed(cmd.arg, message);
      }
      break;
    case Kind::Mkdir:
      emit logMessage(ok ? tr("Created folder %1").arg(message) : tr("Creating folder %1 failed: %2").arg(cmd.arg, message));
      emit makeDirFinished(cmd.arg, ok, message);
      break;
    case Kind::Fput:
      if (ok) {
        emit logMessage(tr("Uploaded %1").arg(message));
      } else {
        emit logMessage(tr("Upload of %1 failed: %2").arg(QFileInfo(cmd.arg).fileName(), message));
      }
      emit uploadFinished(cmd.arg, cmd.arg2, ok, message);
      break;
    case Kind::Fdel:
      emit logMessage(ok ? tr("Deleted %1").arg(cmd.arg) : tr("Deleting %1 failed: %2").arg(cmd.arg, message));
      emit removePathFinished(cmd.arg, ok, message);
      break;
    case Kind::Ping:
      if (!ok) emit logMessage(tr("Ping failed: %1").arg(message));
      break;
    case Kind::Quit:
      break;
  }

  if (m_state == State::Idle) {
    startNext();
  } else if (m_state == State::Disconnected && m_queue.isEmpty()) {
    emit queueDrained();
  }
}
