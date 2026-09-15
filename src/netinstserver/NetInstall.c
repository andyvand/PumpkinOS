/*
 * Net Install - network application installer for PumpkinOS (server side).
 *
 * Listens on a TCP port for the netinstclient desktop tool, receives .prc /
 * .pdb files and installs them: the file is staged in a hidden directory of
 * the card volume and imported with VFSImportDatabaseFromFile (the routine
 * the Launcher uses for /app_install: it replaces an older version and
 * registers new applications), then the Launcher is asked to refresh
 * (pumpkin_send_deploy).  It also answers LIST (the installed databases) and
 * DEL requests.
 *
 * The protocol engine lives in netinst.c and only uses libpit; this file is
 * the PalmOS UI plus the storage callbacks.  Everything runs in the main
 * thread: the engine is polled from the event loop with a short time slice.
 */

#ifdef ESP_PLATFORM
#include "esp32.h"
#endif

#include <PalmOS.h>
#include <VFSMgr.h>

#include "sys.h"
#include "pumpkin.h"
#include "xalloc.h"
#include "debug.h"

#include "netinst.h"
#include "resource.h"

#define TAG            "NetInstall"

#define PREFS_ID       1
#define PREFS_VERSION  1

#define VOLUME         1                 /* the card volume (vfs/app_card) */
#define INCOMING_DIR   "/.netinst"       /* staging directory for incoming databases */
#define FILE_ROOT      "/PALM/Programs"  /* directory the file commands may touch */
#define PATH_LEN       (sizeof(INCOMING_DIR) + NETINST_FILE_LEN + 2)

#ifdef ESP32
#define MAX_FILE_SIZE  (8 * 1024 * 1024)
#else
#define MAX_FILE_SIZE  (64 * 1024 * 1024)
#endif

#define LOG_LINES      7
#define STATUS_LEN     48
#define POLL_BUDGET_US 20000

typedef struct {
  UInt16 port;
  UInt8 autoStart;
  UInt8 pad;
} netinstall_prefs_t;

typedef struct {
  netinstall_prefs_t prefs;
  netinst_t *server;
  Boolean stop;

  /* file being received */
  FileRef file;
  Boolean fileOpen;
  Boolean raw;                 /* plain file upload (FPUT), not a database */
  char uploadPath[NETINST_PATH_LEN];
  char tmpPath[PATH_LEN];
  char dbName[NETINST_NAME_LEN];
  UInt32 installed;

  /* activity log (ring buffer, newest last) */
  char log[LOG_LINES][NETINST_LOG_LEN];
  int nlog, logFirst;
  Boolean logDirty;

  /* status line */
  char status[STATUS_LEN];
  char shownStatus[STATUS_LEN];
  RectangleType statusRect, logRect;
  Boolean formOpen;
} netinstall_app_t;

static Boolean ApplicationHandleEvent(EventPtr event);

/*
 * Field helpers
 */

static void setField(FormType *frm, UInt16 fieldId, const char *s) {
  FieldType *fld;
  UInt16 len;

  fld = (FieldType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, fieldId));
  if (fld == NULL) return;
  len = FldGetTextLength(fld);
  if (len) FldDelete(fld, 0, len);
  if (s && s[0]) FldInsert(fld, s, StrLen(s));
}

static UInt32 getFieldNum(FormType *frm, UInt16 fieldId) {
  FieldType *fld;
  char *s;

  fld = (FieldType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, fieldId));
  if (fld == NULL) return 0;
  s = FldGetTextPtr(fld);
  return (s && s[0]) ? (UInt32)StrAToI(s) : 0;
}

static void showObject(FormType *frm, UInt16 id, Boolean show) {
  UInt16 index = FrmGetObjectIndex(frm, id);

  if (show) {
    FrmShowObject(frm, index);
  } else {
    FrmHideObject(frm, index);
  }
}

/*
 * Log and status drawing
 */

