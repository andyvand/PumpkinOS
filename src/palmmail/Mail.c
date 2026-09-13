/*
 * Mail - an IMAP/SMTP e-mail client for PumpkinOS.
 *
 * Messages are read from an IMAP4rev1 mailbox and sent through SMTP.
 * Connections are secured with the PumpkinOS secure provider (liblopenssl,
 * libls2n or liblmbedtls), either as implicit TLS or through STARTTLS.
 * The protocol code lives in mailnet.c and runs in a worker thread while a
 * small modal dialog keeps the user interface alive.
 */

#ifdef ESP_PLATFORM
#include "esp32.h"
#endif

#include <PalmOS.h>

#include "sys.h"
#include "thread.h"
#include "mutex.h"
#include "xalloc.h"
#include "pumpkin.h"
#include "debug.h"

#include "mailnet.h"
#include "resource.h"

#define TAG              "Mail"
#define PREFS_ID         1
#define PREFS_VERSION    1
#define DEFAULT_FETCH    30
#define MAX_FETCH        200
#define NET_TIMEOUT      30            /* seconds */
#ifdef ESP32
#define MESSAGE_LIMIT    (48 * 1024)   /* bytes of a message that are downloaded */
#else
#define MESSAGE_LIMIT    (128 * 1024)
#endif
#define BODY_MAX         30000         /* characters shown in the message field */
#define QUOTE_MAX        6000          /* quoted text in replies and forwards */
#define COMPOSE_MAX      8000
#define CANCEL_GRACE     15            /* tenths of a second to wait for a cancelled job */
#define FROM_WIDTH       60            /* pixels for the sender column in the list */
#define LABEL_WIDTH      32            /* pixels for the header labels in the view form */

typedef struct {
  mail_account_t account;
  char folder[MAIL_FOLDER_LEN];
  UInt16 configured;
} mail_prefs_t;

enum { jobHeaders = 1, jobMessage, jobDelete, jobFolders, jobSend };
enum { composeNew = 0, composeReply, composeForward };

/* one network operation, executed in a worker thread */
typedef struct {
  int type;
  mail_ctx_t ctx;               /* used directly by SMTP; copied into the IMAP session */
  mail_session_t *session;      /* IMAP session borrowed from the application */
  mail_account_t account;
  char folder[MAIL_FOLDER_LEN];
  uint32_t uid;
  int max;
  char *rcpts;
  char *data;
  int datalen;

  int result;
  mail_header_t *headers;
  int nheaders, total;
  mail_message_t msg;
  mail_folder_t *folders;
  int nfolders;

  mutex_t *mutex;
  volatile int done;
  volatile int cancel;
  volatile int abandoned;
  int status_changed;
  char status[MAIL_STATUS_LEN];
} mail_job_t;

typedef struct {
  mail_prefs_t prefs;
  mail_session_t *session;
  mutex_t *mutex;
  Boolean stop;
  Boolean promptedAccount;
  Boolean fetchOnOpen;

  /* message list */
  mail_header_t *headers;
  int nheaders, total;
  char title[40];

  /* folders */
  mail_folder_t *folders;
  int nfolders;
  char **folderNames;
  int nfolderNames;

  /* message being viewed */
  int current;
  uint32_t msgUid;
  mail_message_t msg;

  /* compose */
  int composeMode;
  UInt16 composeReturn;
  char composeTitle[16];
  char composeTo[256];
  char composeCc[256];
  char composeSubject[MAIL_SUBJECT_LEN * 2];
  char inReplyTo[128];
  char *composeBody;

  /* account form */
  int editImapSec, editSmtpSec;

  /* running job */
  mail_job_t *job;
  int abandonedJobs;
  Boolean waitOpened;
  RectangleType waitRect;
} mail_app_t;

static Boolean ApplicationHandleEvent(EventType *event);

/*
 * Small helpers
 */

static void *getObject(FormType *frm, UInt16 id) {
  UInt16 index = FrmGetObjectIndex(frm, id);
  return index == frmInvalidObjectId ? NULL : FrmGetObjectPtr(frm, index);
}

static void strCopy(char *dst, const char *src, int size) {
  if (size <= 0) return;
  StrNCopy(dst, src ? src : "", size - 1);
  dst[size - 1] = 0;
}

static void strTrim(char *s) {
  int n = StrLen(s);
  char *p = s;

  while (n > 0 && (s[n - 1] == ' ' || s[n - 1] == '\t' || s[n - 1] == '\n' || s[n - 1] == '\r')) s[--n] = 0;
  while (*p == ' ' || *p == '\t') p++;
  if (p != s) MemMove(s, p, StrLen(p) + 1);
}

/* replace the text of a field; the form frees the handle when it closes */
static void setFieldText(FormType *frm, UInt16 id, const char *text) {
  FieldType *fld = getObject(frm, id);
  MemHandle h, old;
  UInt32 len;
  char *p;

  if (fld == NULL) return;
  len = text ? StrLen(text) : 0;
  if ((h = MemHandleNew(len + 1)) == NULL) return;
  p = MemHandleLock(h);
  if (len) MemMove(p, text, len);
  p[len] = 0;
  MemHandleUnlock(h);

  old = FldGetTextHandle(fld);
  FldSetTextHandle(fld, h);
  if (old) MemHandleFree(old);
}

static const char *getFieldText(FormType *frm, UInt16 id) {
  FieldType *fld = getObject(frm, id);
  const char *s = fld ? FldGetTextPtr(fld) : NULL;
  return s ? s : "";
}

static void copyField(FormType *frm, UInt16 id, char *dst, int size) {
  strCopy(dst, getFieldText(frm, id), size);
  strTrim(dst);
}

static void setFieldNum(FormType *frm, UInt16 id, int n) {
  char buf[16];
  StrIToA(buf, n);
  setFieldText(frm, id, buf);
}

static void drawFieldIfVisible(FormType *frm, UInt16 id) {
  FieldType *fld = getObject(frm, id);
  if (fld && FrmVisible(frm)) FldDrawField(fld);
}

/* keep a scrollbar in sync with a multi-line field */
static void updateScrollbar(FormType *frm, UInt16 fldId, UInt16 sclId) {
  FieldType *fld = getObject(frm, fldId);
  ScrollBarType *bar = getObject(frm, sclId);
  UInt16 scrollPos, textHeight, fieldHeight;
  Int16 maxValue;

  if (fld == NULL || bar == NULL) return;
  FldGetScrollValues(fld, &scrollPos, &textHeight, &fieldHeight);
  if (textHeight > fieldHeight) maxValue = textHeight - fieldHeight;
  else if (scrollPos) maxValue = scrollPos;
  else maxValue = 0;
  SclSetScrollBar(bar, scrollPos, 0, maxValue, fieldHeight > 1 ? fieldHeight - 1 : 1);
}

