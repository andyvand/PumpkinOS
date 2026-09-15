/*
 * netinst - network install protocol server (engine).
 *
 * See netinst.h for the protocol.  Everything here only uses libpit, the
 * application supplies the storage callbacks.
 */

#ifdef ESP_PLATFORM
#include "esp32.h"
#endif

#include "sys.h"
#include "xalloc.h"
#include "debug.h"
#include "netinst.h"

#define TAG "NETINST"

#define HEADER_LEN      78          /* PalmOS database header */
#define CHUNK_LEN       8192        /* socket read / file write granularity */
#define LINE_LEN        160
#define IDLE_TIMEOUT    60000000LL  /* us without traffic before a client is dropped */
#define LIST_LINE_LEN   96

enum { stLine = 0, stHeader, stData };

struct netinst_t {
  netinst_ops_t ops;
  uint16_t port;
  uint32_t maxsize;
  char devname[NETINST_DEVNAME_LEN];
  char fileroot[NETINST_PATH_LEN];  /* empty: file commands disabled */

  int lsock;                        /* TCP listening socket, -1 when stopped */
  int usock;                        /* UDP discovery socket, -1 when unavailable */
  int csock;                        /* connected client, -1 when none */
  char client[NETINST_HOST_LEN];
  int64_t last_activity;

  int state;
  char line[LINE_LEN];
  int linelen;
  int line_overflow;

  /* transfer in progress */
  int raw;                          /* FPUT (plain file) instead of PUT (database) */
  char filename[NETINST_FILE_LEN];
  char path[NETINST_PATH_LEN];      /* FPUT destination */
  char dbname[NETINST_NAME_LEN];
  uint32_t total, received;
  uint8_t hdr[HEADER_LEN];
  int hdrlen;
  int file_open;

  uint8_t buf[CHUNK_LEN];
};

/* list building */
typedef struct {
  char *buf;
  int len, size, count;
} list_ctx_t;

/*
 * helpers
 */

static void str_copy(char *dst, const char *src, int size) {
  if (size <= 0) return;
  sys_strncpy(dst, src, size - 1);
  dst[size - 1] = 0;
}

void netinst_id2s(uint32_t id, char *s) {
  int i;

  s[0] = (id >> 24) & 0xFF;
  s[1] = (id >> 16) & 0xFF;
  s[2] = (id >> 8) & 0xFF;
  s[3] = id & 0xFF;
  s[4] = 0;
  for (i = 0; i < 4; i++) {
    if ((unsigned char)s[i] < 32 || (unsigned char)s[i] == 127) s[i] = '?';
  }
}

static void logmsg(netinst_t *s, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));

static void logmsg(netinst_t *s, const char *fmt, ...) {
  char msg[NETINST_LOG_LEN];
  sys_va_list ap;

  sys_va_start(ap, fmt);
  sys_vsnprintf(msg, sizeof(msg), fmt, ap);
  sys_va_end(ap);
  msg[sizeof(msg) - 1] = 0;

  debug(DEBUG_INFO, TAG, "%s", msg);
  if (s->ops.log) s->ops.log(s->ops.data, msg);
}

int netinst_valid_filename(const char *name) {
  int i, n;
  char c;

  if (name == NULL) return 0;
  n = sys_strlen(name);
  if (n < 5 || n >= NETINST_FILE_LEN) return 0;
  if (name[0] == '.' || name[0] == '-') return 0;

  for (i = 0; i < n; i++) {
    c = name[i];
    if (!(sys_isalnum((unsigned char)c) || c == '.' || c == '_' || c == '-' || c == ' ')) return 0;
    if (c == '.' && name[i + 1] == '.') return 0;
  }

  if (sys_strcasecmp(&name[n - 4], ".prc") && sys_strcasecmp(&name[n - 4], ".pdb")) return 0;

  return 1;
}

