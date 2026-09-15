// netinstclient - installs PalmOS applications on a PumpkinOS device over
// the network.  Without arguments a Qt window opens; with --host (or
// --discover) the tool runs on the command line without a GUI.

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QTextStream>
#include <QTimer>

#include "Discovery.h"
#include "MainWindow.h"
#include "NetInstClient.h"

namespace {

QTextStream &out() {
  static QTextStream s(stdout);
  return s;
}

QTextStream &err() {
  static QTextStream s(stderr);
  return s;
}

// Runs the requested commands and exits the application when done.
class CliRunner : public QObject {
public:
  struct FileOps {
    QStringList ls;        // directories to list
    QStringList mkdirs;    // directories to create
    QStringList rms;       // paths to delete
    QStringList uploads;   // local files / directories to upload
    QString dest;          // remote directory for the uploads
  };

  CliRunner(const QString &host, quint16 port, bool list, const QStringList &deletes, const QStringList &files,
            const FileOps &fileOps, bool verbose, int timeoutMs, QObject *parent = nullptr)
    : QObject(parent), m_host(host), m_port(port), m_list(list), m_deletes(deletes), m_files(files), m_fileOps(fileOps), m_verbose(verbose) {
    m_client = new NetInstClient(this);
    m_client->setTimeout(timeoutMs);

    connect(m_client, &NetInstClient::logMessage, this, [this](const QString &msg) {
      if (m_verbose) err() << msg << '\n' << Qt::flush;
    });
    connect(m_client, &NetInstClient::errorOccurred, this, [this](const QString &msg) {
      err() << "error: " << msg << '\n' << Qt::flush;
      m_failed = true;
    });
    connect(m_client, &NetInstClient::connected, this, [this](const QString &name, quint32 maxSize) {
      err() << "connected to " << name << " (max file size " << maxSize << " bytes)\n" << Qt::flush;
      for (const QString &f : m_files) m_client->put(f);
      for (const QString &d : m_deletes) m_client->remove(d);
      if (m_list) m_client->list();
      for (const QString &d : m_fileOps.mkdirs) m_client->makeDir(d);
      for (const QString &u : m_fileOps.uploads) queueUpload(u, m_fileOps.dest);
      for (const QString &r : m_fileOps.rms) m_client->removePath(r);
      for (const QString &d : m_fileOps.ls) m_client->listDir(d == QLatin1String("/") ? QString() : d);
      m_client->quit();
    });
    connect(m_client, &NetInstClient::makeDirFinished, this, [this](const QString &path, bool ok, const QString &msg) {
      out() << (ok ? "mkdir     " : "FAILED    ") << path;
      if (!ok) out() << ": " << msg;
      out() << '\n' << Qt::flush;
      if (!ok) m_failed = true;
    });
    connect(m_client, &NetInstClient::uploadFinished, this, [this](const QString &local, const QString &remote, bool ok, const QString &msg) {
      m_lastPct = -1;
      out() << (ok ? "uploaded  " : "FAILED    ") << QFileInfo(local).fileName() << " -> " << remote;
      if (!ok) out() << ": " << msg;
      out() << '\n' << Qt::flush;
      if (!ok) m_failed = true;
    });
    connect(m_client, &NetInstClient::removePathFinished, this, [this](const QString &path, bool ok, const QString &msg) {
      out() << (ok ? "removed   " : "FAILED    ") << path;
      if (!ok) out() << ": " << msg;
      out() << '\n' << Qt::flush;
      if (!ok) m_failed = true;
    });
    connect(m_client, &NetInstClient::dirListed, this, [](const QString &path, const QList<FileEntry> &entries) {
      out() << path << ":\n";
      for (const FileEntry &e : entries) {
        out() << (e.isDir ? "  <DIR>  " : QStringLiteral("  %1 ").arg(e.size, 8)) << e.name << '\n';
      }
      out() << Qt::flush;
    });
    connect(m_client, &NetInstClient::dirListFailed, this, [this](const QString &, const QString &) { m_failed = true; });
    connect(m_client, &NetInstClient::putProgress, this, [this](const QString &path, qint64 sent, qint64 total) {
      if (!m_verbose || total <= 0) return;
      const int pct = int(sent * 100 / total);
      if (pct != m_lastPct) {
        m_lastPct = pct;
        err() << '\r' << QFileInfo(path).fileName() << ' ' << pct << '%' << (pct == 100 ? "\n" : "") << Qt::flush;
      }
    });
    connect(m_client, &NetInstClient::putFinished, this, [this](const QString &path, bool ok, const QString &msg) {
      m_lastPct = -1;
      out() << (ok ? "installed " : "FAILED    ") << QFileInfo(path).fileName();
      if (ok) out() << " -> \"" << msg << '"'; else out() << ": " << msg;
      out() << '\n' << Qt::flush;
      if (!ok) m_failed = true;
    });
    connect(m_client, &NetInstClient::removeFinished, this, [this](const QString &name, bool ok, const QString &msg) {
      out() << (ok ? "deleted   " : "FAILED    ") << '"' << name << '"';
      if (!ok) out() << ": " << msg;
      out() << '\n' << Qt::flush;
      if (!ok) m_failed = true;
    });
    connect(m_client, &NetInstClient::listReceived, this, [](const QList<DbEntry> &entries) {
      out() << QStringLiteral("%1 %2 %3 %4 %5 %6\n").arg(QStringLiteral("TYPE"), 4).arg(QStringLiteral("CREA"), 4)
                 .arg(QStringLiteral("ATTR"), 4).arg(QStringLiteral("VER"), 5).arg(QStringLiteral("SIZE"), 9).arg(QStringLiteral("NAME"));
      for (const DbEntry &e : entries) {
        out() << QStringLiteral("%1 %2 %3 %4 %5 %6\n").arg(e.typeString(), 4).arg(e.creatorString(), 4)
                   .arg(e.attributes, 4, 16, QLatin1Char('0')).arg(e.version, 5).arg(e.size, 9).arg(e.name);
      }
      out() << Qt::flush;
    });
    connect(m_client, &NetInstClient::disconnected, this, [this]() { finish(); });
    connect(m_client, &NetInstClient::queueDrained, this, [this]() {
      if (!m_client->isConnected()) finish();
    });
  }