static void scrollField(FormType *frm, UInt16 fldId, UInt16 sclId, Int16 lines) {
  FieldType *fld = getObject(frm, fldId);

  if (fld == NULL || lines == 0) return;
  if (lines < 0) FldScrollField(fld, -lines, winUp);
  else FldScrollField(fld, lines, winDown);
  updateScrollbar(frm, fldId, sclId);
}

static void pageField(FormType *frm, UInt16 fldId, UInt16 sclId, WinDirectionType dir) {
  FieldType *fld = getObject(frm, fldId);
  UInt16 lines;

  if (fld == NULL || !FldScrollable(fld, dir)) return;
  lines = FldGetVisibleLines(fld);
  FldScrollField(fld, lines > 1 ? lines - 1 : 1, dir);
  updateScrollbar(frm, fldId, sclId);
}

static void showError(const char *msg) {
  FrmCustomAlert(ErrorAlert, msg, "", "");
}

static void showInfo(const char *msg) {
  FrmCustomAlert(InfoAlert, msg, "", "");
}

static Boolean confirm(const char *msg) {
  return FrmCustomAlert(ConfirmAlert, msg, "", "") == 0;
}

/* "Display Name <addr>" -> "Display Name" (or the address when there is no name) */
static void displayName(const char *from, char *out, int size) {
  const char *lt = StrChr(from, '<');
  int n;

  if (lt && lt > from) {
    n = (int)(lt - from);
    if (n > size - 1) n = size - 1;
    MemMove(out, from, n);
    out[n] = 0;
    strTrim(out);
    n = StrLen(out);
    if (n >= 2 && out[0] == '"' && out[n - 1] == '"') {
      MemMove(out, out + 1, n - 2);
      out[n - 2] = 0;
    }
    if (out[0]) return;
  }
  mail_extract_address(from, out, size);
  if (out[0] == 0) strCopy(out, from, size);
}

/*
 * Preferences
 */

static int defaultPort(int smtp, int sec) {
  if (smtp) return sec == MAIL_SEC_TLS ? 465 : (sec == MAIL_SEC_STARTTLS ? 587 : 25);
  return sec == MAIL_SEC_TLS ? 993 : 143;
}

static void loadPrefs(mail_app_t *a) {
  UInt16 size = sizeof(mail_prefs_t);
  Int16 version;
  mail_account_t *acc = &a->prefs.account;

  MemSet(&a->prefs, sizeof(mail_prefs_t), 0);
  version = PrefGetAppPreferences(pumpkin_get_app_creator(), PREFS_ID, &a->prefs, &size, true);
  if (version == noPreferenceFound || version != PREFS_VERSION || size != sizeof(mail_prefs_t)) {
    MemSet(&a->prefs, sizeof(mail_prefs_t), 0);
    acc->imap_sec = MAIL_SEC_TLS;
    acc->imap_port = defaultPort(0, MAIL_SEC_TLS);
    acc->smtp_sec = MAIL_SEC_TLS;
    acc->smtp_port = defaultPort(1, MAIL_SEC_TLS);
    acc->fetch_count = DEFAULT_FETCH;
  }

  /* make sure every string is terminated, whatever was stored */
  acc->name[MAIL_NAME_LEN - 1] = 0;
  acc->email[MAIL_ADDR_LEN - 1] = 0;
  acc->user[MAIL_USER_LEN - 1] = 0;
  acc->pass[MAIL_PASS_LEN - 1] = 0;
  acc->imap_host[MAIL_HOST_LEN - 1] = 0;
  acc->smtp_host[MAIL_HOST_LEN - 1] = 0;
  a->prefs.folder[MAIL_FOLDER_LEN - 1] = 0;
  if (a->prefs.folder[0] == 0) StrCopy(a->prefs.folder, "INBOX");
  if (acc->fetch_count < 1 || acc->fetch_count > MAX_FETCH) acc->fetch_count = DEFAULT_FETCH;
  if (acc->imap_sec > MAIL_SEC_TLS) acc->imap_sec = MAIL_SEC_TLS;
  if (acc->smtp_sec > MAIL_SEC_TLS) acc->smtp_sec = MAIL_SEC_TLS;
}

static void savePrefs(mail_app_t *a) {
  PrefSetAppPreferences(pumpkin_get_app_creator(), PREFS_ID, PREFS_VERSION, &a->prefs, sizeof(mail_prefs_t), true);
}

/*
 * Jobs: network operations in a worker thread
 */

static mail_job_t *jobNew(mail_app_t *a, int type) {
  mail_job_t *job;

  if ((job = xcalloc(1, sizeof(mail_job_t))) == NULL) return NULL;
  job->type = type;
  job->account = a->prefs.account;
  StrCopy(job->folder, a->prefs.folder);

  return job;
}

static void jobFree(mail_job_t *job) {
  if (job == NULL) return;
  if (job->headers) xfree(job->headers);
  if (job->folders) xfree(job->folders);
  if (job->rcpts) xfree(job->rcpts);
  if (job->data) xfree(job->data);
  mail_message_free(&job->msg);
  if (job->session) mail_session_destroy(job->session);
  xfree(job);
}

/* runs in the worker thread */
static int jobProgress(void *data, const char *status) {
  mail_job_t *job = (mail_job_t *)data;

  mutex_lock(job->mutex);
  StrNCopy(job->status, status, MAIL_STATUS_LEN - 1);
  job->status[MAIL_STATUS_LEN - 1] = 0;
  job->status_changed = 1;
  mutex_unlock(job->mutex);

  return job->cancel;
}