static int valid_path_char(char c) {
  if ((unsigned char)c < 32 || c == 127) return 0;
  switch (c) {
    case '\\': case ':': case '*': case '?': case '"': case '<': case '>': case '|':
      return 0;
  }
  return 1;
}

int netinst_valid_path(const char *root, const char *in, char *out, int outlen) {
  char comp[NETINST_PATH_LEN], canon[NETINST_PATH_LEN];
  const char *p, *r;
  int i, n, len, rlen;

  if (root == NULL || root[0] != '/' || in == NULL || out == NULL || outlen < 2) return -1;
  if (in[0] != '/') return -1;

  /* canonicalize: split on '/', drop empty components, reject . and .. */
  canon[0] = 0;
  len = 0;
  for (p = in; *p;) {
    while (*p == '/') p++;
    if (*p == 0) break;
    for (n = 0; p[n] && p[n] != '/'; n++) {
      if (n >= (int)sizeof(comp) - 1 || !valid_path_char(p[n])) return -1;
      comp[n] = p[n];
    }
    comp[n] = 0;
    p += n;
    if (!sys_strcmp(comp, ".") || !sys_strcmp(comp, "..")) return -1;
    if (comp[n - 1] == ' ' || comp[n - 1] == '.') return -1;   /* trailing blank/dot */
    if (len + 1 + n >= (int)sizeof(canon)) return -1;
    canon[len++] = '/';
    sys_memcpy(&canon[len], comp, n);
    len += n;
    canon[len] = 0;
  }
  if (len == 0) return -1;   /* the volume root itself is never inside the file root */

  /* must start with root (case-insensitive), followed by end or '/' */
  rlen = sys_strlen(root);
  while (rlen > 1 && root[rlen - 1] == '/') rlen--;
  if (len < rlen || sys_strncasecmp(canon, root, rlen) != 0) return -1;
  if (canon[rlen] != 0 && canon[rlen] != '/') return -1;

  /* write the root in its own spelling, then the rest */
  if (rlen + (len - rlen) >= outlen) return -1;
  for (i = 0, r = root; i < rlen; i++) out[i] = r[i];
  sys_memcpy(&out[rlen], &canon[rlen], len - rlen);
  out[len] = 0;

  return 0;
}

static uint32_t get4b(const uint8_t *p) {
  return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3];
}

int netinst_parse_header(const uint8_t *hdr, char *dbname, uint32_t *type, uint32_t *creator) {
  int i, n = -1;

  for (i = 0; i < NETINST_NAME_LEN; i++) {
    if (hdr[i] == 0) {
      n = i;
      break;
    }
    if (hdr[i] < 32 || hdr[i] == 127) return -1;
  }
  if (n <= 0) return -1;

  if (dbname) {
    sys_memcpy(dbname, hdr, n);
    dbname[n] = 0;
  }
  if (type) *type = get4b(&hdr[60]);
  if (creator) *creator = get4b(&hdr[64]);

  return 0;
}

/*
 * client connection
 */

static int send_raw(netinst_t *s, const char *buf, int len) {
  if (s->csock == -1) return -1;
  if (sys_write(s->csock, (uint8_t *)buf, len) != len) {
    debug(DEBUG_ERROR, TAG, "write to client failed");
    return -1;
  }
  return 0;
}

static int send_line(netinst_t *s, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));

static int send_line(netinst_t *s, const char *fmt, ...) {
  char buf[LIST_LINE_LEN + 32];
  sys_va_list ap;
  int n;

  sys_va_start(ap, fmt);
  n = sys_vsnprintf(buf, sizeof(buf) - 2, fmt, ap);
  sys_va_end(ap);
  if (n < 0) return -1;
  if (n > (int)sizeof(buf) - 2) n = sizeof(buf) - 2;
  buf[n++] = '\n';
  buf[n] = 0;

  debug(DEBUG_TRACE, TAG, "> %s", buf);
  return send_raw(s, buf, n);
}

