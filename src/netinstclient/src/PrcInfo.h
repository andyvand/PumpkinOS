#pragma once

#include <QString>

// The 78 byte header of a PalmOS database (.prc / .pdb file).
struct PrcInfo {
  bool valid = false;
  QString name;        // database name (up to 31 characters)
  quint32 type = 0;
  quint32 creator = 0;
  quint16 attributes = 0;
  quint16 version = 0;
  qint64 size = 0;     // file size

  QString typeString() const { return idToString(type); }
  QString creatorString() const { return idToString(creator); }
  bool isApplication() const { return type == 0x6170706C; }  // 'appl'

  static PrcInfo fromFile(const QString &path);
  static PrcInfo fromHeader(const QByteArray &header, qint64 fileSize = 0);
  static QString idToString(quint32 id);
  static const int HeaderLength = 78;
  static const int NameLength = 32;
};