/* the worker thread */
static int jobThread(void *arg) {
  mail_job_t *job = (mail_job_t *)arg;
  int abandoned;

  switch (job->type) {
    case jobHeaders:
      job->result = mail_imap_fetch_headers(job->session, job->folder, job->max, &job->headers, &job->nheaders, &job->total);
      break;
    case jobMessage:
      job->result = mail_imap_fetch_message(job->session, job->folder, job->uid, MESSAGE_LIMIT, &job->msg);
      break;
    case jobDelete:
      job->result = mail_imap_delete_message(job->session, job->folder, job->uid);
      break;
    case jobFolders:
      job->result = mail_imap_list_folders(job->session, &job->folders, &job->nfolders);
      break;
    case jobSend:
      job->result = mail_smtp_send(&job->ctx, &job->account, job->rcpts, job->data, job->datalen);
      break;
    default:
      job->result = MAIL_ERR_ARG;
      break;
  }

  /* keep the error text of the session in the job for the UI */
  if (job->session) {
    mail_ctx_t *ctx = mail_session_ctx(job->session);
    if (ctx) StrNCopy(job->ctx.error, ctx->error, MAIL_ERROR_LEN - 1);
  }

  mutex_lock(job->mutex);
  abandoned = job->abandoned;
  job->done = 1;
  mutex_unlock(job->mutex);

  /* the UI gave up on this job: nobody else will free it */
  if (abandoned) jobFree(job);

  return 0;
}

static void waitDrawStatus(mail_app_t *a, const char *status) {
  FormType *frm = FrmGetActiveForm();
  FontID old;

  if (!a->waitOpened || frm == NULL || FrmGetFormId(frm) != WaitForm) return;
  WinEraseRectangle(&a->waitRect, 0);
  old = FntSetFont(stdFont);
  WinDrawTruncChars(status, StrLen(status), a->waitRect.topLeft.x + 2, a->waitRect.topLeft.y + 5, a->waitRect.extent.x - 4);
  FntSetFont(old);
}

/*
 * Start a job and pump events until it finishes, showing the WaitForm.
 * Returns true when the job completed (job->result is valid and the caller
 * owns the job); false when it was abandoned (the worker frees it later).
 */
static Boolean runJob(mail_app_t *a, mail_job_t *job) {
  EventType event;
  UInt32 tps;
  int handle, done = 0, changed, cancelTicks = 0;
  char status[MAIL_STATUS_LEN];
  mail_ctx_t *ctx;

  job->mutex = a->mutex;
  job->ctx.secure = (secure_provider_t *)pumpkin_get_secure();
  job->ctx.progress = jobProgress;
  job->ctx.progress_data = job;
  job->ctx.cancel = &job->cancel;
  job->ctx.timeout = NET_TIMEOUT;
  StrCopy(job->status, "Connecting...");

  if (job->type != jobSend) {
    if (a->session == NULL) {
      a->session = mail_session_create(&job->ctx, &a->prefs.account);
      if (a->session == NULL) {
        job->result = MAIL_ERR_MEM;
        job->done = 1;
        return true;
      }
    }
    mail_session_set_account(a->session, &a->prefs.account);
    ctx = mail_session_ctx(a->session);
    *ctx = job->ctx;
    /* the worker owns the session until the job is over */
    job->session = a->session;
    a->session = NULL;
  }

  a->job = job;
  a->waitOpened = false;

  if ((handle = thread_begin("MAILJOB", jobThread, job)) == -1) {
    a->job = NULL;
    if (job->session) {
      a->session = job->session;
      job->session = NULL;
    }
    job->result = MAIL_ERR_MEM;
    StrCopy(job->ctx.error, "Could not start the network thread");
    job->done = 1;
    return true;
  }

  FrmPopupForm(WaitForm);
  tps = SysTicksPerSecond();

  for (;;) {
    EvtGetEvent(&event, tps / 10);

    if (event.eType == appStopEvent) {
      a->stop = true;
      job->cancel = 1;
    } else if (event.eType == ctlSelectEvent && event.data.ctlSelect.controlID == waitCancelBtn) {
      job->cancel = 1;
    } else if (event.eType != nilEvent) {
      if (!SysHandleEvent(&event)) {
        if (!ApplicationHandleEvent(&event)) {
          FrmDispatchEvent(&event);
        }
      }
    }

    mutex_lock(a->mutex);
    done = job->done;
    changed = job->status_changed;
    job->status_changed = 0;
    if (changed) StrCopy(status, job->status);
    mutex_unlock(a->mutex);

    /* wait until the dialog is really open before tearing it down again */
    if (!a->waitOpened) continue;
    if (done) break;
    if (changed) waitDrawStatus(a, status);

    if (job->cancel) {
      if (cancelTicks == 0) waitDrawStatus(a, "Cancelling...");
      if (++cancelTicks > CANCEL_GRACE) break;
    }
  }

  if (!done) {
    mutex_lock(a->mutex);
    done = job->done;
    if (!done) {
      job->abandoned = 1;
      a->abandonedJobs++;
    }
    mutex_unlock(a->mutex);
    if (!done) {
      /* asks the thread to end: the TLS provider checks thread_must_end() while waiting */
      thread_end("MAILJOB", handle);
      debug(DEBUG_INFO, TAG, "job abandoned");
    }
  }

  a->job = NULL;
  a->waitOpened = false;
  FrmReturnToForm(0);

  if (!done) return false;

  if (job->session) {
    a->session = job->session;
    job->session = NULL;
  }

  return true;
}

/* show the reason a job failed (nothing for a cancel) */
static void showJobError(mail_job_t *job) {
  char *msg;
  const char *detail = job->ctx.error[0] ? job->ctx.error : mail_error_string(job->result);

  if (job->result == MAIL_OK || job->result == MAIL_ERR_CANCEL) return;
  if ((msg = xmalloc(512)) == NULL) return;

  switch (job->result) {
    case MAIL_ERR_NOTLS:
      sys_snprintf(msg, 512, "No TLS provider is loaded, so a secure connection is not possible. "
        "Start PumpkinOS with liblopenssl or libls2n (enable HTTPS on the ESP32), "
        "or set Secure to None in the account settings.");
      break;
    case MAIL_ERR_TLS:
      sys_snprintf(msg, 512, "Secure connection failed. %s. Check the server name, port and security setting.", detail);
      break;
    case MAIL_ERR_AUTH:
      sys_snprintf(msg, 512, "%s. Check the user name and password (many providers require an app-specific password).", detail);
      break;
    default:
      sys_snprintf(msg, 512, "%s", detail);
      break;
  }
  showError(msg);
  xfree(msg);
}

/*
 * Main form: message list
 */

static void setMainTitle(mail_app_t *a, FormType *frm) {
  char folder[20];

  displayName(a->prefs.folder, folder, sizeof(folder));
  strCopy(folder, a->prefs.folder, 16);
  if (a->total > 0) {
    sys_snprintf(a->title, sizeof(a->title), "%s (%d)", folder, a->total);
  } else {
    sys_snprintf(a->title, sizeof(a->title), "%s", folder);
  }
  FrmSetTitle(frm, a->title);
}

