#include "MainWindow.h"

#include <QApplication>
#include <QCheckBox>
#include <QCloseEvent>
#include <QComboBox>
#include <QDir>
#include <QDirIterator>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QInputDialog>
#include <QLineEdit>
#include <QTabWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QMimeData>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QTableWidget>
#include <QTime>
#include <QUrl>
#include <QVBoxLayout>

#include "PrcInfo.h"

static const int MaxRecentHosts = 10;
static const QString DefaultFileRoot = QStringLiteral("/PALM/Programs");

DropTableWidget::DropTableWidget(int rows, int columns, QWidget *parent) : QTableWidget(rows, columns, parent) {
  setAcceptDrops(true);
  viewport()->setAcceptDrops(true);
  setDragDropMode(QAbstractItemView::DropOnly);
}

void DropTableWidget::dragEnterEvent(QDragEnterEvent *event) {
  if (event->mimeData()->hasUrls()) event->acceptProposedAction();
}

void DropTableWidget::dragMoveEvent(QDragMoveEvent *event) {
  if (event->mimeData()->hasUrls()) event->acceptProposedAction();
}

void DropTableWidget::dropEvent(QDropEvent *event) {
  QStringList paths;
  const QList<QUrl> urls = event->mimeData()->urls();
  for (const QUrl &url : urls) {
    if (url.isLocalFile()) paths << url.toLocalFile();
  }
  if (!paths.isEmpty()) {
    event->acceptProposedAction();
    emit filesDropped(paths);
  }
}