static void abort_transfer(netinst_t *s) {
  char err[NETINST_ERR_LEN];

  if (s->file_open) {
    err[0] = 0;
    s->ops.end_file(s->ops.data, 0, err, sizeof(err));
    s->file_open = 0;
  }
  s->state = stLine;
  s->total = s->received = 0;
  s->hdrlen = 0;
  s->raw = 0;
}

static void close_client(netinst_t *s, const char *why) {
  if (s->csock == -1) return;

  if (s->state != stLine) {
    logmsg(s, "%s aborted", s->filename);
  }
  abort_transfer(s);

  sys_socket_shutdown(s->csock, SYS_SHUTDOWN_RDWR);
  sys_close(s->csock);
  s->csock = -1;
  s->linelen = 0;
  s->line_overflow = 0;
  if (why) logmsg(s, "%s %s", s->client, why);
  s->client[0] = 0;
}

static void list_cb(void *cbdata, const netinst_db_t *db) {
  list_ctx_t *ctx = (list_ctx_t *)cbdata;
  char *p;
  int n;

  if (ctx->buf == NULL) return;

  if (ctx->len + LIST_LINE_LEN + 1 > ctx->size) {
    n = ctx->size * 2;
    if ((p = xrealloc(ctx->buf, n)) == NULL) {
      xfree(ctx->buf);
      ctx->buf = NULL;
      return;
    }
    ctx->buf = p;
    ctx->size = n;
  }

  n = sys_snprintf(&ctx->buf[ctx->len], LIST_LINE_LEN, "%08X %08X %04X %u %u %s\n",
                   db->type, db->creator, db->attributes, db->version, db->size, db->name);
  if (n < 0) return;
  if (n >= LIST_LINE_LEN) {
    n = LIST_LINE_LEN - 1;
    ctx->buf[ctx->len + n - 1] = '\n';
  }
  ctx->len += n;
  ctx->count++;
}

static void cmd_list(netinst_t *s) {
  list_ctx_t ctx;

  ctx.size = 4096;
  ctx.len = 0;
  ctx.count = 0;
  if ((ctx.buf = xmalloc(ctx.size)) == NULL) {
    send_line(s, "-ERR out of memory");
    return;
  }

  if (s->ops.list == NULL || s->ops.list(s->ops.data, list_cb, &ctx) < 0 || ctx.buf == NULL) {
    if (ctx.buf) xfree(ctx.buf);
    send_line(s, "-ERR could not list databases");
    return;
  }

  if (send_line(s, "+LIST %d", ctx.count) == 0 && ctx.len > 0) {
    send_raw(s, ctx.buf, ctx.len);
  }
  xfree(ctx.buf);
  debug(DEBUG_INFO, TAG, "%s: listed %d databases", s->client, ctx.count);
}

static void cmd_put(netinst_t *s, char *args) {
  char *p, *name;
  unsigned long size;

  /* PUT <size> <file name> */
  size = sys_strtoul(args, &p, 10);
  if (p == args || *p != ' ') {
    send_line(s, "-ERR usage: PUT <size> <filename>");
    return;
  }
  while (*p == ' ') p++;
  name = p;

  if (!netinst_valid_filename(name)) {
    send_line(s, "-ERR invalid file name (use name.prc or name.pdb)");
    return;
  }
  if (size < HEADER_LEN) {
    send_line(s, "-ERR file too small");
    return;
  }
  if (size > s->maxsize) {
    send_line(s, "-ERR file too large (max %u bytes)", s->maxsize);
    return;
  }

  str_copy(s->filename, name, sizeof(s->filename));
  s->raw = 0;
  s->total = (uint32_t)size;
  s->received = 0;
  s->hdrlen = 0;
  s->dbname[0] = 0;
  s->state = stHeader;

  if (send_line(s, "+GO") == 0) {
    logmsg(s, "receiving %s (%lu)", name, size);
  }
}

/*
 * File commands
 */