static void drawMessageItem(Int16 itemNum, RectangleType *bounds, Char **itemsText) {
  mail_app_t *a = (mail_app_t *)pumpkin_get_data();
  mail_header_t *h;
  char name[MAIL_FROM_LEN];
  FontID old;
  Coord x, y, w;

  (void)itemsText;
  if (a == NULL || itemNum < 0 || itemNum >= a->nheaders) return;
  h = &a->headers[itemNum];

  old = FntSetFont(h->seen ? stdFont : boldFont);
  x = bounds->topLeft.x;
  y = bounds->topLeft.y;
  w = bounds->extent.x;

  displayName(h->from, name, sizeof(name));
  WinDrawTruncChars(name, StrLen(name), x + 1, y, FROM_WIDTH - 3);
  WinDrawTruncChars(h->subject, StrLen(h->subject), x + FROM_WIDTH, y, w - FROM_WIDTH - 2);
  FntSetFont(old);
}

static void drawEmptyHint(mail_app_t *a, FormType *frm) {
  ListType *list = getObject(frm, msgList);
  RectangleType r;
  FontID old;
  const char *msg;

  if (list == NULL || a->nheaders > 0) return;
  FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, msgList), &r);
  msg = a->prefs.configured ? "No messages. Tap Get Mail." : "Tap Account to set up your mailbox.";
  old = FntSetFont(stdFont);
  WinDrawTruncChars(msg, StrLen(msg), r.topLeft.x + 4, r.topLeft.y + 4, r.extent.x - 8);
  FntSetFont(old);
}

static void refreshList(mail_app_t *a) {
  FormType *frm = FrmGetActiveForm();
  ListType *list;

  if (frm == NULL || FrmGetFormId(frm) != MainForm) return;
  list = getObject(frm, msgList);
  if (list == NULL) return;

  LstSetListChoices(list, NULL, a->nheaders);
  LstSetSelection(list, noListSelection);
  if (a->nheaders > 0) LstSetTopItem(list, 0);
  LstDrawList(list);
  drawEmptyHint(a, frm);
  setMainTitle(a, frm);
}

static void fetchHeaders(mail_app_t *a) {
  mail_job_t *job;

  if (!a->prefs.configured) return;
  if ((job = jobNew(a, jobHeaders)) == NULL) return;
  job->max = a->prefs.account.fetch_count;

  if (!runJob(a, job)) return;
  if (a->stop) { jobFree(job); return; }

  if (job->result != MAIL_OK) {
    showJobError(job);
    jobFree(job);
    return;
  }

  if (a->headers) xfree(a->headers);
  a->headers = job->headers;
  a->nheaders = job->nheaders;
  a->total = job->total;
  job->headers = NULL;
  jobFree(job);

  refreshList(a);
}

static void openMessage(mail_app_t *a, int index) {
  mail_job_t *job;

  if (index < 0 || index >= a->nheaders) return;
  if ((job = jobNew(a, jobMessage)) == NULL) return;
  job->uid = a->headers[index].uid;

  if (!runJob(a, job)) return;
  if (a->stop) { jobFree(job); return; }

  if (job->result != MAIL_OK) {
    showJobError(job);
    jobFree(job);
    return;
  }

  mail_message_free(&a->msg);
  a->msg = job->msg;
  MemSet(&job->msg, sizeof(mail_message_t), 0);
  a->current = index;
  a->msgUid = a->headers[index].uid;
  a->headers[index].seen = 1;
  if (a->msg.subject[0] == 0) StrCopy(a->msg.subject, "(no subject)");
  jobFree(job);

  FrmGotoForm(ViewForm);
}

/* fill the folder popup list; returns false if the list could not be fetched */
static Boolean loadFolders(mail_app_t *a, Boolean force) {
  mail_job_t *job;
  int i, n;

  if (a->nfolders > 0 && !force) return true;
  if (!a->prefs.configured) return false;
  if ((job = jobNew(a, jobFolders)) == NULL) return false;

  if (!runJob(a, job)) return false;
  if (a->stop) { jobFree(job); return false; }

  if (job->result != MAIL_OK) {
    showJobError(job);
    jobFree(job);
    return false;
  }

  if (a->folders) xfree(a->folders);
  if (a->folderNames) xfree(a->folderNames);
  a->folders = job->folders;
  a->nfolders = job->nfolders;
  job->folders = NULL;
  jobFree(job);

  a->folderNames = xcalloc(a->nfolders + 1, sizeof(char *));
  n = 0;
  if (a->folderNames) {
    for (i = 0; i < a->nfolders; i++) {
      if (a->folders[i].selectable) a->folderNames[n++] = a->folders[i].name;
    }
  }
  a->nfolderNames = n;

  return n > 0;
}

static void prepareFolderPopup(mail_app_t *a, FormType *frm) {
  ListType *list = getObject(frm, folderList);
  int i, sel = noListSelection;

  if (list == NULL) return;
  for (i = 0; i < a->nfolderNames; i++) {
    if (StrCompare(a->folderNames[i], a->prefs.folder) == 0) sel = i;
  }
  LstSetListChoices(list, a->folderNames, a->nfolderNames);
  LstSetSelection(list, sel);
  if (sel != noListSelection) LstMakeItemVisible(list, sel);
}

static void selectFolder(mail_app_t *a, int index) {
  if (index < 0 || index >= a->nfolderNames) return;
  if (StrCompare(a->folderNames[index], a->prefs.folder) == 0) return;

  strCopy(a->prefs.folder, a->folderNames[index], MAIL_FOLDER_LEN);
  savePrefs(a);
  if (a->headers) xfree(a->headers);
  a->headers = NULL;
  a->nheaders = 0;
  a->total = 0;
  refreshList(a);
  fetchHeaders(a);
}

static void startCompose(mail_app_t *a, int mode);