  void start() {
    m_client->connectToHost(m_host, m_port);
  }

  // a file is uploaded into dest; a directory is recreated below dest
  void queueUpload(const QString &local, const QString &dest) {
    const QFileInfo fi(local);
    auto join = [](const QString &d, const QString &n) { return d.endsWith('/') ? d + n : d + '/' + n; };
    if (fi.isDir()) {
      const QString remoteBase = join(dest, fi.fileName());
      m_client->makeDir(remoteBase);
      const QDir base(local);
      QDirIterator it(local, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
      while (it.hasNext()) {
        const QString path = it.next();
        const QFileInfo sub(path);
        const QString rel = base.relativeFilePath(path);
        if (sub.isDir()) m_client->makeDir(join(remoteBase, rel));
        else if (sub.isFile()) m_client->uploadFile(path, join(remoteBase, rel));
      }
    } else {
      m_client->uploadFile(local, join(dest, fi.fileName()));
    }
  }

private:
  void finish() {
    if (m_finished) return;
    m_finished = true;
    QCoreApplication::exit(m_failed ? 1 : 0);
  }

  NetInstClient *m_client;
  QString m_host;
  quint16 m_port;
  bool m_list;
  QStringList m_deletes;
  QStringList m_files;
  FileOps m_fileOps;
  bool m_verbose;
  bool m_failed = false;
  bool m_finished = false;
  int m_lastPct = -1;
};

} // namespace

int main(int argc, char *argv[]) {
  QCoreApplication::setOrganizationName(QStringLiteral("PumpkinOS"));
  QCoreApplication::setApplicationName(QStringLiteral("NetInstClient"));
  QCoreApplication::setApplicationVersion(QStringLiteral(NETINSTCLIENT_VERSION));

  // decide between GUI and command line before creating the application object
  bool cli = false;
  for (int i = 1; i < argc; i++) {
    const QString a = QString::fromLocal8Bit(argv[i]);
    if (a == QLatin1String("--host") || a.startsWith(QLatin1String("--host=")) || a == QLatin1String("-H") ||
        a == QLatin1String("--discover") || a == QLatin1String("-h") || a == QLatin1String("--help") ||
        a == QLatin1String("-v") || a == QLatin1String("--version")) {
      cli = true;
    }
  }

  QScopedPointer<QCoreApplication> app(cli ? new QCoreApplication(argc, argv) : new QApplication(argc, argv));

  QCommandLineParser parser;
  parser.setApplicationDescription(QStringLiteral(
    "Installs PalmOS applications (.prc / .pdb) on a PumpkinOS device running the \"Net Install\" application.\n"
    "Without --host or --discover a graphical window is opened; files given on the command line are added to it."));
  parser.addHelpOption();
  parser.addVersionOption();
  const QCommandLineOption hostOpt({QStringLiteral("H"), QStringLiteral("host")}, QStringLiteral("Device address (enables command line mode)."), QStringLiteral("host"));
  const QCommandLineOption portOpt({QStringLiteral("p"), QStringLiteral("port")}, QStringLiteral("TCP port of the device (default %1).").arg(NetInstClient::DefaultPort), QStringLiteral("port"));
  const QCommandLineOption listOpt({QStringLiteral("l"), QStringLiteral("list")}, QStringLiteral("List the databases installed on the device."));
  const QCommandLineOption deleteOpt({QStringLiteral("d"), QStringLiteral("delete")}, QStringLiteral("Delete the database with this name (repeatable)."), QStringLiteral("name"));
  const QCommandLineOption discoverOpt(QStringLiteral("discover"), QStringLiteral("Find devices on the local network and exit."));
  const QCommandLineOption timeoutOpt(QStringLiteral("timeout"), QStringLiteral("Network timeout in seconds (default 30)."), QStringLiteral("seconds"));
  const QCommandLineOption quietOpt({QStringLiteral("q"), QStringLiteral("quiet")}, QStringLiteral("Do not print progress."));
  const QCommandLineOption lsOpt(QStringLiteral("ls"), QStringLiteral("List a folder of the device's card (\"/\" = the file root, /PALM/Programs). Repeatable."), QStringLiteral("path"));
  const QCommandLineOption mkdirOpt(QStringLiteral("mkdir"), QStringLiteral("Create a folder (with its parents) on the card. Repeatable."), QStringLiteral("path"));
  const QCommandLineOption rmOpt(QStringLiteral("rm"), QStringLiteral("Delete a file or an empty folder on the card. Repeatable."), QStringLiteral("path"));
  const QCommandLineOption uploadOpt(QStringLiteral("upload"), QStringLiteral("Upload a file, or a folder with its contents, into --dest. Repeatable."), QStringLiteral("local path"));
  const QCommandLineOption destOpt(QStringLiteral("dest"), QStringLiteral("Folder on the card that --upload writes into (default /PALM/Programs)."), QStringLiteral("path"));
  parser.addOption(hostOpt);
  parser.addOption(portOpt);
  parser.addOption(listOpt);
  parser.addOption(deleteOpt);
  parser.addOption(discoverOpt);
  parser.addOption(timeoutOpt);
  parser.addOption(quietOpt);
  parser.addOption(lsOpt);
  parser.addOption(mkdirOpt);
  parser.addOption(rmOpt);
  parser.addOption(uploadOpt);
  parser.addOption(destOpt);
  parser.addPositionalArgument(QStringLiteral("files"), QStringLiteral("PalmOS .prc / .pdb files to install."), QStringLiteral("[files...]"));
  parser.process(*app);

  quint16 port = NetInstClient::DefaultPort;
  if (parser.isSet(portOpt)) {
    const int p = parser.value(portOpt).toInt();
    if (p < 1 || p > 65535) {
      err() << "invalid port\n";
      return 2;
    }
    port = quint16(p);
  }
  const QStringList files = parser.positionalArguments();

  if (parser.isSet(discoverOpt)) {
    Discovery discovery;
    QObject::connect(&discovery, &Discovery::deviceFound, [](const DiscoveredDevice &dev) {
      out() << dev.address.toString() << ' ' << dev.port << ' ' << dev.name << '\n' << Qt::flush;
    });
    QObject::connect(&discovery, &Discovery::finished, [&discovery]() {
      QCoreApplication::exit(discovery.devices().isEmpty() ? 1 : 0);
    });
    discovery.start(port, 2000);
    return app->exec();
  }

  if (cli) {
    const QString host = parser.value(hostOpt);
    if (host.isEmpty()) {
      err() << "missing --host\n";
      return 2;
    }
    for (const QString &f : files) {
      if (!QFileInfo::exists(f)) {
        err() << "no such file: " << f << '\n';
        return 2;
      }
    }
    CliRunner::FileOps fileOps;
    fileOps.ls = parser.values(lsOpt);
    fileOps.mkdirs = parser.values(mkdirOpt);
    fileOps.rms = parser.values(rmOpt);
    fileOps.uploads = parser.values(uploadOpt);
    fileOps.dest = parser.isSet(destOpt) ? parser.value(destOpt) : QStringLiteral("/PALM/Programs");
    for (const QString &u : fileOps.uploads) {
      if (!QFileInfo::exists(u)) {
        err() << "no such file: " << u << '\n';
        return 2;
      }
    }
    if (files.isEmpty() && !parser.isSet(listOpt) && !parser.isSet(deleteOpt) && fileOps.ls.isEmpty() &&
        fileOps.mkdirs.isEmpty() && fileOps.rms.isEmpty() && fileOps.uploads.isEmpty()) {
      err() << "nothing to do: give files to install, --list, --delete, --ls, --mkdir, --upload or --rm\n";
      return 2;
    }
    const int timeoutMs = parser.isSet(timeoutOpt) ? parser.value(timeoutOpt).toInt() * 1000 : 30000;
    CliRunner runner(host, port, parser.isSet(listOpt), parser.values(deleteOpt), files, fileOps, !parser.isSet(quietOpt), timeoutMs);
    QTimer::singleShot(0, &runner, [&runner]() { runner.start(); });
    return app->exec();
  }

  MainWindow window;
  if (parser.isSet(portOpt)) window.setHost(QString(), port);
  if (!files.isEmpty()) window.addFiles(files);
  window.show();
  return app->exec();
}