static const char *basename_of(const char *path) {
  const char *b = path, *p;

  for (p = path; *p; p++) {
    if (*p == '/' && p[1]) b = p + 1;
  }
  return b;
}

/* validate a path argument against the file root; sends -ERR and returns -1 on failure */
static int file_path_arg(netinst_t *s, const char *arg, char *out, int outlen, int allow_root) {
  while (*arg == ' ') arg++;

  if (s->fileroot[0] == 0 || s->ops.list_dir == NULL) {
    send_line(s, "-ERR file commands are not supported");
    return -1;
  }
  if (arg[0] == 0) {
    if (!allow_root) {
      send_line(s, "-ERR missing path");
      return -1;
    }
    str_copy(out, s->fileroot, outlen);
    return 0;
  }
  if (netinst_valid_path(s->fileroot, arg, out, outlen) != 0) {
    send_line(s, "-ERR invalid path (must be inside %s)", s->fileroot);
    return -1;
  }
  if (!allow_root && !sys_strcmp(out, s->fileroot)) {
    send_line(s, "-ERR the root directory itself cannot be changed");
    return -1;
  }
  return 0;
}

static int finish_file(netinst_t *s);

static void cmd_fput(netinst_t *s, char *args) {
  char err[NETINST_ERR_LEN];
  char *p;
  unsigned long size;

  /* FPUT <size> <path> */
  size = sys_strtoul(args, &p, 10);
  if (p == args || *p != ' ') {
    send_line(s, "-ERR usage: FPUT <size> <path>");
    return;
  }
  if (file_path_arg(s, p, s->path, sizeof(s->path), 0) != 0) return;
  if (s->ops.begin_upload == NULL) {
    send_line(s, "-ERR uploads are not supported");
    return;
  }
  if (size > s->maxsize) {
    send_line(s, "-ERR file too large (max %u bytes)", s->maxsize);
    return;
  }

  err[0] = 0;
  if (s->ops.begin_upload(s->ops.data, s->path, (uint32_t)size, err, sizeof(err)) != 0) {
    send_line(s, "-ERR %s", err[0] ? err : "cannot create file");
    logmsg(s, "%s: %s", basename_of(s->path), err);
    return;
  }
  s->file_open = 1;
  s->raw = 1;
  str_copy(s->filename, basename_of(s->path), sizeof(s->filename));
  s->total = (uint32_t)size;
  s->received = 0;
  s->hdrlen = 0;
  s->dbname[0] = 0;

  if (send_line(s, "+GO") != 0) return;
  logmsg(s, "receiving %s (%lu)", s->filename, size);

  if (size == 0) {
    finish_file(s);          /* empty file: nothing follows */
  } else {
    s->state = stData;
  }
}

static void cmd_mkdir(netinst_t *s, char *args) {
  char path[NETINST_PATH_LEN], err[NETINST_ERR_LEN];

  if (file_path_arg(s, args, path, sizeof(path), 0) != 0) return;
  if (s->ops.mkdir == NULL) {
    send_line(s, "-ERR not supported");
    return;
  }
  err[0] = 0;
  if (s->ops.mkdir(s->ops.data, path, err, sizeof(err)) == 0) {
    send_line(s, "+OK %s", path);
    logmsg(s, "created %s", basename_of(path));
  } else {
    send_line(s, "-ERR %s", err[0] ? err : "cannot create directory");
    logmsg(s, "mkdir %s: %s", basename_of(path), err);
  }
}

static void cmd_fdel(netinst_t *s, char *args) {
  char path[NETINST_PATH_LEN], err[NETINST_ERR_LEN];

  if (file_path_arg(s, args, path, sizeof(path), 0) != 0) return;
  if (s->ops.remove_path == NULL) {
    send_line(s, "-ERR not supported");
    return;
  }
  err[0] = 0;
  if (s->ops.remove_path(s->ops.data, path, err, sizeof(err)) == 0) {
    send_line(s, "+OK");
    logmsg(s, "deleted %s", basename_of(path));
  } else {
    send_line(s, "-ERR %s", err[0] ? err : "cannot delete");
    logmsg(s, "delete %s: %s", basename_of(path), err);
  }
}