static Boolean MainFormHandleEvent(EventType *event) {
  mail_app_t *a = (mail_app_t *)pumpkin_get_data();
  FormType *frm;
  ListType *list;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      list = getObject(frm, msgList);
      if (list) {
        LstSetDrawFunction(list, drawMessageItem);
        LstSetListChoices(list, NULL, a->nheaders);
        LstSetSelection(list, noListSelection);
        if (a->current >= 0 && a->current < a->nheaders) LstMakeItemVisible(list, a->current);
      }
      setMainTitle(a, frm);
      FrmDrawForm(frm);
      drawEmptyHint(a, frm);

      if (!a->prefs.configured) {
        if (!a->promptedAccount) {
          a->promptedAccount = true;
          FrmAlert(NoAccountAlert);
          FrmGotoForm(AccountForm);
        }
      } else if (a->fetchOnOpen) {
        a->fetchOnOpen = false;
        fetchHeaders(a);
      }
      handled = true;
      break;

    case frmUpdateEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      drawEmptyHint(a, frm);
      handled = true;
      break;

    case lstSelectEvent:
      if (event->data.lstSelect.listID == msgList) {
        openMessage(a, event->data.lstSelect.selection);
        handled = true;
      }
      break;

    case popSelectEvent:
      if (event->data.popSelect.listID == folderList) {
        selectFolder(a, event->data.popSelect.selection);
        handled = true;
      }
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case newBtn:
          startCompose(a, composeNew);
          handled = true;
          break;
        case getBtn:
          fetchHeaders(a);
          handled = true;
          break;
        case accountBtn:
          FrmGotoForm(AccountForm);
          handled = true;
          break;
        case folderTrig:
          /* fetch the folder list on first use, then let the form pop up the list */
          frm = FrmGetActiveForm();
          if (loadFolders(a, false)) {
            prepareFolderPopup(a, frm);
            handled = false;
          } else {
            handled = true;
          }
          break;
      }
      break;

    case keyDownEvent:
      frm = FrmGetActiveForm();
      list = getObject(frm, msgList);
      if (list && (event->data.keyDown.modifiers & commandKeyMask)) {
        switch (event->data.keyDown.chr) {
          case vchrPageUp:
            LstScrollList(list, winUp, 11);
            handled = true;
            break;
          case vchrPageDown:
            LstScrollList(list, winDown, 11);
            handled = true;
            break;
        }
      }
      break;

    case menuEvent:
      switch (event->data.menu.itemID) {
        case newCmd:
          startCompose(a, composeNew);
          handled = true;
          break;
        case getCmd:
          fetchHeaders(a);
          handled = true;
          break;
        case foldersCmd:
          frm = FrmGetActiveForm();
          if (loadFolders(a, true)) {
            ListType *fl;
            Int16 sel;
            prepareFolderPopup(a, frm);
            fl = getObject(frm, folderList);
            if (fl && (sel = LstPopupList(fl)) != noListSelection) selectFolder(a, sel);
          }
          handled = true;
          break;
        case accountCmd:
          FrmGotoForm(AccountForm);
          handled = true;
          break;
        case aboutCmd:
          AbtShowAbout(pumpkin_get_app_creator());
          handled = true;
          break;
      }
      break;

    default:
      break;
  }

  return handled;
}

/*
 * View form: one message
 */

static void drawHeaderLine(const char *label, const char *value, Coord y) {
  FntSetFont(boldFont);
  WinDrawChars(label, StrLen(label), 2, y);
  FntSetFont(stdFont);
  WinDrawTruncChars(value, StrLen(value), LABEL_WIDTH + 2, y, 160 - LABEL_WIDTH - 4);
}

static void drawMessageHeaders(mail_app_t *a, FormType *frm) {
  RectangleType r;
  FontID old;
  Coord y, lh;

  FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, headerGad), &r);
  WinEraseRectangle(&r, 0);
  old = FntSetFont(stdFont);
  lh = FntLineHeight();
  y = r.topLeft.y + 1;
  drawHeaderLine("From:", a->msg.from, y);
  drawHeaderLine("To:", a->msg.to[0] ? a->msg.to : "(undisclosed)", y + lh);
  drawHeaderLine("Subj:", a->msg.subject, y + 2 * lh);
  drawHeaderLine("Date:", a->msg.date, y + 3 * lh);
  FntSetFont(old);
}

static void setBodyText(mail_app_t *a, FormType *frm) {
  char *text;
  int len;

  len = a->msg.text ? a->msg.text_len : 0;
  if (len > BODY_MAX) {
    if ((text = xmalloc(BODY_MAX + 32)) != NULL) {
      MemMove(text, a->msg.text, BODY_MAX);
      StrCopy(text + BODY_MAX, "\n[...]");
      setFieldText(frm, bodyFld, text);
      xfree(text);
    }
  } else {
    setFieldText(frm, bodyFld, a->msg.text ? a->msg.text : "");
  }
}

static void deleteCurrent(mail_app_t *a) {
  mail_job_t *job;
  int i;

  if (a->current < 0 || a->current >= a->nheaders) return;
  if (!confirm("Delete this message from the server?")) return;

  if ((job = jobNew(a, jobDelete)) == NULL) return;
  job->uid = a->msgUid;

  if (!runJob(a, job)) return;
  if (a->stop) { jobFree(job); return; }

  if (job->result != MAIL_OK) {
    showJobError(job);
    jobFree(job);
    return;
  }
  jobFree(job);

  /* drop it from the list */
  for (i = a->current; i + 1 < a->nheaders; i++) a->headers[i] = a->headers[i + 1];
  a->nheaders--;
  if (a->total > 0) a->total--;
  if (a->current >= a->nheaders) a->current = a->nheaders - 1;
  mail_message_free(&a->msg);

  FrmGotoForm(MainForm);
}

static Boolean ViewFormHandleEvent(EventType *event) {
  mail_app_t *a = (mail_app_t *)pumpkin_get_data();
  FormType *frm;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      setBodyText(a, frm);
      FrmDrawForm(frm);
      drawMessageHeaders(a, frm);
      updateScrollbar(frm, bodyFld, bodyScl);
      handled = true;
      break;

    case frmUpdateEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      drawMessageHeaders(a, frm);
      updateScrollbar(frm, bodyFld, bodyScl);
      handled = true;
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case doneBtn:
          FrmGotoForm(MainForm);
          handled = true;
          break;
        case replyBtn:
          startCompose(a, composeReply);
          handled = true;
          break;
        case fwdBtn:
          startCompose(a, composeForward);
          handled = true;
          break;
        case delBtn:
          deleteCurrent(a);
          handled = true;
          break;
      }
      break;

    case sclRepeatEvent:
      frm = FrmGetActiveForm();
      scrollField(frm, bodyFld, bodyScl, event->data.sclRepeat.newValue - event->data.sclRepeat.value);
      break;

    case sclExitEvent:
      frm = FrmGetActiveForm();
      scrollField(frm, bodyFld, bodyScl, event->data.sclExit.newValue - event->data.sclExit.value);
      handled = true;
      break;

    case keyDownEvent:
      frm = FrmGetActiveForm();
      if (event->data.keyDown.modifiers & commandKeyMask) {
        switch (event->data.keyDown.chr) {
          case vchrPageUp:
          case vchrRockerUp:
            pageField(frm, bodyFld, bodyScl, winUp);
            handled = true;
            break;
          case vchrPageDown:
          case vchrRockerDown:
            pageField(frm, bodyFld, bodyScl, winDown);
            handled = true;
            break;
          case vchrRockerLeft:
            FrmGotoForm(MainForm);
            handled = true;
            break;
        }
      }
      break;

    case menuEvent:
      switch (event->data.menu.itemID) {
        case replyCmd:
          startCompose(a, composeReply);
          handled = true;
          break;
        case fwdCmd:
          startCompose(a, composeForward);
          handled = true;
          break;
        case delCmd:
          deleteCurrent(a);
          handled = true;
          break;
        case doneCmd:
          FrmGotoForm(MainForm);
          handled = true;
          break;
        case aboutCmd:
          AbtShowAbout(pumpkin_get_app_creator());
          handled = true;
          break;
      }
      break;

    default:
      break;
  }

  return handled;
}

