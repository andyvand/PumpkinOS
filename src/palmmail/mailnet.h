/*
 * mailnet - IMAP4 / SMTP client library for the PumpkinOS Mail application.
 *
 * This module only depends on libpit (sys_*, xalloc, debug, mutex) and on the
 * PumpkinOS secure provider (secure.h), so it can run in a worker thread
 * without touching any PalmOS UI API.
 */

#ifndef MAILNET_H
#define MAILNET_H

#include "sys.h"
#include "secure.h"

#ifdef __cplusplus
extern "C" {
#endif

/* connection security */
#define MAIL_SEC_NONE      0   /* plain TCP, no encryption */
#define MAIL_SEC_STARTTLS  1   /* plain TCP, upgraded with STARTTLS */
#define MAIL_SEC_TLS       2   /* implicit TLS (IMAPS 993 / SMTPS 465) */

/* incoming protocol */
#define MAIL_PROTO_IMAP    0
#define MAIL_PROTO_POP3    1

/* return codes */
#define MAIL_OK            0
#define MAIL_ERR_IO       -1   /* connect, read or write failed */
#define MAIL_ERR_TLS      -2   /* TLS handshake / certificate verification failed */
#define MAIL_ERR_NOTLS    -3   /* TLS requested but no secure provider is loaded */
#define MAIL_ERR_PROTO    -4   /* the server answered with an error (see mail_ctx_t.error) */
#define MAIL_ERR_AUTH     -5   /* login rejected */
#define MAIL_ERR_CANCEL   -6   /* cancelled by the user */
#define MAIL_ERR_MEM      -7   /* out of memory */
#define MAIL_ERR_ARG      -8   /* invalid arguments (missing host, empty recipients, ...) */

#define MAIL_ERROR_LEN    160
#define MAIL_STATUS_LEN    48

#define MAIL_HOST_LEN      96
#define MAIL_NAME_LEN      64
#define MAIL_ADDR_LEN      96
#define MAIL_USER_LEN      96
#define MAIL_PASS_LEN      64
#define MAIL_FOLDER_LEN    64

#define MAIL_FROM_LEN      64
#define MAIL_SUBJECT_LEN   96
#define MAIL_DATE_LEN      40
#define MAIL_UIDL_LEN      72

#define MAIL_MAX_FOLDERS   64

/* account settings; this struct is stored verbatim in the application preferences */
typedef struct {
  char name[MAIL_NAME_LEN];        /* display name used in From: */
  char email[MAIL_ADDR_LEN];       /* sender address */
  char user[MAIL_USER_LEN];        /* login name (IMAP and SMTP) */
  char pass[MAIL_PASS_LEN];        /* password (IMAP and SMTP) */
  char imap_host[MAIL_HOST_LEN];
  uint16_t imap_port;
  uint8_t imap_sec;                /* MAIL_SEC_* */
  uint8_t proto;                   /* MAIL_PROTO_*: protocol spoken to imap_host */
  char smtp_host[MAIL_HOST_LEN];
  uint16_t smtp_port;
  uint8_t smtp_sec;                /* MAIL_SEC_* */
  uint8_t pad2;
  uint16_t fetch_count;            /* number of most recent headers to download */
} mail_account_t;

/* one entry of the message list */
typedef struct {
  uint32_t uid;                    /* IMAP UID, or POP3 message number */
  char uidl[MAIL_UIDL_LEN];        /* POP3 unique id (empty for IMAP) */
  uint32_t size;
  uint8_t seen;
  uint8_t answered;
  uint8_t flagged;
  uint8_t deleted;
  char from[MAIL_FROM_LEN];        /* decoded, Latin-1 */
  char subject[MAIL_SUBJECT_LEN];  /* decoded, Latin-1 */
  char date[MAIL_DATE_LEN];        /* raw Date: header, shortened */
} mail_header_t;

typedef struct {
  char name[MAIL_FOLDER_LEN];      /* server-side mailbox name */
  uint8_t selectable;
} mail_folder_t;

/* the parsed top-level headers and body text of one message */
typedef struct {
  char from[128];
  char reply_to[128];
  char to[256];
  char cc[256];
  char subject[MAIL_SUBJECT_LEN * 2];
  char date[MAIL_DATE_LEN];
  char message_id[128];
  char *text;                      /* Latin-1 body text, '\n' line ends, xmalloc'd */
  int text_len;
  int truncated;                   /* body was cut at the fetch limit */
  int nattach;                     /* number of attachments (names are listed in text) */
} mail_message_t;

/* progress callback: called with a short status string whenever the operation
   enters a new phase; return non-zero to cancel the operation */
typedef int (*mail_progress_f)(void *data, const char *status);

/* shared operation context */
typedef struct {
  secure_provider_t *secure;       /* may be NULL: then only MAIL_SEC_NONE works */
  mail_progress_f progress;
  void *progress_data;
  volatile int *cancel;            /* optional: set to non-zero to abort */
  int timeout;                     /* seconds per network operation (0 = default) */
  char error[MAIL_ERROR_LEN];      /* human readable description of the last failure */
} mail_ctx_t;

/* an IMAP or POP3 session (account->proto decides): keeps the connection open between operations */
typedef struct mail_session_t mail_session_t;

/* the session keeps its own copy of *ctx; change it later through mail_session_ctx() */
mail_session_t *mail_session_create(mail_ctx_t *ctx, mail_account_t *account);
void mail_session_destroy(mail_session_t *s);
mail_ctx_t *mail_session_ctx(mail_session_t *s);
/* close the connection (the next operation reconnects) */
void mail_session_disconnect(mail_session_t *s);
/* the account may be changed while no operation is running */
void mail_session_set_account(mail_session_t *s, mail_account_t *account);

/* LIST all mailboxes; *folders is xmalloc'd (free with xfree) */
int mail_imap_list_folders(mail_session_t *s, mail_folder_t **folders, int *nfolders);

/* download the headers of the newest `max` messages of `folder`.
   *headers is xmalloc'd, newest message first; *total is the number of
   messages in the mailbox */
int mail_imap_fetch_headers(mail_session_t *s, const char *folder, int max,
                            mail_header_t **headers, int *nheaders, int *total);

/* download one message (at most `limit` bytes of it), parse it and mark it \Seen.
   The caller must call mail_message_free() on msg */
int mail_imap_fetch_message(mail_session_t *s, const char *folder, uint32_t uid,
                            int limit, mail_message_t *msg);

/* flag the message as \Deleted and expunge the mailbox */
int mail_imap_delete_message(mail_session_t *s, const char *folder, uint32_t uid);

void mail_message_free(mail_message_t *msg);

/* protocol independent entry points: dispatch to the IMAP or POP3 code
   according to the account's `proto`. For POP3 the folder is ignored (there
   is only the mail drop), the folder list is just "INBOX", the message is
   identified through hdr->uidl, and there is no server-side \Seen flag */
int mail_list_folders(mail_session_t *s, mail_folder_t **folders, int *nfolders);
int mail_fetch_headers(mail_session_t *s, const char *folder, int max,
                       mail_header_t **headers, int *nheaders, int *total);
int mail_fetch_message(mail_session_t *s, const char *folder, const mail_header_t *hdr,
                       int limit, mail_message_t *msg);
int mail_delete_message(mail_session_t *s, const char *folder, const mail_header_t *hdr);

/* POP3 (RFC 1939): USER/PASS login, STLS for MAIL_SEC_STARTTLS */
int mail_pop3_fetch_headers(mail_session_t *s, int max, mail_header_t **headers, int *nheaders, int *total);
int mail_pop3_fetch_message(mail_session_t *s, const char *uidl, int limit, mail_message_t *msg);
/* DELE + QUIT: the deletion is committed by closing the session */
int mail_pop3_delete_message(mail_session_t *s, const char *uidl);

/* send a message. `rcpts` is a comma separated list of addresses
   (display names allowed); `data` is the complete RFC 5322 message with
   CRLF line ends (see mail_build_message) */
int mail_smtp_send(mail_ctx_t *ctx, mail_account_t *account, const char *rcpts,
                   const char *data, int len);

/* build an RFC 5322 message with a Latin-1 text body.
   Returns an xmalloc'd buffer (CRLF line ends, dot-stuffing NOT applied) */
char *mail_build_message(mail_account_t *account, const char *to, const char *cc,
                         const char *subject, const char *in_reply_to,
                         const char *body, int *len);

/* helpers shared with the UI */
int mail_utf8_to_latin1(char *s);                   /* in place, returns new length */
void mail_decode_header(const char *in, char *out, int size); /* RFC 2047 -> Latin-1 */
int mail_extract_address(const char *in, char *out, int size); /* "N <a@b>" -> a@b */
void mail_short_date(const char *in, char *out, int size);   /* "Sat, 13 Sep 2026 ..." -> "13 Sep 2026 10:11" */
const char *mail_error_string(int code);

#ifdef __cplusplus
}
#endif

#endif /* MAILNET_H */