static void drawLog(netinstall_app_t *a) {
  FormType *frm = FrmGetActiveForm();
  RectangleType r;
  FontID old;
  Coord y;
  int i, k, h;

  a->logDirty = false;
  if (!a->formOpen || frm == NULL || FrmGetFormId(frm) != MainForm) return;

  r = a->logRect;
  WinEraseRectangle(&r, 0);
  WinDrawRectangleFrame(rectangleFrame, &r);

  old = FntSetFont(stdFont);
  h = FntLineHeight();
  y = r.topLeft.y + 2;

  for (i = 0; i < a->nlog && y + h <= r.topLeft.y + r.extent.y - 1; i++) {
    k = (a->logFirst + i) % LOG_LINES;
    WinDrawTruncChars(a->log[k], StrLen(a->log[k]), r.topLeft.x + 3, y, r.extent.x - 6);
    y += h;
  }
  FntSetFont(old);
}

static void appendLog(netinstall_app_t *a, const char *msg) {
  int k;

  if (a->nlog < LOG_LINES) {
    k = (a->logFirst + a->nlog) % LOG_LINES;
    a->nlog++;
  } else {
    k = a->logFirst;
    a->logFirst = (a->logFirst + 1) % LOG_LINES;
  }
  StrNCopy(a->log[k], msg, NETINST_LOG_LEN - 1);
  a->log[k][NETINST_LOG_LEN - 1] = 0;
  a->logDirty = true;
}

static void drawStatus(netinstall_app_t *a, Boolean force) {
  FormType *frm = FrmGetActiveForm();
  RectangleType r;
  FontID old;
  char *s;

  if (!a->formOpen || frm == NULL || FrmGetFormId(frm) != MainForm) return;
  if (!force && StrCompare(a->status, a->shownStatus) == 0) return;

  StrCopy(a->shownStatus, a->status);
  r = a->statusRect;
  WinEraseRectangle(&r, 0);

  old = FntSetFont(boldFont);
  s = a->status;
  WinDrawTruncChars(s, StrLen(s), r.topLeft.x, r.topLeft.y + 1, r.extent.x);
  FntSetFont(old);
}

static void updateStatus(netinstall_app_t *a) {
  char name[NETINST_FILE_LEN];
  UInt32 received, total;

  if (!netinst_running(a->server)) {
    StrCopy(a->status, "Stopped");
  } else if (netinst_transfer(a->server, name, sizeof(name), &received, &total)) {
    StrNPrintF(a->status, sizeof(a->status) - 1, "%s %lu%%", name, total ? (received * 100UL) / total : 0UL);
  } else if (netinst_connected(a->server)) {
    StrNPrintF(a->status, sizeof(a->status) - 1, "Client %s", netinst_client(a->server));
  } else {
    StrNPrintF(a->status, sizeof(a->status) - 1, "Listening on port %u", netinst_port(a->server));
  }
  a->status[sizeof(a->status) - 1] = 0;
  drawStatus(a, false);
}

static void updateButtons(netinstall_app_t *a) {
  FormType *frm = FrmGetActiveForm();
  Boolean running;

  if (!a->formOpen || frm == NULL || FrmGetFormId(frm) != MainForm) return;
  running = netinst_running(a->server);
  showObject(frm, startBtn, !running);
  showObject(frm, stopBtn, running);
}

/*
 * Storage callbacks (called by the engine from netinst_poll, main thread)
 */

static void closeIncoming(netinstall_app_t *a) {
  if (a->fileOpen) {
    VFSFileClose(a->file);
    a->fileOpen = false;
  }
}

/* create `path` and every missing parent; 0 or -1 */
static int ensureDir(const char *path, char *err, int errlen) {
  char partial[NETINST_PATH_LEN];
  FileRef ref;
  Err e;
  int i;

  for (i = 1; ; i++) {
    if (path[i] == '/' || path[i] == 0) {
      if (i >= (int)sizeof(partial)) {
        sys_snprintf(err, errlen, "path too long");
        return -1;
      }
      MemMove(partial, path, i);
      partial[i] = 0;
      if (VFSFileOpen(VOLUME, partial, vfsModeRead, &ref) == errNone) {
        VFSFileClose(ref);
      } else if ((e = VFSDirCreate(VOLUME, partial)) != errNone && e != vfsErrFileAlreadyExists) {
        sys_snprintf(err, errlen, "cannot create directory %s (%d)", partial, e);
        return -1;
      }
      if (path[i] == 0) break;
    }
  }

  return 0;
}