/*
 * Compose form
 */

/* append s to buf (size bytes), keeping it terminated */
static void appendText(char *buf, int size, const char *s) {
  int n = StrLen(buf), m = StrLen(s);

  if (n + m > size - 1) m = size - 1 - n;
  if (m > 0) {
    MemMove(buf + n, s, m);
    buf[n + m] = 0;
  }
}

/* build the quoted text of a reply or the body of a forward */
static void buildQuotedBody(mail_app_t *a, int mode) {
  const char *text = a->msg.text ? a->msg.text : "";
  const char *p, *eol;
  char *buf, line[600];
  int n, size = QUOTE_MAX;

  if (a->composeBody) xfree(a->composeBody);
  if ((a->composeBody = xmalloc(size)) == NULL) return;
  buf = a->composeBody;
  buf[0] = 0;

  if (mode == composeReply) {
    sys_snprintf(line, sizeof(line), "\n\nOn %s, %s wrote:\n", a->msg.date, a->msg.from);
    appendText(buf, size, line);
    for (p = text; *p && StrLen(buf) < size - 8;) {
      eol = StrChr(p, '\n');
      n = eol ? (int)(eol - p) : StrLen(p);
      if (n > (int)sizeof(line) - 4) n = sizeof(line) - 4;
      line[0] = '>'; line[1] = ' ';
      MemMove(line + 2, p, n);
      line[n + 2] = '\n';
      line[n + 3] = 0;
      appendText(buf, size, line);
      p = eol ? eol + 1 : p + n;
    }
  } else {
    sys_snprintf(line, sizeof(line), "\n\n-------- Forwarded message --------\nFrom: %s\nDate: %s\nSubject: %s\nTo: %s\n\n",
      a->msg.from, a->msg.date, a->msg.subject, a->msg.to);
    appendText(buf, size, line);
    appendText(buf, size, text);
  }
  if (StrLen(buf) >= size - 8) appendText(buf, size, "\n[...]");
}

static void startCompose(mail_app_t *a, int mode) {
  a->composeMode = mode;
  a->composeTo[0] = 0;
  a->composeCc[0] = 0;
  a->composeSubject[0] = 0;
  a->inReplyTo[0] = 0;
  if (a->composeBody) {
    xfree(a->composeBody);
    a->composeBody = NULL;
  }

  if (mode == composeNew) {
    StrCopy(a->composeTitle, "New Message");
    a->composeReturn = MainForm;
  } else if (mode == composeReply) {
    StrCopy(a->composeTitle, "Reply");
    a->composeReturn = ViewForm;
    strCopy(a->composeTo, a->msg.reply_to[0] ? a->msg.reply_to : a->msg.from, sizeof(a->composeTo));
    if (StrNCaselessCompare(a->msg.subject, "Re:", 3) == 0) {
      strCopy(a->composeSubject, a->msg.subject, sizeof(a->composeSubject));
    } else {
      sys_snprintf(a->composeSubject, sizeof(a->composeSubject), "Re: %s", a->msg.subject);
    }
    strCopy(a->inReplyTo, a->msg.message_id, sizeof(a->inReplyTo));
    buildQuotedBody(a, mode);
  } else {
    StrCopy(a->composeTitle, "Forward");
    a->composeReturn = ViewForm;
    if (StrNCaselessCompare(a->msg.subject, "Fwd:", 4) == 0) {
      strCopy(a->composeSubject, a->msg.subject, sizeof(a->composeSubject));
    } else {
      sys_snprintf(a->composeSubject, sizeof(a->composeSubject), "Fwd: %s", a->msg.subject);
    }
    buildQuotedBody(a, mode);
  }

  FrmGotoForm(ComposeForm);
}

static void sendMessage(mail_app_t *a) {
  FormType *frm = FrmGetActiveForm();
  mail_job_t *job;
  const char *to, *cc, *subject, *body;
  char *data, *rcpts;
  int len;

  to = getFieldText(frm, toFld);
  cc = getFieldText(frm, ccFld);
  subject = getFieldText(frm, subjFld);
  body = getFieldText(frm, cbodyFld);

  if (to[0] == 0) {
    showError("Please enter at least one recipient.");
    FrmSetFocus(frm, FrmGetObjectIndex(frm, toFld));
    return;
  }
  if (a->prefs.account.smtp_host[0] == 0) {
    showError("No SMTP server is configured. Open the account settings first.");
    return;
  }

  if ((data = mail_build_message(&a->prefs.account, to, cc, subject, a->inReplyTo, body, &len)) == NULL) {
    showError("Not enough memory to build the message.");
    return;
  }
  if ((rcpts = xmalloc(StrLen(to) + StrLen(cc) + 2)) == NULL) {
    xfree(data);
    return;
  }
  sys_snprintf(rcpts, StrLen(to) + StrLen(cc) + 2, "%s,%s", to, cc);

  if ((job = jobNew(a, jobSend)) == NULL) {
    xfree(data);
    xfree(rcpts);
    return;
  }
  job->rcpts = rcpts;
  job->data = data;
  job->datalen = len;

  if (!runJob(a, job)) return;
  if (a->stop) { jobFree(job); return; }

  if (job->result != MAIL_OK) {
    showJobError(job);
    jobFree(job);
    return;
  }
  jobFree(job);

  showInfo("Message sent.");
  if (a->composeBody) {
    xfree(a->composeBody);
    a->composeBody = NULL;
  }
  FrmGotoForm(a->composeReturn);
}

