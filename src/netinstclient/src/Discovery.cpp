#include "Discovery.h"

#include <QNetworkInterface>

Discovery::Discovery(QObject *parent) : QObject(parent) {
  connect(&m_socket, &QUdpSocket::readyRead, this, &Discovery::onReadyRead);
  m_timer.setSingleShot(true);
  connect(&m_timer, &QTimer::timeout, this, [this]() {
    m_socket.close();
    emit finished();
  });
}

void Discovery::start(quint16 port, int timeoutMs) {
  stop();
  m_devices.clear();

  if (!m_socket.bind(QHostAddress::AnyIPv4, 0, QUdpSocket::ShareAddress)) {
    emit finished();
    return;
  }

  const QByteArray query("NETINST?");
  m_socket.writeDatagram(query, QHostAddress::Broadcast, port);

  // also the directed broadcast address of every IPv4 interface, some
  // systems do not route 255.255.255.255 to all of them
  const QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
  for (const QNetworkInterface &iface : ifaces) {
    if (!(iface.flags() & QNetworkInterface::IsUp) || !(iface.flags() & QNetworkInterface::CanBroadcast)) continue;
    const QList<QNetworkAddressEntry> entries = iface.addressEntries();
    for (const QNetworkAddressEntry &entry : entries) {
      if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol) continue;
      const QHostAddress bcast = entry.broadcast();
      if (!bcast.isNull()) m_socket.writeDatagram(query, bcast, port);
    }
  }
  // and the local machine itself (a PumpkinOS running on this host)
  m_socket.writeDatagram(query, QHostAddress::LocalHost, port);

  m_timer.start(timeoutMs);
}

void Discovery::stop() {
  m_timer.stop();
  if (m_socket.state() != QAbstractSocket::UnconnectedState) m_socket.close();
}

void Discovery::onReadyRead() {
  while (m_socket.hasPendingDatagrams()) {
    QByteArray data;
    data.resize(int(m_socket.pendingDatagramSize()));
    QHostAddress sender;
    quint16 senderPort = 0;
    const qint64 n = m_socket.readDatagram(data.data(), data.size(), &sender, &senderPort);
    if (n <= 0) continue;
    data.truncate(int(n));

    // NETINST! <port> <name>
    if (!data.startsWith("NETINST!")) continue;
    const QString text = QString::fromLatin1(data).trimmed();
    DiscoveredDevice dev;
    dev.address = sender;
    dev.port = quint16(text.section(' ', 1, 1).toUInt());
    dev.name = text.section(' ', 2);
    if (dev.port == 0) dev.port = senderPort;

    // IPv4-mapped addresses look nicer as plain IPv4
    bool ok = false;
    const quint32 v4 = sender.toIPv4Address(&ok);
    if (ok) dev.address = QHostAddress(v4);

    bool dup = false;
    for (const DiscoveredDevice &d : m_devices) {
      if (d.address == dev.address && d.port == dev.port) { dup = true; break; }
    }
    if (dup) continue;

    m_devices.append(dev);
    emit deviceFound(dev);
  }
}