/* open a file of the card volume for writing, truncating it */
static int openForWrite(netinstall_app_t *a, const char *path, char *err, int errlen) {
  Err e;

  e = VFSFileCreate(VOLUME, path);
  if (e != errNone && e != vfsErrFileAlreadyExists) {
    sys_snprintf(err, errlen, "cannot create file (%d)", e);
    return -1;
  }
  /* vfsModeCreate truncates an existing file */
  if ((e = VFSFileOpen(VOLUME, path, vfsModeWrite | vfsModeCreate, &a->file)) != errNone) {
    sys_snprintf(err, errlen, "cannot open file (%d)", e);
    return -1;
  }
  a->fileOpen = true;

  return 0;
}

static int cbBeginFile(void *data, const char *filename, const char *dbname, uint32_t size, char *err, int errlen) {
  netinstall_app_t *a = (netinstall_app_t *)data;
  Err e;
  FileRef dir;
  LocalID dbID;
  UInt16 attr = 0;
  UInt32 type = 0, creator = 0;

  closeIncoming(a);
  a->raw = false;

  /* a database that is open (a running application) cannot be replaced;
     PumpkinOS does not report dmHdrAttrOpen, so at least catch ourselves */
  if ((dbID = DmFindDatabase(0, (char *)dbname)) != 0 &&
      DmDatabaseInfo(0, dbID, NULL, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &type, &creator) == errNone) {
    if (attr & dmHdrAttrOpen) {
      sys_snprintf(err, errlen, "\"%s\" is in use, close it on the device first", dbname);
      return -1;
    }
    if (type == sysFileTApplication && creator == pumpkin_get_app_creator()) {
      sys_snprintf(err, errlen, "Net Install cannot replace itself while it is running");
      return -1;
    }
  }
  StrNCopy(a->dbName, dbname, sizeof(a->dbName) - 1);
  a->dbName[sizeof(a->dbName) - 1] = 0;

  if (VFSFileOpen(VOLUME, INCOMING_DIR, vfsModeRead, &dir) == errNone) {
    VFSFileClose(dir);
  } else if ((e = VFSDirCreate(VOLUME, INCOMING_DIR)) != errNone) {
    sys_snprintf(err, errlen, "cannot create %s (%d)", INCOMING_DIR, e);
    return -1;
  }

  StrNPrintF(a->tmpPath, sizeof(a->tmpPath) - 1, "%s/%s", INCOMING_DIR, filename);

  if (openForWrite(a, a->tmpPath, err, errlen) != 0) {
    VFSFileDelete(VOLUME, a->tmpPath);
    return -1;
  }

  return 0;
}

/* FPUT: plain file inside FILE_ROOT */
static int cbBeginUpload(void *data, const char *path, uint32_t size, char *err, int errlen) {
  netinstall_app_t *a = (netinstall_app_t *)data;
  char parent[NETINST_PATH_LEN];
  int i, n;

  closeIncoming(a);
  a->raw = true;
  StrNCopy(a->uploadPath, path, sizeof(a->uploadPath) - 1);
  a->uploadPath[sizeof(a->uploadPath) - 1] = 0;

  /* create the parent directories */
  n = StrLen(path);
  for (i = n - 1; i > 0 && path[i] != '/'; i--);
  if (i > 0) {
    MemMove(parent, path, i);
    parent[i] = 0;
    if (ensureDir(parent, err, errlen) != 0) return -1;
  }

  return openForWrite(a, path, err, errlen);
}

static int cbWriteFile(void *data, const uint8_t *buf, int len, char *err, int errlen) {
  netinstall_app_t *a = (netinstall_app_t *)data;
  UInt32 written = 0;
  Err e;

  if (!a->fileOpen) {
    sys_snprintf(err, errlen, "file not open");
    return -1;
  }
  if ((e = VFSFileWrite(a->file, len, buf, &written)) != errNone || written != (UInt32)len) {
    sys_snprintf(err, errlen, "write failed (%d)", e);
    return -1;
  }

  return 0;
}