static void cancelCompose(mail_app_t *a) {
  FormType *frm = FrmGetActiveForm();
  const char *body = getFieldText(frm, cbodyFld);
  const char *to = getFieldText(frm, toFld);
  const char *subject = getFieldText(frm, subjFld);
  Boolean dirty;

  if (a->composeMode == composeNew) {
    dirty = body[0] || to[0] || subject[0];
  } else {
    /* only the pre-filled quote: nothing typed */
    dirty = a->composeBody == NULL || StrCompare(body, a->composeBody) != 0;
  }
  if (dirty && !confirm("Discard this message?")) return;

  FrmGotoForm(a->composeReturn);
}

static Boolean ComposeFormHandleEvent(EventType *event) {
  mail_app_t *a = (mail_app_t *)pumpkin_get_data();
  FormType *frm;
  FieldType *fld;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      FrmSetTitle(frm, a->composeTitle);
      setFieldText(frm, toFld, a->composeTo);
      setFieldText(frm, ccFld, a->composeCc);
      setFieldText(frm, subjFld, a->composeSubject);
      setFieldText(frm, cbodyFld, a->composeBody ? a->composeBody : "");
      if ((fld = getObject(frm, cbodyFld)) != NULL) FldSetInsPtPosition(fld, 0);
      FrmDrawForm(frm);
      updateScrollbar(frm, cbodyFld, cbodyScl);
      FrmSetFocus(frm, FrmGetObjectIndex(frm, a->composeMode == composeNew ? toFld : cbodyFld));
      handled = true;
      break;

    case frmUpdateEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      updateScrollbar(frm, cbodyFld, cbodyScl);
      handled = true;
      break;

    case ctlSelectEvent:
      switch (event->data.ctlSelect.controlID) {
        case sendBtn:
          sendMessage(a);
          handled = true;
          break;
        case cancelBtn:
          cancelCompose(a);
          handled = true;
          break;
      }
      break;

    case fldChangedEvent:
      if (event->data.fldChanged.fieldID == cbodyFld) {
        frm = FrmGetActiveForm();
        updateScrollbar(frm, cbodyFld, cbodyScl);
        handled = true;
      }
      break;

    case sclRepeatEvent:
      frm = FrmGetActiveForm();
      scrollField(frm, cbodyFld, cbodyScl, event->data.sclRepeat.newValue - event->data.sclRepeat.value);
      break;

    case sclExitEvent:
      frm = FrmGetActiveForm();
      scrollField(frm, cbodyFld, cbodyScl, event->data.sclExit.newValue - event->data.sclExit.value);
      handled = true;
      break;

    case keyDownEvent:
      frm = FrmGetActiveForm();
      if (event->data.keyDown.modifiers & commandKeyMask) {
        switch (event->data.keyDown.chr) {
          case vchrPageUp:
            pageField(frm, cbodyFld, cbodyScl, winUp);
            handled = true;
            break;
          case vchrPageDown:
            pageField(frm, cbodyFld, cbodyScl, winDown);
            handled = true;
            break;
        }
      }
      break;

    case menuEvent:
      switch (event->data.menu.itemID) {
        case sendCmd:
          sendMessage(a);
          handled = true;
          break;
        case discardCmd:
          cancelCompose(a);
          handled = true;
          break;
        case aboutCmd:
          AbtShowAbout(pumpkin_get_app_creator());
          handled = true;
          break;
      }
      break;

    default:
      break;
  }

  return handled;
}

/*
 * Account form
 */

static void setSecurityPopup(FormType *frm, UInt16 trigId, UInt16 listId, int sec) {
  ListType *list = getObject(frm, listId);
  ControlType *trig = getObject(frm, trigId);

  if (list == NULL || trig == NULL) return;
  if (sec < 0 || sec > MAIL_SEC_TLS) sec = MAIL_SEC_TLS;
  LstSetSelection(list, sec);
  CtlSetLabel(trig, LstGetSelectionText(list, sec));
}

static void fillAccountForm(mail_app_t *a, FormType *frm) {
  mail_account_t *acc = &a->prefs.account;

  setFieldText(frm, nameFld, acc->name);
  setFieldText(frm, emailFld, acc->email);
  setFieldText(frm, userFld, acc->user);
  setFieldText(frm, passFld, acc->pass);
  setFieldText(frm, imapHostFld, acc->imap_host);
  setFieldNum(frm, imapPortFld, acc->imap_port);
  setFieldText(frm, smtpHostFld, acc->smtp_host);
  setFieldNum(frm, smtpPortFld, acc->smtp_port);
  setFieldNum(frm, fetchFld, acc->fetch_count);
  a->editImapSec = acc->imap_sec;
  a->editSmtpSec = acc->smtp_sec;
  setSecurityPopup(frm, imapSecTrig, imapSecList, a->editImapSec);
  setSecurityPopup(frm, smtpSecTrig, smtpSecList, a->editSmtpSec);
}

/* security popup changed: move the port along when it still had the default value */
static void securityChanged(mail_app_t *a, FormType *frm, int smtp, int sec) {
  UInt16 portId = smtp ? smtpPortFld : imapPortFld;
  int *cur = smtp ? &a->editSmtpSec : &a->editImapSec;
  int port = StrAToI(getFieldText(frm, portId));

  if (sec < 0 || sec > MAIL_SEC_TLS) return;
  if (port == 0 || port == defaultPort(smtp, *cur)) {
    setFieldNum(frm, portId, defaultPort(smtp, sec));
    drawFieldIfVisible(frm, portId);
  }
  *cur = sec;
  setSecurityPopup(frm, smtp ? smtpSecTrig : imapSecTrig, smtp ? smtpSecList : imapSecList, sec);
}