static void dir_cb(void *cbdata, const char *name, int is_dir, uint32_t size) {
  list_ctx_t *ctx = (list_ctx_t *)cbdata;
  char *p;
  int n;

  if (ctx->buf == NULL) return;

  if (ctx->len + NETINST_PATH_LEN + 16 > ctx->size) {
    n = ctx->size * 2;
    if ((p = xrealloc(ctx->buf, n)) == NULL) {
      xfree(ctx->buf);
      ctx->buf = NULL;
      return;
    }
    ctx->buf = p;
    ctx->size = n;
  }

  n = sys_snprintf(&ctx->buf[ctx->len], NETINST_PATH_LEN + 16, "%c %u %s\n", is_dir ? 'D' : 'F', size, name);
  if (n < 0) return;
  if (n >= NETINST_PATH_LEN + 16) {
    n = NETINST_PATH_LEN + 15;
    ctx->buf[ctx->len + n - 1] = '\n';
  }
  ctx->len += n;
  ctx->count++;
}

static void cmd_ls(netinst_t *s, char *args) {
  char path[NETINST_PATH_LEN], err[NETINST_ERR_LEN];
  list_ctx_t ctx;
  int r;

  if (file_path_arg(s, args, path, sizeof(path), 1) != 0) return;

  ctx.size = 4096;
  ctx.len = 0;
  ctx.count = 0;
  if ((ctx.buf = xmalloc(ctx.size)) == NULL) {
    send_line(s, "-ERR out of memory");
    return;
  }

  err[0] = 0;
  r = s->ops.list_dir(s->ops.data, path, dir_cb, &ctx, err, sizeof(err));
  if (r < 0 || ctx.buf == NULL) {
    if (ctx.buf) xfree(ctx.buf);
    send_line(s, "-ERR %s", err[0] ? err : "cannot list directory");
    return;
  }

  if (send_line(s, "+LS %d %s", ctx.count, path) == 0 && ctx.len > 0) {
    send_raw(s, ctx.buf, ctx.len);
  }
  xfree(ctx.buf);
  debug(DEBUG_INFO, TAG, "%s: listed %d entries of %s", s->client, ctx.count, path);
}

static void cmd_del(netinst_t *s, char *name) {
  char err[NETINST_ERR_LEN];

  while (*name == ' ') name++;
  if (name[0] == 0 || sys_strlen(name) >= NETINST_NAME_LEN) {
    send_line(s, "-ERR usage: DEL <database name>");
    return;
  }
  if (s->ops.remove == NULL) {
    send_line(s, "-ERR not supported");
    return;
  }

  err[0] = 0;
  if (s->ops.remove(s->ops.data, name, err, sizeof(err)) == 0) {
    send_line(s, "+OK");
    logmsg(s, "deleted %s", name);
  } else {
    send_line(s, "-ERR %s", err[0] ? err : "delete failed");
    logmsg(s, "delete %s: %s", name, err);
  }
}