static int cbEndFile(void *data, int ok, char *err, int errlen) {
  netinstall_app_t *a = (netinstall_app_t *)data;
  UInt16 cardNo = 0, attr = 0;
  LocalID dbID = 0;
  Err e;

  closeIncoming(a);

  if (a->raw) {
    /* plain upload: the file is already in place */
    a->raw = false;
    if (!ok) VFSFileDelete(VOLUME, a->uploadPath);
    return 0;
  }

  if (!ok) {
    VFSFileDelete(VOLUME, a->tmpPath);
    return 0;
  }

  /* replaces an existing version and registers new applications, exactly
     like the deployment of /app_install at startup */
  e = VFSImportDatabaseFromFile(VOLUME, a->tmpPath, &cardNo, &dbID);
  VFSFileDelete(VOLUME, a->tmpPath);
  if (e != errNone) {
    if ((dbID = DmFindDatabase(0, a->dbName)) != 0 &&
        DmDatabaseInfo(0, dbID, NULL, &attr, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) == errNone &&
        (attr & dmHdrAttrOpen)) {
      sys_snprintf(err, errlen, "\"%s\" is in use, close it on the device first", a->dbName);
    } else {
      sys_snprintf(err, errlen, "install failed (%d)", e);
    }
    return -1;
  }

  /* make the Launcher rescan the installed applications */
  pumpkin_send_deploy();
  a->installed++;

  return 0;
}

static int cbList(void *data, netinst_list_f cb, void *cbdata) {
  netinst_db_t db;
  LocalID dbID;
  UInt16 n, i, attr, version;
  UInt32 type, creator, total;
  int count = 0;

  n = DmNumDatabases(0);
  for (i = 0; i < n; i++) {
    if ((dbID = DmGetDatabase(0, i)) == 0) continue;
    MemSet(&db, sizeof(db), 0);
    attr = version = 0;
    type = creator = 0;
    if (DmDatabaseInfo(0, dbID, db.name, &attr, &version, NULL, NULL, NULL, NULL, NULL, NULL, &type, &creator) != errNone) continue;
    if (db.name[0] == 0) continue;
    total = 0;
    DmDatabaseSize(0, dbID, NULL, &total, NULL);
    db.type = type;
    db.creator = creator;
    db.attributes = attr;
    db.version = version;
    db.size = total;
    cb(cbdata, &db);
    count++;
  }

  return count;
}

static int cbRemove(void *data, const char *name, char *err, int errlen) {
  LocalID dbID;
  UInt32 type = 0, creator = 0;
  Err e;

  if ((dbID = DmFindDatabase(0, (char *)name)) == 0) {
    sys_snprintf(err, errlen, "database not found");
    return -1;
  }
  DmDatabaseInfo(0, dbID, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &type, &creator);
  if (type == sysFileTApplication && (creator == pumpkin_get_app_creator() || creator == 'lnch')) {
    sys_snprintf(err, errlen, "refusing to delete a system application");
    return -1;
  }
  if ((e = DmDeleteDatabase(0, dbID)) != errNone) {
    sys_snprintf(err, errlen, e == dmErrDatabaseOpen ? "database is in use" : "delete failed (%d)", e);
    return -1;
  }

  /* make the Launcher rescan */
  pumpkin_send_deploy();
  return 0;
}

static int cbMkdir(void *data, const char *path, char *err, int errlen) {
  return ensureDir(path, err, errlen);
}

static int cbListDir(void *data, const char *path, netinst_dir_f cb, void *cbdata, char *err, int errlen) {
  FileRef dirRef, fileRef;
  FileInfoType info;
  UInt32 iterator, size;
  char name[NETINST_FILE_LEN * 2], full[NETINST_PATH_LEN + NETINST_FILE_LEN * 2];
  Err e;
  int count = 0;

  if ((e = VFSFileOpen(VOLUME, path, vfsModeRead, &dirRef)) != errNone) {
    sys_snprintf(err, errlen, "directory not found (%d)", e);
    return -1;
  }

  iterator = vfsIteratorStart;
  while (iterator != vfsIteratorStop) {
    MemSet(&info, sizeof(info), 0);
    MemSet(name, sizeof(name), 0);
    info.nameP = name;
    info.nameBufLen = sizeof(name) - 1;
    if (VFSDirEntryEnumerate(dirRef, &iterator, &info) != errNone) break;
    if (name[0] == 0) continue;

    size = 0;
    if (!(info.attributes & vfsFileAttrDirectory)) {
      StrNPrintF(full, sizeof(full) - 1, "%s/%s", path, name);
      if (VFSFileOpen(VOLUME, full, vfsModeRead, &fileRef) == errNone) {
        VFSFileSize(fileRef, &size);
        VFSFileClose(fileRef);
      }
    }
    cb(cbdata, name, (info.attributes & vfsFileAttrDirectory) ? 1 : 0, size);
    count++;
  }
  VFSFileClose(dirRef);

  return count;
}

