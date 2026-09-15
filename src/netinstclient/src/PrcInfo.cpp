#include "PrcInfo.h"

#include <QFile>

static quint32 get4(const QByteArray &b, int pos) {
  return (quint32(quint8(b[pos])) << 24) | (quint32(quint8(b[pos + 1])) << 16) |
         (quint32(quint8(b[pos + 2])) << 8) | quint32(quint8(b[pos + 3]));
}

static quint16 get2(const QByteArray &b, int pos) {
  return quint16((quint16(quint8(b[pos])) << 8) | quint16(quint8(b[pos + 1])));
}

QString PrcInfo::idToString(quint32 id) {
  QString s;
  for (int shift = 24; shift >= 0; shift -= 8) {
    const char c = char((id >> shift) & 0xFF);
    s += (c >= 32 && c != 127) ? QChar::fromLatin1(c) : QChar('?');
  }
  return s;
}

PrcInfo PrcInfo::fromHeader(const QByteArray &header, qint64 fileSize) {
  PrcInfo info;
  if (header.size() < HeaderLength) return info;

  int n = -1;
  for (int i = 0; i < NameLength; i++) {
    const quint8 c = quint8(header[i]);
    if (c == 0) { n = i; break; }
    if (c < 32 || c == 127) return info;
  }
  if (n <= 0) return info;

  info.name = QString::fromLatin1(header.constData(), n);
  info.attributes = get2(header, 32);
  info.version = get2(header, 34);
  info.type = get4(header, 60);
  info.creator = get4(header, 64);
  info.size = fileSize;
  info.valid = true;
  return info;
}

PrcInfo PrcInfo::fromFile(const QString &path) {
  QFile f(path);
  if (!f.open(QIODevice::ReadOnly)) return PrcInfo();
  const QByteArray header = f.read(HeaderLength);
  return fromHeader(header, f.size());
}
