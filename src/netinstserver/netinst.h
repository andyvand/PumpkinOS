/*
 * netinst - network install protocol server for PumpkinOS.
 *
 * This module only depends on libpit (sys_*, xalloc, debug): every storage
 * operation (writing the received file, listing and deleting databases) is
 * done through the netinst_ops_t callbacks supplied by the application, so
 * the engine can be exercised on the host without PalmOS and never calls a
 * PalmOS API itself.
 *
 * The engine is single threaded and non-blocking: the application calls
 * netinst_poll() regularly (from its event loop) and the engine services the
 * listening socket, the discovery socket and the connected client for at most
 * one time slice per call.
 *
 * Protocol (TCP, text lines terminated by '\n', binary payload after PUT):
 *
 *   server greeting:  +NETINST 1 <maxsize> <device name>
 *   PING              -> +PONG
 *   LIST              -> +LIST <n>            then n lines:
 *                        <type-hex8> <creator-hex8> <attr-hex4> <version> <size> <name>
 *   PUT <size> <file> -> +GO                  client sends <size> bytes
 *                     -> +OK <database name>  or -ERR <message>
 *   DEL <name>        -> +OK                  or -ERR <message>
 *   QUIT              -> +BYE                 connection is closed
 *
 * File commands (paths are absolute on the card volume and must lie inside the
 * file root given to netinst_create(), e.g. /PALM/Programs):
 *
 *   LS [path]         -> +LS <n> <path>       then n lines:  <D|F> <size> <name>
 *   MKDIR <path>      -> +OK <path>           (parents are created too)
 *   FPUT <size> <path>-> +GO                  client sends <size> bytes
 *                     -> +OK <path>           or -ERR <message>
 *   FDEL <path>       -> +OK                  file or empty directory
 *
 * Discovery (UDP, same port): a datagram "NETINST?" is answered with
 * "NETINST! <port> <device name>".
 */

#ifndef NETINST_H
#define NETINST_H

#include "sys.h"

#ifdef __cplusplus
extern "C" {
#endif

#define NETINST_VERSION       2
#define NETINST_DEFAULT_PORT  5150

#define NETINST_NAME_LEN      32     /* PalmOS database name (dmDBNameLength) */
#define NETINST_FILE_LEN      64     /* file name received from the client */
#define NETINST_ERR_LEN       96
#define NETINST_LOG_LEN       80
#define NETINST_DEVNAME_LEN   32
#define NETINST_HOST_LEN      64
#define NETINST_PATH_LEN      200    /* absolute path on the card volume */

/* one installed database, reported by the list callback */
typedef struct {
  char name[NETINST_NAME_LEN];
  uint32_t type;
  uint32_t creator;
  uint16_t attributes;
  uint16_t version;
  uint32_t size;
} netinst_db_t;

/* called by the application once for every database during ops->list() */
typedef void (*netinst_list_f)(void *cbdata, const netinst_db_t *db);

/* called by the application once for every entry during ops->list_dir() */
typedef void (*netinst_dir_f)(void *cbdata, const char *name, int is_dir, uint32_t size);

/* storage callbacks implemented by the application (all run in the caller's
   thread, i.e. the thread that calls netinst_poll()) */
typedef struct {
  void *data;

  /* open a new destination for `filename` (already validated: base name with
     .prc or .pdb extension); the database header (the first 78 bytes) has
     already been received and validated: `dbname` is its name. Return 0, or
     -1 with a message in err. */
  int (*begin_file)(void *data, const char *filename, const char *dbname, uint32_t size, char *err, int errlen);

  /* append len bytes; return 0 or -1 */
  int (*write_file)(void *data, const uint8_t *buf, int len, char *err, int errlen);

  /* ok=1: the file is complete, commit it (install it); ok=0: abort and discard.
     Return 0 or -1 (only meaningful when ok=1). */
  int (*end_file)(void *data, int ok, char *err, int errlen);

  /* call cb(cbdata, db) for every installed database; return the count or -1 */
  int (*list)(void *data, netinst_list_f cb, void *cbdata);

  /* delete the database called `name`; return 0 or -1 */
  int (*remove)(void *data, const char *name, char *err, int errlen);

  /* file commands (all optional: when NULL the command answers -ERR).
     Paths are absolute, validated and inside the file root. */

  /* open `path` for writing (truncate), creating missing parent directories;
     the following write_file/end_file calls refer to it. 0 or -1 */
  int (*begin_upload)(void *data, const char *path, uint32_t size, char *err, int errlen);
  /* create the directory and its missing parents; 0 or -1 (existing is fine) */
  int (*mkdir)(void *data, const char *path, char *err, int errlen);
  /* call cb for every entry of the directory; return the count or -1 */
  int (*list_dir)(void *data, const char *path, netinst_dir_f cb, void *cbdata, char *err, int errlen);
  /* delete a file or an empty directory; 0 or -1 */
  int (*remove_path)(void *data, const char *path, char *err, int errlen);

  /* optional: one line of human readable activity (no newline) */
  void (*log)(void *data, const char *msg);
} netinst_ops_t;

typedef struct netinst_t netinst_t;

/* the engine keeps a copy of *ops. `fileroot` is the directory the file
   commands are confined to (e.g. "/PALM/Programs"); NULL disables them */
netinst_t *netinst_create(const netinst_ops_t *ops, uint16_t port, uint32_t maxsize, const char *devname, const char *fileroot);
void netinst_destroy(netinst_t *s);

/* bind the TCP listening socket and the UDP discovery socket; 0 or -1 */
int netinst_start(netinst_t *s);
/* close everything (a connected client is dropped, a partial file discarded) */
void netinst_stop(netinst_t *s);

int netinst_running(netinst_t *s);          /* listening */
int netinst_connected(netinst_t *s);        /* a client is connected */
uint16_t netinst_port(netinst_t *s);        /* the port actually bound */
const char *netinst_client(netinst_t *s);   /* address of the connected client, or "" */

/* progress of the current transfer (0 when idle): returns 1 and fills the
   arguments when a PUT is in progress */
int netinst_transfer(netinst_t *s, char *filename, int len, uint32_t *received, uint32_t *total);

/* service the sockets without blocking for more than `budget_us`
   microseconds. Returns 1 when there is more work pending (data is flowing,
   poll again soon), 0 when idle, -1 when the server is not running. */
int netinst_poll(netinst_t *s, uint32_t budget_us);

/* helpers shared with the application */
int netinst_valid_filename(const char *name);           /* base name, .prc/.pdb */
/* canonicalize an absolute path and check that it lies inside `root`
   (case-insensitive match of the root components, which are then written in
   the spelling of `root`). Returns 0 and fills out, or -1 */
int netinst_valid_path(const char *root, const char *in, char *out, int outlen);
int netinst_parse_header(const uint8_t *hdr, char *dbname, uint32_t *type, uint32_t *creator);
void netinst_id2s(uint32_t id, char *s);                 /* 4-char id -> string (5 bytes) */

#ifdef __cplusplus
}
#endif

#endif /* NETINST_H */