static int cbRemovePath(void *data, const char *path, char *err, int errlen) {
  FileRef ref;
  Err e;

  if (VFSFileOpen(VOLUME, path, vfsModeRead, &ref) != errNone) {
    sys_snprintf(err, errlen, "not found");
    return -1;
  }
  VFSFileClose(ref);

  /* VFSFileDelete removes files and empty directories */
  if ((e = VFSFileDelete(VOLUME, path)) != errNone) {
    sys_snprintf(err, errlen, "cannot delete (directory not empty?) (%d)", e);
    return -1;
  }

  return 0;
}

static void cbLog(void *data, const char *msg) {
  appendLog((netinstall_app_t *)data, msg);
}

/*
 * Server control
 */

static Boolean startServer(netinstall_app_t *a, UInt16 port) {
  netinst_ops_t ops;
  char devname[NETINST_DEVNAME_LEN];

  if (a->server) {
    netinst_destroy(a->server);
    a->server = NULL;
  }

  MemSet(&ops, sizeof(ops), 0);
  ops.data = a;
  ops.begin_file = cbBeginFile;
  ops.write_file = cbWriteFile;
  ops.end_file = cbEndFile;
  ops.list = cbList;
  ops.remove = cbRemove;
  ops.begin_upload = cbBeginUpload;
  ops.mkdir = cbMkdir;
  ops.list_dir = cbListDir;
  ops.remove_path = cbRemovePath;
  ops.log = cbLog;

  StrNPrintF(devname, sizeof(devname) - 1, "PumpkinOS %s", SYSTEM_OS);
  devname[sizeof(devname) - 1] = 0;

  if ((a->server = netinst_create(&ops, port, MAX_FILE_SIZE, devname, FILE_ROOT)) == NULL) {
    appendLog(a, "out of memory");
    return false;
  }

  return netinst_start(a->server) == 0;
}

static void stopServer(netinstall_app_t *a) {
  if (a->server) netinst_stop(a->server);
}

/*
 * Forms
 */

static void MainFormInit(netinstall_app_t *a, FormType *frm) {
  ControlType *ctl;
  UInt16 index;
  char buf[8];

  StrPrintF(buf, "%u", a->prefs.port);
  setField(frm, portFld, buf);

  ctl = (ControlType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, autoChk));
  if (ctl) CtlSetValue(ctl, a->prefs.autoStart ? 1 : 0);

  index = FrmGetObjectIndex(frm, statusGad);
  FrmGetObjectBounds(frm, index, &a->statusRect);
  index = FrmGetObjectIndex(frm, logGad);
  FrmGetObjectBounds(frm, index, &a->logRect);
}

static Boolean MainFormHandleEvent(EventPtr event) {
  netinstall_app_t *a = (netinstall_app_t *)pumpkin_get_data();
  FormType *frm;
  ControlType *ctl;
  UInt32 port;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      MainFormInit(a, frm);
      a->formOpen = true;
      updateButtons(a);
      FrmDrawForm(frm);
      updateStatus(a);
      drawStatus(a, true);
      drawLog(a);
      handled = true;
      break;

    case frmUpdateEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      drawStatus(a, true);
      drawLog(a);
      handled = true;
      break;

    case ctlSelectEvent:
      frm = FrmGetActiveForm();
      switch (event->data.ctlSelect.controlID) {
        case startBtn:
          port = getFieldNum(frm, portFld);
          if (port == 0 || port > 65535) {
            FrmCustomAlert(ErrorAlert, "The port must be a number between 1 and 65535.", "", "");
          } else {
            a->prefs.port = (UInt16)port;
            if (!startServer(a, a->prefs.port)) {
              FrmCustomAlert(ErrorAlert, "Could not listen on that port. It may be in use by another program.", "", "");
            }
            updateButtons(a);
            updateStatus(a);
            drawLog(a);
          }
          handled = true;
          break;
        case stopBtn:
          stopServer(a);
          updateButtons(a);
          updateStatus(a);
          drawLog(a);
          handled = true;
          break;
        case autoChk:
          ctl = (ControlType *)FrmGetObjectPtr(frm, FrmGetObjectIndex(frm, autoChk));
          a->prefs.autoStart = ctl && CtlGetValue(ctl) ? 1 : 0;
          handled = true;
          break;
      }
      break;

    case menuEvent:
      switch (event->data.menu.itemID) {
        case clearLogCmd:
          a->nlog = 0;
          a->logFirst = 0;
          drawLog(a);
          break;
        case aboutCmd:
          AbtShowAbout(pumpkin_get_app_creator());
          break;
      }
      handled = true;
      break;

    default:
      break;
  }

  return handled;
}

