#pragma once

#include <QObject>
#include <QHostAddress>
#include <QList>
#include <QTimer>
#include <QUdpSocket>

struct DiscoveredDevice {
  QHostAddress address;
  quint16 port = 0;
  QString name;
};

// Finds Net Install servers on the local network: broadcasts "NETINST?" on
// the UDP port and collects the "NETINST! <port> <name>" answers.
class Discovery : public QObject {
  Q_OBJECT
public:
  explicit Discovery(QObject *parent = nullptr);

  void start(quint16 port, int timeoutMs = 1500);
  void stop();
  bool isRunning() const { return m_timer.isActive(); }
  QList<DiscoveredDevice> devices() const { return m_devices; }

signals:
  void deviceFound(const DiscoveredDevice &device);
  void finished();

private:
  void onReadyRead();

  QUdpSocket m_socket;
  QTimer m_timer;
  QList<DiscoveredDevice> m_devices;
};