/* returns -1 when the connection must be closed */
static int dispatch_line(netinst_t *s) {
  char *cmd, *args;
  int i;

  /* strip CR and trailing blanks */
  while (s->linelen > 0 && (s->line[s->linelen - 1] == '\r' || s->line[s->linelen - 1] == ' ')) s->linelen--;
  s->line[s->linelen] = 0;
  debug(DEBUG_TRACE, TAG, "< %s", s->line);

  cmd = s->line;
  while (*cmd == ' ') cmd++;
  if (*cmd == 0) return 0;   /* empty line */

  for (args = cmd; *args && *args != ' '; args++);
  if (*args) {
    *args++ = 0;
    while (*args == ' ') args++;
  }
  for (i = 0; cmd[i]; i++) cmd[i] = sys_toupper(cmd[i]);

  if (!sys_strcmp(cmd, "PING")) {
    return send_line(s, "+PONG");
  }
  if (!sys_strcmp(cmd, "LIST")) {
    cmd_list(s);
    return 0;
  }
  if (!sys_strcmp(cmd, "PUT")) {
    cmd_put(s, args);
    return 0;
  }
  if (!sys_strcmp(cmd, "DEL")) {
    cmd_del(s, args);
    return 0;
  }
  if (!sys_strcmp(cmd, "LS")) {
    cmd_ls(s, args);
    return 0;
  }
  if (!sys_strcmp(cmd, "MKDIR")) {
    cmd_mkdir(s, args);
    return 0;
  }
  if (!sys_strcmp(cmd, "FPUT")) {
    cmd_fput(s, args);
    return 0;
  }
  if (!sys_strcmp(cmd, "FDEL")) {
    cmd_fdel(s, args);
    return 0;
  }
  if (!sys_strcmp(cmd, "QUIT")) {
    send_line(s, "+BYE");
    return -1;
  }

  return send_line(s, "-ERR unknown command");
}

/* the header is complete: validate it and open the destination */
static int start_file(netinst_t *s) {
  char err[NETINST_ERR_LEN], stype[8], screator[8];
  uint32_t type, creator;

  if (netinst_parse_header(s->hdr, s->dbname, &type, &creator) != 0) {
    send_line(s, "-ERR not a PalmOS database");
    logmsg(s, "%s: not a database", s->filename);
    return -1;
  }

  err[0] = 0;
  if (s->ops.begin_file == NULL ||
      s->ops.begin_file(s->ops.data, s->filename, s->dbname, s->total, err, sizeof(err)) != 0) {
    send_line(s, "-ERR %s", err[0] ? err : "cannot create file");
    logmsg(s, "%s: %s", s->filename, err);
    return -1;
  }
  s->file_open = 1;

  if (s->ops.write_file(s->ops.data, s->hdr, HEADER_LEN, err, sizeof(err)) != 0) {
    send_line(s, "-ERR %s", err[0] ? err : "write failed");
    return -1;
  }
  s->received = HEADER_LEN;

  netinst_id2s(type, stype);
  netinst_id2s(creator, screator);
  debug(DEBUG_INFO, TAG, "%s: database \"%s\" type '%s' creator '%s'", s->client, s->dbname, stype, screator);

  if (s->received == s->total) {
    s->state = stLine;   /* degenerate: header only */
    return 1;
  }
  s->state = stData;
  return 0;
}

static int finish_file(netinst_t *s) {
  char err[NETINST_ERR_LEN];
  int r;

  err[0] = 0;
  r = s->ops.end_file(s->ops.data, 1, err, sizeof(err));
  s->file_open = 0;
  s->state = stLine;

  if (s->raw) {
    s->raw = 0;
    if (r == 0) {
      logmsg(s, "uploaded %s", s->filename);
      return send_line(s, "+OK %s", s->path);
    }
    logmsg(s, "%s: %s", s->filename, err);
    send_line(s, "-ERR %s", err[0] ? err : "upload failed");
    return 0;
  }

  if (r == 0) {
    logmsg(s, "installed %s", s->dbname);
    return send_line(s, "+OK %s", s->dbname);
  }

  logmsg(s, "%s: %s", s->filename, err);
  send_line(s, "-ERR %s", err[0] ? err : "install failed");
  return 0;
}