static Boolean ApplicationHandleEvent(EventPtr event) {
  FormType *frm;
  Boolean handled = false;

  if (event->eType == frmLoadEvent) {
    frm = FrmInitForm(event->data.frmLoad.formID);
    FrmSetActiveForm(frm);
    if (event->data.frmLoad.formID == MainForm) {
      FrmSetEventHandler(frm, MainFormHandleEvent);
    }
    handled = true;
  }

  return handled;
}

static void EventLoop(netinstall_app_t *a) {
  EventType event;
  Err err;
  Int32 tps, wait;
  int busy = 0;

  tps = SysTicksPerSecond();
  if (tps <= 0) tps = 100;

  do {
    if (!netinst_running(a->server)) {
      wait = evtWaitForever;
    } else if (busy) {
      wait = 0;
    } else if (netinst_connected(a->server)) {
      wait = tps / 100 > 0 ? tps / 100 : 1;
    } else {
      wait = tps / 10 > 0 ? tps / 10 : 1;
    }

    EvtGetEvent(&event, wait);
    if (!SysHandleEvent(&event)) {
      if (!MenuHandleEvent(NULL, &event, &err)) {
        if (!ApplicationHandleEvent(&event)) {
          FrmDispatchEvent(&event);
        }
      }
    }

    busy = 0;
    if (netinst_running(a->server)) {
      busy = netinst_poll(a->server, POLL_BUDGET_US) > 0;
      updateStatus(a);
      if (a->logDirty) drawLog(a);
    }
  } while (event.eType != appStopEvent && !a->stop);
}

static void StartApplication(netinstall_app_t *a) {
  UInt32 creator;
  UInt16 size;

  creator = pumpkin_get_app_creator();
  size = sizeof(netinstall_prefs_t);
  if (PrefGetAppPreferences(creator, PREFS_ID, &a->prefs, &size, true) == noPreferenceFound || size != sizeof(netinstall_prefs_t)) {
    MemSet(&a->prefs, sizeof(a->prefs), 0);
    a->prefs.port = NETINST_DEFAULT_PORT;
    a->prefs.autoStart = 1;
  }
  if (a->prefs.port == 0) a->prefs.port = NETINST_DEFAULT_PORT;

  if (a->prefs.autoStart) {
    startServer(a, a->prefs.port);
  }

  FrmGotoForm(MainForm);
}

static void StopApplication(netinstall_app_t *a) {
  UInt32 creator;

  a->formOpen = false;
  if (a->server) {
    netinst_destroy(a->server);   /* also discards a partial file */
    a->server = NULL;
  }
  closeIncoming(a);

  FrmCloseAllForms();

  creator = pumpkin_get_app_creator();
  PrefSetAppPreferences(creator, PREFS_ID, PREFS_VERSION, &a->prefs, sizeof(netinstall_prefs_t), true);
}

#ifdef ESP32
UInt32 NetInstPilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
#else
UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
#endif
{
  netinstall_app_t *a;

  if (cmd == sysAppLaunchCmdNormalLaunch) {
    if ((a = xcalloc(1, sizeof(netinstall_app_t))) == NULL) return 0;
    pumpkin_set_data(a);

    StartApplication(a);
    EventLoop(a);
    StopApplication(a);

    pumpkin_set_data(NULL);
    xfree(a);
  }

  return 0;
}