static QString formatSize(qint64 bytes) {
  if (bytes < 1024) return QStringLiteral("%1 B").arg(bytes);
  if (bytes < 1024 * 1024) return QStringLiteral("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
  return QStringLiteral("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 2);
}

static bool isDatabaseFile(const QString &path) {
  const QString ext = QFileInfo(path).suffix().toLower();
  return ext == QLatin1String("prc") || ext == QLatin1String("pdb");
}

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent), m_client(new NetInstClient(this)), m_discovery(new Discovery(this)) {
  setWindowTitle(tr("PumpkinOS Net Install"));
  setAcceptDrops(true);
  buildUi();
  loadSettings();

  connect(m_client, &NetInstClient::connected, this, &MainWindow::onConnected);
  connect(m_client, &NetInstClient::disconnected, this, &MainWindow::onDisconnected);
  connect(m_client, &NetInstClient::errorOccurred, this, [this](const QString &msg) { log(tr("Error: %1").arg(msg)); });
  connect(m_client, &NetInstClient::logMessage, this, &MainWindow::log);
  connect(m_client, &NetInstClient::listReceived, this, &MainWindow::onListReceived);
  connect(m_client, &NetInstClient::putProgress, this, &MainWindow::onPutProgress);
  connect(m_client, &NetInstClient::putFinished, this, &MainWindow::onPutFinished);
  connect(m_client, &NetInstClient::uploadFinished, this, [this](const QString &local, const QString &, bool ok, const QString &msg) {
    onPutFinished(local, ok, msg);
  });
  connect(m_client, &NetInstClient::queueDrained, this, &MainWindow::onQueueDrained);
  connect(m_client, &NetInstClient::dirListed, this, &MainWindow::onDirListed);
  connect(m_client, &NetInstClient::dirListFailed, this, [this](const QString &, const QString &) {
    m_dirTable->setRowCount(0);
    updateUiState();
  });

  connect(m_discovery, &Discovery::deviceFound, this, [this](const DiscoveredDevice &dev) {
    const QString host = dev.address.toString();
    log(tr("Found %1 at %2:%3").arg(dev.name, host).arg(dev.port));
    if (m_hostCombo->findText(host) < 0) m_hostCombo->insertItem(0, host);
    m_hostCombo->setCurrentText(host);
    m_portSpin->setValue(dev.port);
  });
  connect(m_discovery, &Discovery::finished, this, [this]() {
    if (m_discovery->devices().isEmpty()) log(tr("No device answered on port %1").arg(m_portSpin->value()));
    m_discoverBtn->setEnabled(true);
    m_discoverBtn->setText(tr("Discover"));
  });

  updateUiState();
  log(tr("Start the \"Net Install\" application on the PumpkinOS device, then connect."));
}

MainWindow::~MainWindow() = default;

void MainWindow::buildUi() {
  QWidget *central = new QWidget(this);
  QVBoxLayout *mainLayout = new QVBoxLayout(central);

  // connection
  QGroupBox *connBox = new QGroupBox(tr("Device"), central);
  QHBoxLayout *connLayout = new QHBoxLayout(connBox);
  connLayout->addWidget(new QLabel(tr("Host:"), connBox));
  m_hostCombo = new QComboBox(connBox);
  m_hostCombo->setEditable(true);
  m_hostCombo->setInsertPolicy(QComboBox::NoInsert);
  m_hostCombo->setMinimumWidth(180);
  m_hostCombo->setToolTip(tr("IP address or host name of the PumpkinOS device"));
  connLayout->addWidget(m_hostCombo, 1);
  connLayout->addWidget(new QLabel(tr("Port:"), connBox));
  m_portSpin = new QSpinBox(connBox);
  m_portSpin->setRange(1, 65535);
  m_portSpin->setValue(NetInstClient::DefaultPort);
  connLayout->addWidget(m_portSpin);
  m_discoverBtn = new QPushButton(tr("Discover"), connBox);
  m_discoverBtn->setToolTip(tr("Look for PumpkinOS devices running Net Install on the local network"));
  connLayout->addWidget(m_discoverBtn);
  m_connectBtn = new QPushButton(tr("Connect"), connBox);
  m_connectBtn->setDefault(true);
  connLayout->addWidget(m_connectBtn);
  mainLayout->addWidget(connBox);

  // middle: files | installed
  QSplitter *splitter = new QSplitter(Qt::Horizontal, central);

  QGroupBox *filesBox = new QGroupBox(tr("Files to install"), splitter);
  QVBoxLayout *filesLayout = new QVBoxLayout(filesBox);
  m_fileList = new QListWidget(filesBox);
  m_fileList->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_fileList->setToolTip(tr("Drop .prc and .pdb files here"));
  filesLayout->addWidget(m_fileList, 1);
  QHBoxLayout *fileBtns = new QHBoxLayout;
  m_addBtn = new QPushButton(tr("Add Files..."), filesBox);
  m_removeBtn = new QPushButton(tr("Remove"), filesBox);
  m_clearBtn = new QPushButton(tr("Clear"), filesBox);
  fileBtns->addWidget(m_addBtn);
  fileBtns->addWidget(m_removeBtn);
  fileBtns->addWidget(m_clearBtn);
  fileBtns->addStretch(1);
  m_installBtn = new QPushButton(tr("Install"), filesBox);
  m_installBtn->setToolTip(tr("Send every file of the list to the device"));
  fileBtns->addWidget(m_installBtn);
  filesLayout->addLayout(fileBtns);
  m_progressLabel = new QLabel(filesBox);
  filesLayout->addWidget(m_progressLabel);
  m_fileProgress = new QProgressBar(filesBox);
  m_fileProgress->setRange(0, 100);
  m_fileProgress->setValue(0);
  m_fileProgress->setFormat(tr("file %p%"));
  filesLayout->addWidget(m_fileProgress);
  m_totalProgress = new QProgressBar(filesBox);
  m_totalProgress->setRange(0, 1);
  m_totalProgress->setValue(0);
  m_totalProgress->setFormat(tr("%v / %m files"));
  filesLayout->addWidget(m_totalProgress);
  splitter->addWidget(filesBox);

  m_tabs = new QTabWidget(splitter);

  QWidget *dbBox = new QWidget(m_tabs);
  QVBoxLayout *dbLayout = new QVBoxLayout(dbBox);
  m_dbTable = new QTableWidget(0, 5, dbBox);
  m_dbTable->setHorizontalHeaderLabels({tr("Name"), tr("Type"), tr("Creator"), tr("Version"), tr("Size")});
  m_dbTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  m_dbTable->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
  m_dbTable->verticalHeader()->setVisible(false);
  m_dbTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_dbTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_dbTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_dbTable->setSortingEnabled(true);
  dbLayout->addWidget(m_dbTable, 1);
  QHBoxLayout *dbBtns = new QHBoxLayout;
  m_appsOnlyCheck = new QCheckBox(tr("Applications only"), dbBox);
  m_appsOnlyCheck->setChecked(true);
  dbBtns->addWidget(m_appsOnlyCheck);
  dbBtns->addStretch(1);
  m_refreshBtn = new QPushButton(tr("Refresh"), dbBox);
  m_deleteBtn = new QPushButton(tr("Delete"), dbBox);
  dbBtns->addWidget(m_refreshBtn);
  dbBtns->addWidget(m_deleteBtn);
  dbLayout->addLayout(dbBtns);
  m_tabs->addTab(dbBox, tr("Applications"));

  // files on the card
  QWidget *filesTab = new QWidget(m_tabs);
  QVBoxLayout *ftLayout = new QVBoxLayout(filesTab);
  QHBoxLayout *pathRow = new QHBoxLayout;
  m_dirUpBtn = new QPushButton(tr("Up"), filesTab);
  m_dirUpBtn->setToolTip(tr("Parent folder"));
  pathRow->addWidget(m_dirUpBtn);
  m_dirPathEdit = new QLineEdit(filesTab);
  m_dirPathEdit->setToolTip(tr("Folder on the device's card (inside %1)").arg(DefaultFileRoot));
  pathRow->addWidget(m_dirPathEdit, 1);
  m_dirGoBtn = new QPushButton(tr("Go"), filesTab);
  pathRow->addWidget(m_dirGoBtn);
  ftLayout->addLayout(pathRow);
  m_dirTable = new DropTableWidget(0, 3, filesTab);
  m_dirTable->setHorizontalHeaderLabels({tr("Name"), tr("Kind"), tr("Size")});
  m_dirTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
  m_dirTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
  m_dirTable->verticalHeader()->setVisible(false);
  m_dirTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  m_dirTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
  m_dirTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
  m_dirTable->setSortingEnabled(true);
  m_dirTable->setToolTip(tr("Drop files or folders here to upload them into this folder; double-click a folder to open it"));
  ftLayout->addWidget(m_dirTable, 1);
  QHBoxLayout *dirBtns = new QHBoxLayout;
  m_newFolderBtn = new QPushButton(tr("New Folder..."), filesTab);
  m_uploadFilesBtn = new QPushButton(tr("Upload Files..."), filesTab);
  m_uploadFolderBtn = new QPushButton(tr("Upload Folder..."), filesTab);
  dirBtns->addWidget(m_newFolderBtn);
  dirBtns->addWidget(m_uploadFilesBtn);
  dirBtns->addWidget(m_uploadFolderBtn);
  dirBtns->addStretch(1);
  m_dirRefreshBtn = new QPushButton(tr("Refresh"), filesTab);
  m_dirDeleteBtn = new QPushButton(tr("Delete"), filesTab);
  dirBtns->addWidget(m_dirRefreshBtn);
  dirBtns->addWidget(m_dirDeleteBtn);
  ftLayout->addLayout(dirBtns);
  m_tabs->addTab(filesTab, tr("Files"));

  splitter->addWidget(m_tabs);
  splitter->setStretchFactor(0, 1);
  splitter->setStretchFactor(1, 1);
  mainLayout->addWidget(splitter, 1);

  // log
  m_log = new QPlainTextEdit(central);
  m_log->setReadOnly(true);
  m_log->setMaximumBlockCount(2000);
  m_log->setMinimumHeight(90);
  m_log->setMaximumHeight(160);
  mainLayout->addWidget(m_log);

  setCentralWidget(central);
  m_statusLabel = new QLabel(tr("Not connected"), this);
  statusBar()->addPermanentWidget(m_statusLabel);
  resize(860, 600);

  connect(m_connectBtn, &QPushButton::clicked, this, &MainWindow::onConnectClicked);
  connect(m_discoverBtn, &QPushButton::clicked, this, &MainWindow::onDiscoverClicked);
  connect(m_addBtn, &QPushButton::clicked, this, &MainWindow::onAddFiles);
  connect(m_removeBtn, &QPushButton::clicked, this, &MainWindow::onRemoveFiles);
  connect(m_clearBtn, &QPushButton::clicked, this, [this]() { m_fileList->clear(); updateUiState(); });
  connect(m_installBtn, &QPushButton::clicked, this, &MainWindow::onInstall);
  connect(m_refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefresh);
  connect(m_deleteBtn, &QPushButton::clicked, this, &MainWindow::onDeleteSelected);
  connect(m_appsOnlyCheck, &QCheckBox::toggled, this, [this](bool) { onListReceived(m_entries); });
  connect(m_dirUpBtn, &QPushButton::clicked, this, &MainWindow::onDirUp);
  connect(m_dirGoBtn, &QPushButton::clicked, this, [this]() { browseDir(m_dirPathEdit->text().trimmed()); });
  connect(m_dirPathEdit, &QLineEdit::returnPressed, this, [this]() { browseDir(m_dirPathEdit->text().trimmed()); });
  connect(m_dirRefreshBtn, &QPushButton::clicked, this, [this]() { browseDir(m_currentDir); });
  connect(m_newFolderBtn, &QPushButton::clicked, this, &MainWindow::onNewFolder);
  connect(m_uploadFilesBtn, &QPushButton::clicked, this, &MainWindow::onUploadFiles);
  connect(m_uploadFolderBtn, &QPushButton::clicked, this, &MainWindow::onUploadFolder);
  connect(m_dirDeleteBtn, &QPushButton::clicked, this, &MainWindow::onDeletePaths);
  connect(m_dirTable, &QTableWidget::cellDoubleClicked, this, &MainWindow::onDirEntryActivated);
  connect(m_dirTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::updateUiState);
  connect(m_dirTable, &DropTableWidget::filesDropped, this, [this](const QStringList &paths) {
    if (m_client->isConnected()) uploadPaths(paths, m_currentDir.isEmpty() ? m_fileRoot : m_currentDir);
  });
  connect(m_hostCombo, &QComboBox::editTextChanged, this, [this](const QString &) { updateUiState(); });
  connect(m_fileList, &QListWidget::itemSelectionChanged, this, &MainWindow::updateUiState);
  connect(m_dbTable, &QTableWidget::itemSelectionChanged, this, &MainWindow::updateUiState);
}

void MainWindow::loadSettings() {
  QSettings settings;
  const QStringList hosts = settings.value(QStringLiteral("recentHosts")).toStringList();
  m_hostCombo->clear();
  m_hostCombo->addItems(hosts);
  m_hostCombo->setCurrentText(settings.value(QStringLiteral("host"), hosts.isEmpty() ? QString() : hosts.first()).toString());
  m_portSpin->setValue(settings.value(QStringLiteral("port"), NetInstClient::DefaultPort).toInt());
  m_lastDir = settings.value(QStringLiteral("lastDir")).toString();
  m_appsOnlyCheck->setChecked(settings.value(QStringLiteral("appsOnly"), true).toBool());
  const QByteArray geometry = settings.value(QStringLiteral("geometry")).toByteArray();
  if (!geometry.isEmpty()) restoreGeometry(geometry);
}

void MainWindow::saveSettings() {
  QSettings settings;
  QStringList hosts;
  for (int i = 0; i < m_hostCombo->count() && hosts.size() < MaxRecentHosts; i++) hosts << m_hostCombo->itemText(i);
  settings.setValue(QStringLiteral("recentHosts"), hosts);
  settings.setValue(QStringLiteral("host"), currentHost());
  settings.setValue(QStringLiteral("port"), m_portSpin->value());
  settings.setValue(QStringLiteral("lastDir"), m_lastDir);
  settings.setValue(QStringLiteral("appsOnly"), m_appsOnlyCheck->isChecked());
  settings.setValue(QStringLiteral("geometry"), saveGeometry());
}

void MainWindow::closeEvent(QCloseEvent *event) {
  saveSettings();
  m_client->disconnectFromHost();
  QMainWindow::closeEvent(event);
}

QString MainWindow::currentHost() const {
  return m_hostCombo->currentText().trimmed();
}

quint16 MainWindow::currentPort() const {
  return quint16(m_portSpin->value());
}

void MainWindow::setHost(const QString &host, quint16 port) {
  if (!host.isEmpty()) m_hostCombo->setCurrentText(host);
  if (port) m_portSpin->setValue(port);
}

void MainWindow::rememberHost(const QString &host) {
  if (host.isEmpty()) return;
  const int idx = m_hostCombo->findText(host);
  if (idx >= 0) m_hostCombo->removeItem(idx);
  m_hostCombo->insertItem(0, host);
  while (m_hostCombo->count() > MaxRecentHosts) m_hostCombo->removeItem(m_hostCombo->count() - 1);
  m_hostCombo->setCurrentIndex(0);
}

void MainWindow::log(const QString &message) {
  m_log->appendPlainText(QStringLiteral("%1  %2").arg(QTime::currentTime().toString(QStringLiteral("HH:mm:ss")), message));
}

void MainWindow::updateUiState() {
  const bool connected = m_client->isConnected();
  const bool busy = m_client->isBusy() || m_client->pendingCount() > 0;

  m_connectBtn->setText(connected ? tr("Disconnect") : tr("Connect"));
  m_connectBtn->setEnabled(connected || !currentHost().isEmpty());
  m_hostCombo->setEnabled(!connected);
  m_portSpin->setEnabled(!connected);
  m_installBtn->setEnabled(connected && !busy && m_fileList->count() > 0);
  m_refreshBtn->setEnabled(connected && !busy);
  m_deleteBtn->setEnabled(connected && !busy && !m_dbTable->selectedItems().isEmpty());
  m_removeBtn->setEnabled(!m_fileList->selectedItems().isEmpty());
  m_clearBtn->setEnabled(m_fileList->count() > 0);

  const bool filesOk = connected && !busy;
  m_dirUpBtn->setEnabled(filesOk && !m_currentDir.isEmpty() && m_currentDir != m_fileRoot);
  m_dirGoBtn->setEnabled(filesOk);
  m_dirRefreshBtn->setEnabled(filesOk);
  m_newFolderBtn->setEnabled(filesOk);
  m_uploadFilesBtn->setEnabled(filesOk);
  m_uploadFolderBtn->setEnabled(filesOk);
  m_dirDeleteBtn->setEnabled(filesOk && !m_dirTable->selectedItems().isEmpty());
  m_dirTable->setEnabled(connected);

  if (connected) {
    m_statusLabel->setText(tr("Connected to %1 (%2)").arg(m_client->deviceName(), m_client->peerAddress()));
  } else {
    m_statusLabel->setText(tr("Not connected"));
  }
}

void MainWindow::addFiles(const QStringList &paths) {
  for (const QString &path : paths) {
    if (!isDatabaseFile(path)) {
      log(tr("Skipping %1: not a .prc or .pdb file").arg(QFileInfo(path).fileName()));
      continue;
    }
    bool dup = false;
    for (int i = 0; i < m_fileList->count(); i++) {
      if (m_fileList->item(i)->data(Qt::UserRole).toString() == path) { dup = true; break; }
    }
    if (dup) continue;

    const PrcInfo info = PrcInfo::fromFile(path);
    QListWidgetItem *item = new QListWidgetItem(m_fileList);
    item->setData(Qt::UserRole, path);
    if (info.valid) {
      item->setText(QStringLiteral("%1  (%2, %3 '%4', %5)").arg(QFileInfo(path).fileName(), info.name,
                    info.isApplication() ? tr("application") : tr("database"), info.creatorString(), formatSize(info.size)));
    } else {
      item->setText(QStringLiteral("%1  (%2)").arg(QFileInfo(path).fileName(), tr("not a valid PalmOS database!")));
      item->setForeground(Qt::red);
    }
    item->setToolTip(path);
    m_lastDir = QFileInfo(path).absolutePath();
  }
  updateUiState();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event) {
  if (event->mimeData()->hasUrls()) event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event) {
  QStringList paths;
  const QList<QUrl> urls = event->mimeData()->urls();
  for (const QUrl &url : urls) {
    if (url.isLocalFile()) paths << url.toLocalFile();
  }
  addFiles(paths);
  event->acceptProposedAction();
}

void MainWindow::onConnectClicked() {
  if (m_client->isConnected()) {
    m_client->disconnectFromHost();
    return;
  }
  const QString host = currentHost();
  if (host.isEmpty()) return;
  rememberHost(host);
  m_client->connectToHost(host, currentPort());
  m_connectBtn->setEnabled(false);
}

void MainWindow::onDiscoverClicked() {
  m_discoverBtn->setEnabled(false);
  m_discoverBtn->setText(tr("Searching..."));
  log(tr("Looking for devices on port %1...").arg(m_portSpin->value()));
  m_discovery->start(currentPort());
}

void MainWindow::onAddFiles() {
  const QStringList files = QFileDialog::getOpenFileNames(this, tr("Add PalmOS files"), m_lastDir,
                                                          tr("PalmOS databases (*.prc *.pdb);;All files (*)"));
  addFiles(files);
}

void MainWindow::onRemoveFiles() {
  const QList<QListWidgetItem *> items = m_fileList->selectedItems();
  for (QListWidgetItem *item : items) delete item;
  updateUiState();
}

void MainWindow::onInstall() {
  if (!m_client->isConnected() || m_fileList->count() == 0) return;

  m_installTotal = 0;
  m_installDone = 0;
  m_installFailed = 0;
  m_installing = true;
  for (int i = 0; i < m_fileList->count(); i++) {
    QListWidgetItem *item = m_fileList->item(i);
    item->setIcon(QIcon());
    m_client->put(item->data(Qt::UserRole).toString());
    m_installTotal++;
  }
  m_totalProgress->setRange(0, m_installTotal);
  m_totalProgress->setValue(0);
  m_fileProgress->setValue(0);
  m_refreshAfterQueue = true;
  updateUiState();
}

void MainWindow::onRefresh() {
  if (!m_client->isConnected()) return;
  m_client->list();
  updateUiState();
}

void MainWindow::onDeleteSelected() {
  QStringList names;
  const QList<QTableWidgetItem *> items = m_dbTable->selectedItems();
  for (QTableWidgetItem *item : items) {
    if (item->column() == 0 && !names.contains(item->text())) names << item->text();
  }
  if (names.isEmpty()) return;

  const QString question = names.size() == 1
    ? tr("Delete \"%1\" from the device?").arg(names.first())
    : tr("Delete %1 databases from the device?").arg(names.size());
  if (QMessageBox::question(this, tr("Delete"), question) != QMessageBox::Yes) return;

  for (const QString &name : names) m_client->remove(name);
  m_refreshAfterQueue = true;
  updateUiState();
}

void MainWindow::onConnected(const QString &deviceName, quint32 maxSize) {
  Q_UNUSED(deviceName);
  Q_UNUSED(maxSize);
  updateUiState();
  onRefresh();
  browseDir(QString());   // the file root
}

void MainWindow::onDisconnected() {
  m_installing = false;
  m_dbTable->setRowCount(0);
  m_entries.clear();
  m_dirTable->setRowCount(0);
  m_currentDir.clear();
  m_dirPathEdit->clear();
  m_progressLabel->clear();
  updateUiState();
}

void MainWindow::onListReceived(const QList<DbEntry> &entries) {
  m_entries = entries;
  const bool appsOnly = m_appsOnlyCheck->isChecked();

  m_dbTable->setSortingEnabled(false);
  m_dbTable->setRowCount(0);
  for (const DbEntry &e : entries) {
    if (appsOnly && !e.isApplication()) continue;
    const int row = m_dbTable->rowCount();
    m_dbTable->insertRow(row);
    m_dbTable->setItem(row, 0, new QTableWidgetItem(e.name));
    m_dbTable->setItem(row, 1, new QTableWidgetItem(e.typeString()));
    m_dbTable->setItem(row, 2, new QTableWidgetItem(e.creatorString()));
    m_dbTable->setItem(row, 3, new QTableWidgetItem(QString::number(e.version)));
    QTableWidgetItem *size = new QTableWidgetItem(formatSize(e.size));
    size->setData(Qt::UserRole, e.size);
    size->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_dbTable->setItem(row, 4, size);
  }
  m_dbTable->setSortingEnabled(true);
  m_dbTable->sortItems(0);
  updateUiState();
}

void MainWindow::onPutProgress(const QString &path, qint64 sent, qint64 total) {
  m_fileProgress->setValue(total > 0 ? int(sent * 100 / total) : 0);
  m_progressLabel->setText(tr("Sending %1: %2 of %3").arg(QFileInfo(path).fileName(), formatSize(sent), formatSize(total)));
}

void MainWindow::onPutFinished(const QString &path, bool ok, const QString &message) {
  Q_UNUSED(message);
  if (ok) m_installDone++; else m_installFailed++;
  m_totalProgress->setValue(m_installDone + m_installFailed);

  for (int i = 0; i < m_fileList->count(); i++) {
    QListWidgetItem *item = m_fileList->item(i);
    if (item->data(Qt::UserRole).toString() == path) {
      item->setForeground(ok ? QBrush(QColor(0, 128, 0)) : QBrush(Qt::red));
      break;
    }
  }
}

void MainWindow::onQueueDrained() {
  if (m_installing) {
    m_installing = false;
    m_progressLabel->setText(m_installFailed == 0
      ? tr("Installed %1 file(s)").arg(m_installDone)
      : tr("Installed %1 file(s), %2 failed").arg(m_installDone).arg(m_installFailed));
    m_fileProgress->setValue(m_installFailed == 0 && m_installDone > 0 ? 100 : m_fileProgress->value());
  }
  if (m_refreshAfterQueue && m_client->isConnected()) {
    m_refreshAfterQueue = false;
    m_client->list();
  }
  if (m_refreshDirAfterQueue && m_client->isConnected()) {
    m_refreshDirAfterQueue = false;
    m_client->listDir(m_currentDir);
  }
  updateUiState();
}

/*
 * Files tab
 */

QString MainWindow::remoteJoin(const QString &dir, const QString &name) const {
  return dir.endsWith('/') ? dir + name : dir + '/' + name;
}

void MainWindow::browseDir(const QString &path) {
  if (!m_client->isConnected()) return;
  m_client->listDir(path);
  updateUiState();
}

void MainWindow::onDirListed(const QString &path, const QList<FileEntry> &entries) {
  m_currentDir = path;
  if (m_fileRoot.isEmpty() || path.length() < m_fileRoot.length()) m_fileRoot = path;
  m_dirPathEdit->setText(path);

  m_dirTable->setSortingEnabled(false);
  m_dirTable->setRowCount(0);
  for (const FileEntry &e : entries) {
    const int row = m_dirTable->rowCount();
    m_dirTable->insertRow(row);
    QTableWidgetItem *name = new QTableWidgetItem(e.name);
    name->setData(Qt::UserRole, e.isDir);
    m_dirTable->setItem(row, 0, name);
    m_dirTable->setItem(row, 1, new QTableWidgetItem(e.isDir ? tr("Folder") : tr("File")));
    QTableWidgetItem *size = new QTableWidgetItem(e.isDir ? QString() : formatSize(e.size));
    size->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_dirTable->setItem(row, 2, size);
  }
  m_dirTable->setSortingEnabled(true);
  m_dirTable->sortItems(1);   // folders first
  updateUiState();
}

void MainWindow::onDirUp() {
  if (m_currentDir.isEmpty() || m_currentDir == m_fileRoot) return;
  const int slash = m_currentDir.lastIndexOf('/');
  browseDir(slash > 0 ? m_currentDir.left(slash) : m_fileRoot);
}

void MainWindow::onDirEntryActivated(int row, int column) {
  Q_UNUSED(column);
  QTableWidgetItem *item = m_dirTable->item(row, 0);
  if (item && item->data(Qt::UserRole).toBool()) {
    browseDir(remoteJoin(m_currentDir, item->text()));
  }
}

void MainWindow::onNewFolder() {
  if (!m_client->isConnected() || m_currentDir.isEmpty()) return;
  bool ok = false;
  const QString name = QInputDialog::getText(this, tr("New Folder"), tr("Name of the new folder in %1:").arg(m_currentDir),
                                             QLineEdit::Normal, QString(), &ok).trimmed();
  if (!ok || name.isEmpty()) return;
  if (name.contains('/') || name == QLatin1String(".") || name == QLatin1String("..")) {
    QMessageBox::warning(this, tr("New Folder"), tr("Invalid folder name."));
    return;
  }
  m_client->makeDir(remoteJoin(m_currentDir, name));
  m_refreshDirAfterQueue = true;
  updateUiState();
}

void MainWindow::uploadPaths(const QStringList &localPaths, const QString &remoteDir) {
  int files = 0;
  for (const QString &local : localPaths) {
    const QFileInfo fi(local);
    if (fi.isDir()) {
      const QString remoteBase = remoteJoin(remoteDir, fi.fileName());
      m_client->makeDir(remoteBase);
      QDirIterator it(local, QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
      const QDir base(local);
      while (it.hasNext()) {
        const QString path = it.next();
        const QFileInfo sub(path);
        const QString rel = base.relativeFilePath(path);
        if (sub.isDir()) {
          m_client->makeDir(remoteJoin(remoteBase, rel));
        } else if (sub.isFile()) {
          m_client->uploadFile(path, remoteJoin(remoteBase, rel));
          files++;
        }
      }
    } else if (fi.isFile()) {
      m_client->uploadFile(local, remoteJoin(remoteDir, fi.fileName()));
      files++;
    }
    m_lastDir = fi.absolutePath();
  }
  if (files > 0) {
    m_totalProgress->setRange(0, files);
    m_totalProgress->setValue(0);
    m_installTotal = files;
    m_installDone = 0;
    m_installFailed = 0;
    m_installing = true;
  }
  m_refreshDirAfterQueue = true;
  updateUiState();
}

void MainWindow::onUploadFiles() {
  if (!m_client->isConnected() || m_currentDir.isEmpty()) return;
  const QStringList files = QFileDialog::getOpenFileNames(this, tr("Upload files to %1").arg(m_currentDir), m_lastDir, tr("All files (*)"));
  if (!files.isEmpty()) uploadPaths(files, m_currentDir);
}

void MainWindow::onUploadFolder() {
  if (!m_client->isConnected() || m_currentDir.isEmpty()) return;
  const QString dir = QFileDialog::getExistingDirectory(this, tr("Upload folder to %1").arg(m_currentDir), m_lastDir);
  if (!dir.isEmpty()) uploadPaths({dir}, m_currentDir);
}

void MainWindow::onDeletePaths() {
  QStringList names;
  const QList<QTableWidgetItem *> items = m_dirTable->selectedItems();
  for (QTableWidgetItem *item : items) {
    if (item->column() == 0 && !names.contains(item->text())) names << item->text();
  }
  if (names.isEmpty()) return;

  const QString question = names.size() == 1
    ? tr("Delete \"%1\" from %2?").arg(names.first(), m_currentDir)
    : tr("Delete %1 entries from %2?").arg(names.size()).arg(m_currentDir);
  if (QMessageBox::question(this, tr("Delete"), question + '\n' + tr("Folders must be empty.")) != QMessageBox::Yes) return;

  for (const QString &name : names) m_client->removePath(remoteJoin(m_currentDir, name));
  m_refreshDirAfterQueue = true;
  updateUiState();
}