/* consume n received bytes according to the current state; -1 closes the connection */
static int feed(netinst_t *s, const uint8_t *buf, int n) {
  char err[NETINST_ERR_LEN];
  int i, k, r;

  for (i = 0; i < n;) {
    switch (s->state) {
      case stLine:
        for (; i < n; i++) {
          if (buf[i] == '\n') {
            i++;
            if (s->line_overflow) {
              s->line_overflow = 0;
              s->linelen = 0;
              if (send_line(s, "-ERR line too long") != 0) return -1;
            } else {
              r = dispatch_line(s);
              s->linelen = 0;
              if (r != 0) return -1;
            }
            break;
          }
          if (s->linelen < LINE_LEN - 1) {
            s->line[s->linelen++] = buf[i];
          } else {
            s->line_overflow = 1;
          }
        }
        break;

      case stHeader:
        k = HEADER_LEN - s->hdrlen;
        if (k > n - i) k = n - i;
        sys_memcpy(&s->hdr[s->hdrlen], &buf[i], k);
        s->hdrlen += k;
        i += k;
        if (s->hdrlen == HEADER_LEN) {
          r = start_file(s);
          if (r < 0) {
            abort_transfer(s);
            return -1;
          }
          if (r == 1) {
            if (finish_file(s) != 0) return -1;
          }
        }
        break;

      case stData:
        k = s->total - s->received;
        if (k > n - i) k = n - i;
        err[0] = 0;
        if (s->ops.write_file(s->ops.data, &buf[i], k, err, sizeof(err)) != 0) {
          send_line(s, "-ERR %s", err[0] ? err : "write failed");
          logmsg(s, "%s: %s", s->filename, err);
          abort_transfer(s);
          return -1;
        }
        s->received += k;
        i += k;
        if (s->received == s->total) {
          if (finish_file(s) != 0) return -1;
        }
        break;
    }
  }

  return 0;
}

/*
 * sockets
 */

static void service_discovery(netinst_t *s) {
  char host[NETINST_HOST_LEN], reply[NETINST_DEVNAME_LEN + 32];
  sys_timeval_t tv;
  int port, n, i;

  if (s->usock == -1) return;

  for (i = 0; i < 4; i++) {
    tv.tv_sec = 0;
    tv.tv_usec = 0;
    n = sys_socket_recvfrom(s->usock, host, sizeof(host), &port, s->buf, sizeof(s->buf) - 1, &tv);
    if (n <= 0) break;
    s->buf[n] = 0;

    if (n >= 8 && !sys_strncmp((char *)s->buf, "NETINST?", 8)) {
      n = sys_snprintf(reply, sizeof(reply), "NETINST! %u %s", s->port, s->devname);
      sys_socket_sendto(s->usock, host, port, (unsigned char *)reply, n);
      debug(DEBUG_INFO, TAG, "discovery request from %s:%d", host, port);
    }
  }
}

static void service_accept(netinst_t *s) {
  char host[NETINST_HOST_LEN];
  sys_timeval_t tv;
  int sock, port;

  tv.tv_sec = 0;
  tv.tv_usec = 0;
  if ((sock = sys_socket_accept(s->lsock, host, sizeof(host), &port, &tv)) <= 0) return;

  if (s->csock != -1) {
    debug(DEBUG_INFO, TAG, "refusing %s:%d, busy", host, port);
    sys_write(sock, (uint8_t *)"-ERR busy\n", 10);
    sys_close(sock);
    return;
  }

  s->csock = sock;
  str_copy(s->client, host, sizeof(s->client));
  s->last_activity = sys_get_clock();
  s->state = stLine;
  s->linelen = 0;
  s->line_overflow = 0;

  logmsg(s, "%s connected", s->client);
  if (send_line(s, "+NETINST %d %u %s", NETINST_VERSION, s->maxsize, s->devname) != 0) {
    close_client(s, "disconnected");
  }
}

