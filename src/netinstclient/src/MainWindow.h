#pragma once

#include <QMainWindow>
#include <QList>
#include <QTableWidget>

#include "NetInstClient.h"
#include "Discovery.h"

class QComboBox;
class QTabWidget;
class QLineEdit;

// Table that accepts dropped files and reports them
class DropTableWidget : public QTableWidget {
  Q_OBJECT
public:
  explicit DropTableWidget(int rows, int columns, QWidget *parent = nullptr);
signals:
  void filesDropped(const QStringList &paths);
protected:
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dragMoveEvent(QDragMoveEvent *event) override;
  void dropEvent(QDropEvent *event) override;
};
class QSpinBox;
class QPushButton;
class QListWidget;
class QProgressBar;
class QPlainTextEdit;
class QLabel;
class QCheckBox;

class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow() override;

  void addFiles(const QStringList &paths);
  void setHost(const QString &host, quint16 port);

  // queue the upload of local files/directories into the remote directory
  void uploadPaths(const QStringList &localPaths, const QString &remoteDir);

protected:
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dropEvent(QDropEvent *event) override;
  void closeEvent(QCloseEvent *event) override;

private:
  void buildUi();
  void loadSettings();
  void saveSettings();
  void updateUiState();
  void log(const QString &message);
  QString currentHost() const;
  quint16 currentPort() const;
  void rememberHost(const QString &host);

  // actions
  void onConnectClicked();
  void onDiscoverClicked();
  void onAddFiles();
  void onRemoveFiles();
  void onInstall();
  void onRefresh();
  void onDeleteSelected();

  // files tab
  void browseDir(const QString &path);
  void onDirUp();
  void onNewFolder();
  void onUploadFiles();
  void onUploadFolder();
  void onDeletePaths();
  void onDirEntryActivated(int row, int column);
  void onDirListed(const QString &path, const QList<FileEntry> &entries);
  QString remoteJoin(const QString &dir, const QString &name) const;

  // client signals
  void onConnected(const QString &deviceName, quint32 maxSize);
  void onDisconnected();
  void onListReceived(const QList<DbEntry> &entries);
  void onPutProgress(const QString &path, qint64 sent, qint64 total);
  void onPutFinished(const QString &path, bool ok, const QString &message);
  void onQueueDrained();

  NetInstClient *m_client;
  Discovery *m_discovery;

  QComboBox *m_hostCombo = nullptr;
  QSpinBox *m_portSpin = nullptr;
  QPushButton *m_discoverBtn = nullptr;
  QPushButton *m_connectBtn = nullptr;
  QLabel *m_statusLabel = nullptr;

  QListWidget *m_fileList = nullptr;
  QPushButton *m_addBtn = nullptr;
  QPushButton *m_removeBtn = nullptr;
  QPushButton *m_clearBtn = nullptr;
  QPushButton *m_installBtn = nullptr;
  QProgressBar *m_fileProgress = nullptr;
  QProgressBar *m_totalProgress = nullptr;
  QLabel *m_progressLabel = nullptr;

  QTabWidget *m_tabs = nullptr;
  QTableWidget *m_dbTable = nullptr;
  QPushButton *m_refreshBtn = nullptr;
  QPushButton *m_deleteBtn = nullptr;
  QCheckBox *m_appsOnlyCheck = nullptr;

  QLineEdit *m_dirPathEdit = nullptr;
  QPushButton *m_dirUpBtn = nullptr;
  QPushButton *m_dirGoBtn = nullptr;
  DropTableWidget *m_dirTable = nullptr;
  QPushButton *m_newFolderBtn = nullptr;
  QPushButton *m_uploadFilesBtn = nullptr;
  QPushButton *m_uploadFolderBtn = nullptr;
  QPushButton *m_dirDeleteBtn = nullptr;
  QPushButton *m_dirRefreshBtn = nullptr;
  QString m_currentDir;          // directory shown in the files tab ("" = root, not yet known)
  QString m_fileRoot;            // as reported by the device
  bool m_refreshDirAfterQueue = false;

  QPlainTextEdit *m_log = nullptr;

  QList<DbEntry> m_entries;
  QString m_lastDir;
  int m_installTotal = 0;
  int m_installDone = 0;
  int m_installFailed = 0;
  bool m_installing = false;
  bool m_refreshAfterQueue = false;
};