static Boolean saveAccountForm(mail_app_t *a, FormType *frm) {
  mail_account_t acc;
  int port, fetch;

  MemSet(&acc, sizeof(acc), 0);
  copyField(frm, nameFld, acc.name, sizeof(acc.name));
  copyField(frm, emailFld, acc.email, sizeof(acc.email));
  copyField(frm, userFld, acc.user, sizeof(acc.user));
  copyField(frm, passFld, acc.pass, sizeof(acc.pass));
  copyField(frm, imapHostFld, acc.imap_host, sizeof(acc.imap_host));
  copyField(frm, smtpHostFld, acc.smtp_host, sizeof(acc.smtp_host));
  acc.imap_sec = a->editImapSec;
  acc.smtp_sec = a->editSmtpSec;

  port = StrAToI(getFieldText(frm, imapPortFld));
  acc.imap_port = (port > 0 && port < 65536) ? port : defaultPort(0, acc.imap_sec);
  port = StrAToI(getFieldText(frm, smtpPortFld));
  acc.smtp_port = (port > 0 && port < 65536) ? port : defaultPort(1, acc.smtp_sec);
  fetch = StrAToI(getFieldText(frm, fetchFld));
  acc.fetch_count = (fetch >= 1 && fetch <= MAX_FETCH) ? fetch : DEFAULT_FETCH;

  if (acc.imap_host[0] == 0 || acc.user[0] == 0) {
    showError("Please fill in the IMAP server and the user name.");
    return false;
  }
  if (!StrChr(acc.email, '@')) {
    showError("Please enter a valid e-mail address.");
    return false;
  }
  if (acc.smtp_host[0] == 0) {
    /* a common convention; the user can still change it */
    strCopy(acc.smtp_host, acc.imap_host, sizeof(acc.smtp_host));
  }

  if (MemCmp(&acc, &a->prefs.account, sizeof(mail_account_t)) != 0) {
    a->prefs.account = acc;
    if (a->session) mail_session_set_account(a->session, &acc);
    if (a->headers) xfree(a->headers);
    a->headers = NULL;
    a->nheaders = 0;
    a->total = 0;
    if (a->folders) xfree(a->folders);
    if (a->folderNames) xfree(a->folderNames);
    a->folders = NULL;
    a->folderNames = NULL;
    a->nfolders = a->nfolderNames = 0;
    StrCopy(a->prefs.folder, "INBOX");
    a->fetchOnOpen = true;
  }
  a->prefs.configured = 1;
  savePrefs(a);

  return true;
}

static Boolean AccountFormHandleEvent(EventType *event) {
  mail_app_t *a = (mail_app_t *)pumpkin_get_data();
  FormType *frm;
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
      frm = FrmGetActiveForm();
      fillAccountForm(a, frm);
      FrmDrawForm(frm);
      FrmSetFocus(frm, FrmGetObjectIndex(frm, a->prefs.account.email[0] ? passFld : nameFld));
      handled = true;
      break;

    case frmUpdateEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      handled = true;
      break;

    case popSelectEvent:
      frm = FrmGetActiveForm();
      if (event->data.popSelect.listID == imapSecList) {
        securityChanged(a, frm, 0, event->data.popSelect.selection);
        handled = true;
      } else if (event->data.popSelect.listID == smtpSecList) {
        securityChanged(a, frm, 1, event->data.popSelect.selection);
        handled = true;
      }
      break;

    case ctlSelectEvent:
      frm = FrmGetActiveForm();
      switch (event->data.ctlSelect.controlID) {
        case okBtn:
          if (saveAccountForm(a, frm)) FrmGotoForm(MainForm);
          handled = true;
          break;
        case acancelBtn:
          FrmGotoForm(MainForm);
          handled = true;
          break;
      }
      break;

    default:
      break;
  }

  return handled;
}

/*
 * Wait form (modal progress dialog shown while a job runs)
 */

static Boolean WaitFormHandleEvent(EventType *event) {
  mail_app_t *a = (mail_app_t *)pumpkin_get_data();
  FormType *frm;
  char status[MAIL_STATUS_LEN];
  Boolean handled = false;

  switch (event->eType) {
    case frmOpenEvent:
    case frmUpdateEvent:
      frm = FrmGetActiveForm();
      FrmDrawForm(frm);
      FrmGetObjectBounds(frm, FrmGetObjectIndex(frm, waitGad), &a->waitRect);
      a->waitOpened = true;
      status[0] = 0;
      if (a->job) {
        mutex_lock(a->mutex);
        StrCopy(status, a->job->status);
        mutex_unlock(a->mutex);
      }
      waitDrawStatus(a, status);
      handled = true;
      break;

    case ctlSelectEvent:
      if (event->data.ctlSelect.controlID == waitCancelBtn) {
        if (a->job) a->job->cancel = 1;
        handled = true;
      }
      break;

    default:
      break;
  }

  return handled;
}

/*
 * Application
 */

static Boolean ApplicationHandleEvent(EventType *event) {
  FormType *frm;
  UInt16 formID;
  Boolean handled = false;

  if (event->eType == frmLoadEvent) {
    formID = event->data.frmLoad.formID;
    frm = FrmInitForm(formID);
    FrmSetActiveForm(frm);
    switch (formID) {
      case MainForm:    FrmSetEventHandler(frm, MainFormHandleEvent); break;
      case ViewForm:    FrmSetEventHandler(frm, ViewFormHandleEvent); break;
      case ComposeForm: FrmSetEventHandler(frm, ComposeFormHandleEvent); break;
      case AccountForm: FrmSetEventHandler(frm, AccountFormHandleEvent); break;
      case WaitForm:    FrmSetEventHandler(frm, WaitFormHandleEvent); break;
    }
    handled = true;
  }

  return handled;
}

static void EventLoop(mail_app_t *a) {
  EventType event;
  Err err;

  do {
    EvtGetEvent(&event, evtWaitForever);
    if (SysHandleEvent(&event)) continue;
    if (MenuHandleEvent(NULL, &event, &err)) continue;
    if (ApplicationHandleEvent(&event)) continue;
    FrmDispatchEvent(&event);
  } while (event.eType != appStopEvent && !a->stop);
}

#ifdef ESP32
UInt32 MailPilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
#else
UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
#endif
{
  mail_app_t *a;

  if (cmd == sysAppLaunchCmdNormalLaunch) {
    if ((a = xcalloc(1, sizeof(mail_app_t))) == NULL) return 0;
    a->mutex = mutex_create("mail");
    a->current = -1;
    a->fetchOnOpen = true;
    loadPrefs(a);
    pumpkin_set_data(a);

    if (pumpkin_get_secure() == NULL) {
      debug(DEBUG_INFO, TAG, "no secure provider: only unencrypted connections are possible");
    }

    FrmGotoForm(MainForm);
    EventLoop(a);
    FrmCloseAllForms();

    if (a->session) mail_session_destroy(a->session);
    mail_message_free(&a->msg);
    if (a->headers) xfree(a->headers);
    if (a->folders) xfree(a->folders);
    if (a->folderNames) xfree(a->folderNames);
    if (a->composeBody) xfree(a->composeBody);
    /* an abandoned job may still finish in its thread and lock the mutex */
    if (a->abandonedJobs == 0) mutex_destroy(a->mutex);
    pumpkin_set_data(NULL);
    xfree(a);
  }

  return 0;
}