/* returns 1 when there may be more data to read */
static int service_client(netinst_t *s, int64_t deadline) {
  int r, nread, want;

  while (s->csock != -1) {
    want = sizeof(s->buf);
    if (s->state == stData) {
      want = s->total - s->received;
      if (want > (int)sizeof(s->buf)) want = sizeof(s->buf);
    }

    r = sys_read_timeout(s->csock, s->buf, want, &nread, 0);
    if (r < 0) {
      close_client(s, "connection error");
      return 0;
    }
    if (r == 0) {
      /* nothing available */
      if (sys_get_clock() - s->last_activity > IDLE_TIMEOUT) {
        close_client(s, "timeout");
      }
      return 0;
    }
    if (nread == 0) {
      close_client(s, "disconnected");
      return 0;
    }

    s->last_activity = sys_get_clock();
    if (feed(s, s->buf, nread) != 0) {
      close_client(s, "connection closed");
      return 0;
    }

    if (sys_get_clock() >= deadline) return 1;
  }

  return 0;
}

/*
 * public API
 */

netinst_t *netinst_create(const netinst_ops_t *ops, uint16_t port, uint32_t maxsize, const char *devname, const char *fileroot) {
  netinst_t *s;
  int n;

  if (ops == NULL || ops->begin_file == NULL || ops->write_file == NULL || ops->end_file == NULL) {
    debug(DEBUG_ERROR, TAG, "missing callbacks");
    return NULL;
  }

  if ((s = xcalloc(1, sizeof(netinst_t))) == NULL) return NULL;
  s->ops = *ops;
  s->port = port ? port : NETINST_DEFAULT_PORT;
  s->maxsize = maxsize;
  str_copy(s->devname, (devname && devname[0]) ? devname : "PumpkinOS", sizeof(s->devname));
  if (fileroot && fileroot[0] == '/') {
    str_copy(s->fileroot, fileroot, sizeof(s->fileroot));
    n = sys_strlen(s->fileroot);
    while (n > 1 && s->fileroot[n - 1] == '/') s->fileroot[--n] = 0;
  }
  s->lsock = s->usock = s->csock = -1;

  return s;
}

void netinst_destroy(netinst_t *s) {
  if (s) {
    netinst_stop(s);
    xfree(s);
  }
}

int netinst_start(netinst_t *s) {
  int port;

  if (s == NULL) return -1;
  if (s->lsock != -1) return 0;

  port = s->port;
  if ((s->lsock = sys_socket_bind("0.0.0.0", &port, IP_STREAM)) == -1) {
    debug(DEBUG_ERROR, TAG, "could not bind TCP port %d", port);
    logmsg(s, "cannot listen on port %d", port);
    return -1;
  }
  s->port = port;

  port = s->port;
  if ((s->usock = sys_socket_bind("0.0.0.0", &port, IP_DGRAM)) == -1) {
    debug(DEBUG_ERROR, TAG, "could not bind UDP port %d, discovery disabled", port);
  }

  logmsg(s, "listening on port %d", s->port);
  return 0;
}

void netinst_stop(netinst_t *s) {
  if (s == NULL) return;

  close_client(s, NULL);

  if (s->usock != -1) {
    sys_close(s->usock);
    s->usock = -1;
  }
  if (s->lsock != -1) {
    sys_close(s->lsock);
    s->lsock = -1;
    logmsg(s, "stopped");
  }
}

int netinst_running(netinst_t *s) {
  return s && s->lsock != -1;
}

int netinst_connected(netinst_t *s) {
  return s && s->csock != -1;
}

uint16_t netinst_port(netinst_t *s) {
  return s ? s->port : 0;
}

const char *netinst_client(netinst_t *s) {
  return (s && s->csock != -1) ? s->client : "";
}

int netinst_transfer(netinst_t *s, char *filename, int len, uint32_t *received, uint32_t *total) {
  if (s == NULL || s->csock == -1 || s->state == stLine) return 0;
  if (filename && len > 0) str_copy(filename, s->filename, len);
  if (received) *received = s->received;
  if (total) *total = s->total;
  return 1;
}

int netinst_poll(netinst_t *s, uint32_t budget_us) {
  int64_t deadline;

  if (s == NULL || s->lsock == -1) return -1;
  deadline = sys_get_clock() + budget_us;

  service_discovery(s);
  service_accept(s);

  return service_client(s, deadline);
}
