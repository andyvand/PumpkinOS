/*
 * mailnet - IMAP4rev1 / SMTP client library for the PumpkinOS Mail application.
 *
 * Transport security comes from the PumpkinOS secure provider
 * (secure_provider_t, see libpit/secure.h): a plain TCP socket is opened
 * with sys_socket_open_connect_timeout() and, for implicit TLS or after a
 * STARTTLS command, wrapped with provider->connect().  Everything here only
 * uses libpit, so it can run in a worker thread.
 */

#ifdef ESP_PLATFORM
#include "esp32.h"
#endif

#include "sys.h"
#include "xalloc.h"
#include "debug.h"
#include "mailnet.h"   /* includes secure.h, which has no include guard */

#define TAG "MAIL"

#define DEFAULT_TIMEOUT   30          /* seconds */
#define POLL_SLICE_US     200000      /* cancel check granularity */
#define CONN_BUFLEN       4096
#define LINE_LEN          2048
#define MAX_RCPTS         32
#define MAX_DEPTH         6           /* MIME nesting */
#define MAX_ATTACH_TEXT   1024

/*
 * Small helpers
 */

static int starts_with_ci(const char *s, const char *prefix) {
  return sys_strncasecmp(s, prefix, sys_strlen(prefix)) == 0;
}

static const char *skip_spaces(const char *s) {
  while (*s == ' ' || *s == '\t') s++;
  return s;
}

/* case-insensitive strstr */
static const char *strstr_ci(const char *haystack, const char *needle) {
  int n = sys_strlen(needle);

  if (n == 0) return haystack;
  for (; *haystack; haystack++) {
    if (sys_strncasecmp(haystack, needle, n) == 0) return haystack;
  }
  return NULL;
}

/* case-insensitive search limited to [haystack, end) */
static char *strstr_ci_len(char *haystack, char *end, const char *needle) {
  int n = sys_strlen(needle);

  for (; haystack + n <= end; haystack++) {
    if (sys_strncasecmp(haystack, needle, n) == 0) return haystack;
  }
  return NULL;
}

static void str_copy(char *dst, const char *src, int size) {
  if (size <= 0) return;
  sys_strncpy(dst, src, size - 1);
  dst[size - 1] = 0;
}

static void str_ncopy(char *dst, const char *src, int n, int size) {
  if (size <= 0) return;
  if (n > size - 1) n = size - 1;
  if (n < 0) n = 0;
  sys_memcpy(dst, src, n);
  dst[n] = 0;
}

static void str_lower(char *s) {
  for (; *s; s++) *s = sys_tolower(*s);
}

static void str_trim(char *s) {
  int n = sys_strlen(s);
  char *p = s;

  while (n > 0 && sys_isspace((unsigned char)s[n - 1])) s[--n] = 0;
  while (*p && sys_isspace((unsigned char)*p)) p++;
  if (p != s) sys_memmove(s, p, sys_strlen(p) + 1);
}

static int check_cancel(mail_ctx_t *ctx) {
  return ctx && ctx->cancel && *ctx->cancel;
}

static void set_error(mail_ctx_t *ctx, const char *msg, const char *detail) {
  if (ctx == NULL) return;
  if (detail && detail[0]) {
    sys_snprintf(ctx->error, sizeof(ctx->error), "%s: %s", msg, detail);
  } else {
    str_copy(ctx->error, msg, sizeof(ctx->error));
  }
  debug(DEBUG_ERROR, TAG, "%s", ctx->error);
}

/* report a new phase; returns MAIL_ERR_CANCEL when the user gave up */
static int progress(mail_ctx_t *ctx, const char *status) {
  if (check_cancel(ctx)) return MAIL_ERR_CANCEL;
  if (ctx && ctx->progress) {
    if (ctx->progress(ctx->progress_data, status)) return MAIL_ERR_CANCEL;
  }
  debug(DEBUG_INFO, TAG, "%s", status);
  return MAIL_OK;
}

const char *mail_error_string(int code) {
  switch (code) {
    case MAIL_OK:         return "OK";
    case MAIL_ERR_IO:     return "Network error";
    case MAIL_ERR_TLS:    return "Secure connection failed";
    case MAIL_ERR_NOTLS:  return "No TLS provider loaded";
    case MAIL_ERR_PROTO:  return "Server error";
    case MAIL_ERR_AUTH:   return "Login failed";
    case MAIL_ERR_CANCEL: return "Cancelled";
    case MAIL_ERR_MEM:    return "Out of memory";
    case MAIL_ERR_ARG:    return "Invalid settings";
  }
  return "Unknown error";
}

/*
 * Character set handling.  PalmOS fonts are Latin-1, so everything that is
 * shown is converted to Latin-1; characters outside of it become '?'.
 */

int mail_utf8_to_latin1(char *s) {
  unsigned char *in = (unsigned char *)s, *out = (unsigned char *)s;
  uint32_t cp;
  int n, i, valid;

  while (*in) {
    if (*in < 0x80) {
      *out++ = *in++;
      continue;
    }
    if ((*in & 0xE0) == 0xC0) { n = 1; cp = *in & 0x1F; }
    else if ((*in & 0xF0) == 0xE0) { n = 2; cp = *in & 0x0F; }
    else if ((*in & 0xF8) == 0xF0) { n = 3; cp = *in & 0x07; }
    else { n = 0; cp = 0; }

    valid = n > 0;
    for (i = 1; valid && i <= n; i++) {
      if ((in[i] & 0xC0) != 0x80) valid = 0;
      else cp = (cp << 6) | (in[i] & 0x3F);
    }
    if (n == 1 && cp < 0x80) valid = 0;      /* overlong */

    if (!valid) {
      /* not UTF-8: assume it already is Latin-1 */
      *out++ = *in++;
      continue;
    }
    in += n + 1;

    if (cp == 0xA0) *out++ = ' ';
    else if (cp < 0x100) *out++ = (unsigned char)cp;
    else {
      switch (cp) {
        case 0x2018: case 0x2019: case 0x201A: case 0x2032: *out++ = '\''; break;
        case 0x201C: case 0x201D: case 0x201E: case 0x2033: *out++ = '"'; break;
        case 0x2013: case 0x2014: case 0x2212: *out++ = '-'; break;
        case 0x2026: *out++ = '.'; *out++ = '.'; *out++ = '.'; break;
        case 0x2022: *out++ = '*'; break;
        case 0x20AC: *out++ = 'E'; break;   /* euro sign is not in Latin-1 */
        case 0x2122: *out++ = 'T'; *out++ = 'M'; break;
        case 0x200B: case 0x200C: case 0x200D: case 0xFEFF: break; /* zero width */
        default: *out++ = '?'; break;
      }
    }
  }
  *out = 0;

  return (int)((char *)out - s);
}

/* convert a decoded body from `charset` to Latin-1 (in place) */
static int convert_charset(char *s, const char *charset) {
  if (charset == NULL || charset[0] == 0) return sys_strlen(s);
  if (strstr_ci(charset, "utf-8") || strstr_ci(charset, "utf8")) {
    return mail_utf8_to_latin1(s);
  }
  /* iso-8859-1, iso-8859-15, windows-1252, us-ascii and friends are close
     enough to Latin-1 to be shown as they are; other charsets are left alone */
  return sys_strlen(s);
}

/*
 * Base64 / quoted-printable
 */

static const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static int b64_value(int c) {
  if (c >= 'A' && c <= 'Z') return c - 'A';
  if (c >= 'a' && c <= 'z') return c - 'a' + 26;
  if (c >= '0' && c <= '9') return c - '0' + 52;
  if (c == '+' || c == '-') return 62;
  if (c == '/' || c == '_') return 63;
  return -1;
}

/* decodes in place (output is never longer than input); returns length */
static int base64_decode(char *data, int len) {
  unsigned char *in = (unsigned char *)data, *out = (unsigned char *)data;
  uint32_t acc = 0;
  int i, v, bits = 0;

  for (i = 0; i < len; i++) {
    if (in[i] == '=') break;
    if ((v = b64_value(in[i])) < 0) continue;   /* skip CRLF and garbage */
    acc = (acc << 6) | v;
    bits += 6;
    if (bits >= 8) {
      bits -= 8;
      *out++ = (acc >> bits) & 0xFF;
    }
  }

  return (int)((char *)out - data);
}

/* out must have room for 4*((len+2)/3)+1 bytes */
static int base64_encode(const unsigned char *in, int len, char *out) {
  int i, o = 0;
  uint32_t v;

  for (i = 0; i < len; i += 3) {
    v = in[i] << 16;
    if (i + 1 < len) v |= in[i + 1] << 8;
    if (i + 2 < len) v |= in[i + 2];
    out[o++] = b64chars[(v >> 18) & 63];
    out[o++] = b64chars[(v >> 12) & 63];
    out[o++] = (i + 1 < len) ? b64chars[(v >> 6) & 63] : '=';
    out[o++] = (i + 2 < len) ? b64chars[v & 63] : '=';
  }
  out[o] = 0;

  return o;
}

static int hex_value(int c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

/* quoted-printable decode in place; `header` selects RFC 2047 "Q" rules ('_' is a space) */
static int qp_decode(char *data, int len, int header) {
  char *in = data, *out = data, *end = data + len;
  int h, l;

  while (in < end) {
    if (*in == '=') {
      if (in + 1 < end && (in[1] == '\r' || in[1] == '\n')) {
        /* soft line break */
        in++;
        if (in < end && *in == '\r') in++;
        if (in < end && *in == '\n') in++;
        continue;
      }
      if (in + 2 < end && (h = hex_value(in[1])) >= 0 && (l = hex_value(in[2])) >= 0) {
        *out++ = (char)((h << 4) | l);
        in += 3;
        continue;
      }
      *out++ = *in++;
    } else if (header && *in == '_') {
      *out++ = ' ';
      in++;
    } else {
      *out++ = *in++;
    }
  }

  return (int)(out - data);
}

/*
 * RFC 2047 header decoding: =?charset?B|Q?text?=
 */

void mail_decode_header(const char *in, char *out, int size) {
  const char *p = in, *q, *cs, *enc, *text, *textend;
  char charset[32], *tmp;
  int o = 0, n, i, cslen;

  if (size <= 0) return;
  out[0] = 0;

  while (*p && o < size - 1) {
    if (p[0] == '=' && p[1] == '?') {
      cs = p + 2;
      q = sys_strchr(cs, '?');
      if (q && (q[1] == 'B' || q[1] == 'b' || q[1] == 'Q' || q[1] == 'q') && q[2] == '?') {
        enc = q + 1;
        text = q + 3;
        textend = sys_strstr(text, "?=");
        if (textend) {
          cslen = (int)(q - cs);
          str_ncopy(charset, cs, cslen, sizeof(charset));
          /* RFC 2231 language suffix: charset*lang */
          if ((tmp = sys_strchr(charset, '*')) != NULL) *tmp = 0;

          n = (int)(textend - text);
          if ((tmp = xmalloc(n + 1)) != NULL) {
            sys_memcpy(tmp, text, n);
            tmp[n] = 0;
            if (*enc == 'B' || *enc == 'b') n = base64_decode(tmp, n);
            else n = qp_decode(tmp, n, 1);
            tmp[n] = 0;
            /* decoded text may contain NULs; cut there */
            n = convert_charset(tmp, charset);
            for (i = 0; i < n && o < size - 1; i++) out[o++] = tmp[i];
            xfree(tmp);
          }
          p = textend + 2;
          /* whitespace between two encoded words is not significant */
          q = skip_spaces(p);
          if (q[0] == '=' && q[1] == '?') p = q;
          continue;
        }
      }
    }
    if (*p == '\r' || *p == '\n' || *p == '\t') {
      /* folded header: collapse into one space */
      if (o > 0 && out[o - 1] != ' ') out[o++] = ' ';
      p++;
      continue;
    }
    out[o++] = *p++;
  }
  out[o] = 0;

  /* raw 8-bit headers are usually UTF-8 */
  mail_utf8_to_latin1(out);
  str_trim(out);
}

/* RFC 2047 "Q" encode a Latin-1 string if it has non-ASCII characters,
   otherwise copy it; out must have room for about 3*len+40 bytes */
static void encode_header_value(const char *in, char *out, int size) {
  static const char hex[] = "0123456789ABCDEF";
  const unsigned char *p;
  int ascii = 1, o = 0, words = 0, count = 0;

  for (p = (const unsigned char *)in; *p; p++) {
    if (*p >= 0x80) { ascii = 0; break; }
  }
  if (ascii) {
    str_copy(out, in, size);
    return;
  }

  for (p = (const unsigned char *)in; *p && o < size - 24; p++) {
    if (count == 0) {
      /* start a new encoded word; fold the header between words */
      if (words > 0) {
        out[o++] = '\r'; out[o++] = '\n'; out[o++] = ' ';
      }
      sys_memcpy(out + o, "=?ISO-8859-1?Q?", 15);
      o += 15;
      words++;
    }
    if (*p == ' ') {
      out[o++] = '_';
    } else if (*p < 0x80 && *p > 0x20 && *p != '=' && *p != '?' && *p != '_') {
      out[o++] = *p;
    } else {
      out[o++] = '=';
      out[o++] = hex[*p >> 4];
      out[o++] = hex[*p & 15];
    }
    count++;
    if (count >= 18) {           /* keep encoded words under 75 characters */
      out[o++] = '?'; out[o++] = '=';
      count = 0;
    }
  }
  if (count > 0) {
    out[o++] = '?'; out[o++] = '=';
  }
  out[o] = 0;
}

/*
 * Header access on a raw RFC 5322 header block
 */

/* find header `name` in hdr[0..len), unfold its value into out; returns 1 if found */
static int header_get(const char *hdr, int len, const char *name, char *out, int size) {
  const char *p = hdr, *end = hdr + len, *eol, *v;
  int nlen = sys_strlen(name), o = 0;

  if (size <= 0) return 0;
  out[0] = 0;

  while (p < end) {
    eol = p;
    while (eol < end && *eol != '\n') eol++;

    if (sys_strncasecmp(p, name, nlen) == 0 && p[nlen] == ':') {
      v = skip_spaces(p + nlen + 1);
      for (;;) {
        /* copy this line */
        while (v < eol && o < size - 1) {
          if (*v != '\r') out[o++] = *v;
          v++;
        }
        /* continuation line? */
        p = eol + 1;
        if (p >= end || (*p != ' ' && *p != '\t')) break;
        eol = p;
        while (eol < end && *eol != '\n') eol++;
        v = skip_spaces(p);
        if (o < size - 1) out[o++] = ' ';
      }
      out[o] = 0;
      str_trim(out);
      return 1;
    }
    p = eol + 1;
  }

  return 0;
}

/* returns the offset of the body (after the blank line); *hlen is the header length */
static int split_message(const char *data, int len, int *hlen) {
  int i;

  for (i = 0; i < len; i++) {
    if (data[i] == '\n') {
      if (i + 1 < len && data[i + 1] == '\n') { *hlen = i + 1; return i + 2; }
      if (i + 2 < len && data[i + 1] == '\r' && data[i + 2] == '\n') { *hlen = i + 1; return i + 3; }
    }
  }
  *hlen = len;
  return len;
}

/* get a parameter of a Content-Type like header value: param=value or param="value" */
static int header_param(const char *value, const char *param, char *out, int size) {
  const char *p = value, *q;
  int plen = sys_strlen(param), n;

  out[0] = 0;
  while ((p = sys_strchr(p, ';')) != NULL) {
    p = skip_spaces(p + 1);
    if (sys_strncasecmp(p, param, plen) == 0 && (p[plen] == '=' || p[plen] == '*')) {
      p = sys_strchr(p, '=');
      if (!p) return 0;
      p++;
      if (*p == '"') {
        p++;
        q = sys_strchr(p, '"');
        n = q ? (int)(q - p) : (int)sys_strlen(p);
      } else {
        q = p;
        while (*q && *q != ';' && !sys_isspace((unsigned char)*q)) q++;
        n = (int)(q - p);
      }
      str_ncopy(out, p, n, size);
      return 1;
    }
  }

  return 0;
}

void mail_short_date(const char *in, char *out, int size) {
  const char *p = in, *tok[8];
  int len[8], n = 0, first = -1, i;

  if (size <= 0) return;

  /* tokenize */
  while (*p && n < 8) {
    while (*p == ' ' || *p == '\t' || *p == ',') p++;
    if (!*p) break;
    tok[n] = p;
    while (*p && *p != ' ' && *p != '\t' && *p != ',') p++;
    len[n] = (int)(p - tok[n]);
    n++;
  }

  for (i = 0; i < n; i++) {
    if (sys_isdigit((unsigned char)tok[i][0]) && len[i] <= 2) { first = i; break; }
  }

  if (first >= 0 && first + 3 < n && !sys_isdigit((unsigned char)tok[first + 1][0])) {
    /* day month year time */
    sys_snprintf(out, size, "%.*s %.*s %.*s %.*s",
      len[first], tok[first], len[first + 1] > 3 ? 3 : len[first + 1], tok[first + 1],
      len[first + 2], tok[first + 2], len[first + 3] > 5 ? 5 : len[first + 3], tok[first + 3]);
  } else {
    str_copy(out, in, size);
  }
}

/* extract the bare address out of "Display Name <user@host>" */
int mail_extract_address(const char *in, char *out, int size) {
  const char *lt, *gt;
  int n;

  if (size <= 0) return 0;
  lt = sys_strchr(in, '<');
  gt = lt ? sys_strchr(lt, '>') : NULL;

  if (lt && gt && gt > lt) {
    str_ncopy(out, lt + 1, (int)(gt - lt - 1), size);
  } else {
    str_copy(out, in, size);
  }
  str_trim(out);

  /* strip stray quotes */
  n = sys_strlen(out);
  if (n >= 2 && out[0] == '"' && out[n - 1] == '"') {
    sys_memmove(out, out + 1, n - 2);
    out[n - 2] = 0;
  }

  return out[0] != 0 && sys_strchr(out, '@') != NULL;
}

/* split "a@b, Name <c@d>; e@f" into bare addresses */
static int split_recipients(const char *list, char addrs[][MAIL_ADDR_LEN], int max) {
  const char *p = list, *start;
  char item[192];
  int n = 0, quoted = 0, angle = 0, len;

  while (*p && n < max) {
    while (*p == ' ' || *p == ',' || *p == ';' || *p == '\n' || *p == '\r') p++;
    if (!*p) break;
    start = p;
    quoted = angle = 0;
    while (*p) {
      if (*p == '"') quoted = !quoted;
      else if (!quoted && *p == '<') angle = 1;
      else if (!quoted && *p == '>') angle = 0;
      else if (!quoted && !angle && (*p == ',' || *p == ';' || *p == '\n')) break;
      p++;
    }
    len = (int)(p - start);
    str_ncopy(item, start, len, sizeof(item));
    if (mail_extract_address(item, addrs[n], MAIL_ADDR_LEN)) n++;
  }

  return n;
}

/*
 * Connection layer
 */

typedef struct {
  mail_ctx_t *ctx;
  int fd;
  secure_provider_t *sp;
  secure_config_t *sc;
  secure_t *s;
  char *buf;
  int len, pos;
  int eof;
  int timeout;
  char host[MAIL_HOST_LEN];
  int port;
} conn_t;

static int conn_timeout(mail_ctx_t *ctx) {
  return (ctx && ctx->timeout > 0) ? ctx->timeout : DEFAULT_TIMEOUT;
}

static conn_t *conn_new(mail_ctx_t *ctx) {
  conn_t *c;

  if ((c = xcalloc(1, sizeof(conn_t))) == NULL) return NULL;
  if ((c->buf = xmalloc(CONN_BUFLEN)) == NULL) {
    xfree(c);
    return NULL;
  }
  c->ctx = ctx;
  c->fd = -1;
  c->timeout = conn_timeout(ctx);

  return c;
}

static void conn_close(conn_t *c) {
  if (c == NULL) return;
  if (c->s && c->sp) c->sp->close(c->s);
  if (c->sc && c->sp) c->sp->destroy(c->sc);
  if (c->fd >= 0) sys_close(c->fd);
  if (c->buf) xfree(c->buf);
  xfree(c);
}

/* upgrade the connection to TLS (implicit TLS or after STARTTLS) */
static int conn_tls(conn_t *c) {
  secure_provider_t *sp = c->ctx ? c->ctx->secure : NULL;
  int r;

  if (sp == NULL) {
    set_error(c->ctx, "No TLS provider is loaded (liblopenssl / libls2n / liblmbedtls)", NULL);
    return MAIL_ERR_NOTLS;
  }
  if (c->len > c->pos) {
    /* plaintext bytes arrived after the STARTTLS reply: protocol violation */
    set_error(c->ctx, "Unexpected data before TLS handshake", NULL);
    return MAIL_ERR_PROTO;
  }
  if ((r = progress(c->ctx, "Securing connection...")) != MAIL_OK) return r;

  c->sp = sp;
  if ((c->sc = sp->new(sp->cert, sp->key)) == NULL) {
    set_error(c->ctx, "Could not create TLS configuration", NULL);
    return MAIL_ERR_TLS;
  }
  if ((c->s = sp->connect(c->sc, c->host, c->port, c->fd)) == NULL) {
    sp->destroy(c->sc);
    c->sc = NULL;
    set_error(c->ctx, "TLS handshake or certificate verification failed for", c->host);
    return MAIL_ERR_TLS;
  }
  debug(DEBUG_INFO, TAG, "TLS connection to %s:%d established", c->host, c->port);

  return MAIL_OK;
}

static int conn_open(conn_t *c, const char *host, int port, int security) {
  char status[MAIL_STATUS_LEN];
  int r;

  if (host == NULL || host[0] == 0 || port <= 0) {
    set_error(c->ctx, "Server name or port is missing", NULL);
    return MAIL_ERR_ARG;
  }
  if (security != MAIL_SEC_NONE && (c->ctx == NULL || c->ctx->secure == NULL)) {
    set_error(c->ctx, "No TLS provider is loaded (liblopenssl / libls2n / liblmbedtls)", NULL);
    return MAIL_ERR_NOTLS;
  }

  str_copy(c->host, host, sizeof(c->host));
  c->port = port;
  sys_snprintf(status, sizeof(status), "Connecting to %.30s...", host);
  if ((r = progress(c->ctx, status)) != MAIL_OK) return r;

  if ((c->fd = sys_socket_open_connect_timeout(c->host, port, IP_STREAM, (uint32_t)c->timeout * 1000000)) == -1) {
    sys_snprintf(status, sizeof(status), "%.30s:%d", host, port);
    set_error(c->ctx, "Could not connect to", status);
    return MAIL_ERR_IO;
  }
  debug(DEBUG_INFO, TAG, "connected to %s:%d (fd %d)", c->host, port, c->fd);

  if (security == MAIL_SEC_TLS) {
    return conn_tls(c);
  }

  return MAIL_OK;
}

/* wait until data is available; 1 = readable, 0 = timeout, <0 = error/cancel */
static int conn_wait(conn_t *c) {
  int64_t left = (int64_t)c->timeout * 1000000;
  int r;

  while (left > 0) {
    if (check_cancel(c->ctx)) return MAIL_ERR_CANCEL;
    if (c->s) r = c->sp->peek(c->s, POLL_SLICE_US);
    else r = sys_select(c->fd, POLL_SLICE_US);
    if (r > 0) return 1;
    if (r < 0) return MAIL_ERR_IO;
    left -= POLL_SLICE_US;
  }

  return 0;
}

/* read more data into the buffer; >0 bytes read, 0 EOF, <0 error */
static int conn_fill(conn_t *c) {
  int r, n = 0, space;

  if (c->eof) return 0;

  if (c->pos > 0) {
    if (c->len > c->pos) sys_memmove(c->buf, c->buf + c->pos, c->len - c->pos);
    c->len -= c->pos;
    c->pos = 0;
  }
  space = CONN_BUFLEN - c->len;
  if (space <= 0) return MAIL_ERR_PROTO;   /* line too long, caller handles */

  if ((r = conn_wait(c)) <= 0) {
    if (r == 0) {
      set_error(c->ctx, "Timeout waiting for", c->host);
      return MAIL_ERR_IO;
    }
    if (r == MAIL_ERR_IO) set_error(c->ctx, "Connection lost to", c->host);
    return r;
  }

  if (c->s) {
    n = c->sp->read(c->s, c->buf + c->len, space);
    if (n < 0) {
      set_error(c->ctx, "TLS read failed from", c->host);
      return MAIL_ERR_IO;
    }
  } else {
    r = sys_read_timeout(c->fd, (uint8_t *)(c->buf + c->len), space, &n, POLL_SLICE_US);
    if (r < 0) {
      set_error(c->ctx, "Read failed from", c->host);
      return MAIL_ERR_IO;
    }
    if (r == 0) return conn_fill(c);   /* spurious wakeup */
  }

  if (n == 0) {
    c->eof = 1;
    return 0;
  }
  c->len += n;

  return n;
}

/* read one line (without CR/LF) into line[size]; over-long lines are truncated.
   Returns the line length, or <0 on error / EOF */
static int conn_readline(conn_t *c, char *line, int size) {
  int i, n, r, o = 0, done = 0;

  while (!done) {
    for (i = c->pos; i < c->len; i++) {
      if (c->buf[i] == '\n') {
        done = 1;
        break;
      }
    }
    n = i - c->pos;
    if (n > 0) {
      if (o + n > size - 1) n = size - 1 - o;
      if (n > 0) {
        sys_memcpy(line + o, c->buf + c->pos, n);
        o += n;
      }
    }
    c->pos = done ? i + 1 : i;

    if (!done) {
      if ((r = conn_fill(c)) <= 0) {
        if (r == 0) {
          set_error(c->ctx, "Connection closed by", c->host);
          return MAIL_ERR_IO;
        }
        if (r == MAIL_ERR_PROTO) {
          /* buffer full without a newline: drop it and keep going */
          c->pos = c->len = 0;
          continue;
        }
        return r;
      }
    }
  }

  while (o > 0 && (line[o - 1] == '\r' || line[o - 1] == '\n')) o--;
  line[o] = 0;

  return o;
}

/* read exactly n bytes; dst may be NULL to discard them */
static int conn_read_bytes(conn_t *c, char *dst, int n) {
  int avail, take, r, o = 0;

  while (o < n) {
    avail = c->len - c->pos;
    if (avail <= 0) {
      if ((r = conn_fill(c)) <= 0) {
        if (r == 0) set_error(c->ctx, "Connection closed by", c->host);
        return r == 0 ? MAIL_ERR_IO : r;
      }
      continue;
    }
    take = n - o;
    if (take > avail) take = avail;
    if (dst) sys_memcpy(dst + o, c->buf + c->pos, take);
    c->pos += take;
    o += take;
  }

  return MAIL_OK;
}

static int conn_write(conn_t *c, const char *data, int len) {
  int r, o = 0;

  while (o < len) {
    if (check_cancel(c->ctx)) return MAIL_ERR_CANCEL;
    if (c->s) r = c->sp->write(c->s, (char *)data + o, len - o);
    else r = sys_write(c->fd, (uint8_t *)data + o, len - o);
    if (r <= 0) {
      set_error(c->ctx, "Write failed to", c->host);
      return MAIL_ERR_IO;
    }
    o += r;
  }

  return MAIL_OK;
}

static int conn_writes(conn_t *c, const char *s) {
  return conn_write(c, s, sys_strlen(s));
}

/*
 * IMAP session
 */

struct mail_session_t {
  mail_ctx_t *ctx;                    /* always points to ctx_storage */
  mail_ctx_t ctx_storage;
  mail_account_t account;
  conn_t *conn;
  int tag;
  int fresh;                          /* connection was just established */
  char selected[MAIL_FOLDER_LEN];
  int exists;
  char line[LINE_LEN];
  char cmd[LINE_LEN];
  char result[MAIL_ERROR_LEN];
  /* POP3 */
  int pop3_count;                     /* messages in the mail drop (STAT) */
  int pop3_ncache;
  struct { uint32_t num; char uidl[MAIL_UIDL_LEN]; } *pop3_cache;   /* number <-> UIDL of listed messages */
};

static void pop3_cache_free(mail_session_t *s) {
  if (s->pop3_cache) xfree(s->pop3_cache);
  s->pop3_cache = NULL;
  s->pop3_ncache = 0;
  s->pop3_count = 0;
}

mail_session_t *mail_session_create(mail_ctx_t *ctx, mail_account_t *account) {
  mail_session_t *s;

  if ((s = xcalloc(1, sizeof(mail_session_t))) == NULL) return NULL;
  if (ctx) s->ctx_storage = *ctx;
  s->ctx = &s->ctx_storage;
  if (account) s->account = *account;

  return s;
}

mail_ctx_t *mail_session_ctx(mail_session_t *s) {
  return s ? s->ctx : NULL;
}

void mail_session_set_account(mail_session_t *s, mail_account_t *account) {
  if (s && account) {
    if (sys_memcmp(&s->account, account, sizeof(mail_account_t))) {
      mail_session_disconnect(s);
    }
    s->account = *account;
  }
}

void mail_session_disconnect(mail_session_t *s) {
  if (s && s->conn) {
    debug(DEBUG_INFO, TAG, "closing IMAP connection");
    conn_close(s->conn);
    s->conn = NULL;
    s->selected[0] = 0;
    s->exists = 0;
    pop3_cache_free(s);
  }
}

void mail_session_destroy(mail_session_t *s) {
  if (s) {
    if (s->conn) {
      /* best effort: tell the server we are leaving, but do not wait */
      conn_writes(s->conn, s->account.proto == MAIL_PROTO_POP3 ? "QUIT\r\n" : "A9999 LOGOUT\r\n");
    }
    mail_session_disconnect(s);
    xfree(s);
  }
}

/* append an IMAP quoted string to buf */
static void imap_quote(char *buf, int size, const char *s) {
  int o = sys_strlen(buf);

  if (o < size - 1) buf[o++] = '"';
  for (; *s && o < size - 3; s++) {
    if (*s == '"' || *s == '\\') buf[o++] = '\\';
    buf[o++] = *s;
  }
  if (o < size - 1) buf[o++] = '"';
  buf[o] = 0;
}

typedef int (*imap_untagged_f)(mail_session_t *s, char *line, void *data);

/* send a command and collect the response; untagged lines go to cb.
   Returns MAIL_OK on a tagged OK, MAIL_ERR_PROTO on NO/BAD (server text in s->result) */
static int imap_command(mail_session_t *s, const char *cmd, imap_untagged_f cb, void *data) {
  char tag[16];
  const char *p;
  int r, n, taglen;

  sys_snprintf(tag, sizeof(tag), "A%04d", ++s->tag);
  taglen = sys_strlen(tag);

  n = sys_snprintf(s->cmd, sizeof(s->cmd), "%s %s\r\n", tag, cmd);
  if (n >= (int)sizeof(s->cmd)) {
    set_error(s->ctx, "Command too long", NULL);
    return MAIL_ERR_ARG;
  }
  if (starts_with_ci(cmd, "LOGIN") || starts_with_ci(cmd, "AUTHENTICATE")) {
    debug(DEBUG_INFO, TAG, "C: %s LOGIN ****", tag);
  } else {
    debug(DEBUG_INFO, TAG, "C: %s %.120s", tag, cmd);
  }
  if ((r = conn_write(s->conn, s->cmd, n)) != MAIL_OK) return r;

  s->result[0] = 0;
  for (;;) {
    if ((r = conn_readline(s->conn, s->line, sizeof(s->line))) < 0) return r;
    debug(DEBUG_TRACE, TAG, "S: %.120s", s->line);

    if (sys_strncmp(s->line, tag, taglen) == 0 && s->line[taglen] == ' ') {
      p = s->line + taglen + 1;
      if (starts_with_ci(p, "OK")) {
        str_copy(s->result, skip_spaces(p + 2), sizeof(s->result));
        return MAIL_OK;
      }
      /* NO or BAD */
      if (starts_with_ci(p, "NO")) p += 2;
      else if (starts_with_ci(p, "BAD")) p += 3;
      p = skip_spaces(p);
      /* skip a response code like [ALERT] */
      if (*p == '[') {
        const char *q = sys_strchr(p, ']');
        if (q) p = skip_spaces(q + 1);
      }
      str_copy(s->result, p, sizeof(s->result));
      set_error(s->ctx, "Server said", s->result);
      return MAIL_ERR_PROTO;
    }

    if (s->line[0] == '*' || s->line[0] == '+') {
      if (cb && (r = cb(s, s->line, data)) < 0) return r;
    }
  }
}

/* "{123}" at the end of the line -> 123, otherwise -1 */
static int imap_literal_size(const char *line) {
  int n = sys_strlen(line), i;

  if (n < 3 || line[n - 1] != '}') return -1;
  for (i = n - 2; i >= 0 && sys_isdigit((unsigned char)line[i]); i--);
  if (i < 0 || line[i] != '{' || i == n - 2) return -1;
  return sys_atoi(line + i + 1);
}

static int imap_untagged_exists(mail_session_t *s, char *line, void *data) {
  char *p;

  /* "* 123 EXISTS" */
  if (line[0] == '*' && line[1] == ' ' && sys_isdigit((unsigned char)line[2])) {
    p = line + 2;
    while (sys_isdigit((unsigned char)*p)) p++;
    if (starts_with_ci(p, " EXISTS")) {
      s->exists = sys_atoi(line + 2);
    }
  }
  /* literals in untagged responses (rare here) must be consumed */
  {
    int n = imap_literal_size(line);
    if (n > 0) {
      int r = conn_read_bytes(s->conn, NULL, n);
      if (r != MAIL_OK) return r;
      return conn_readline(s->conn, s->line, sizeof(s->line)) < 0 ? MAIL_ERR_IO : 0;
    }
  }
  (void)data;
  return 0;
}

static int imap_connect(mail_session_t *s) {
  mail_account_t *a = &s->account;
  int r, preauth = 0;

  if ((s->conn = conn_new(s->ctx)) == NULL) return MAIL_ERR_MEM;

  if ((r = conn_open(s->conn, a->imap_host, a->imap_port, a->imap_sec)) != MAIL_OK) goto fail;

  /* greeting */
  if ((r = conn_readline(s->conn, s->line, sizeof(s->line))) < 0) goto fail;
  debug(DEBUG_INFO, TAG, "S: %.120s", s->line);
  if (!starts_with_ci(s->line, "* OK") && !starts_with_ci(s->line, "* PREAUTH")) {
    set_error(s->ctx, "Unexpected IMAP greeting", s->line);
    r = MAIL_ERR_PROTO;
    goto fail;
  }
  preauth = starts_with_ci(s->line, "* PREAUTH");

  if (a->imap_sec == MAIL_SEC_STARTTLS) {
    if ((r = imap_command(s, "STARTTLS", NULL, NULL)) != MAIL_OK) {
      if (r == MAIL_ERR_PROTO) set_error(s->ctx, "Server refused STARTTLS", s->result);
      goto fail;
    }
    if ((r = conn_tls(s->conn)) != MAIL_OK) goto fail;
  }

  if (!preauth) {
    if ((r = progress(s->ctx, "Logging in...")) != MAIL_OK) goto fail;
    str_copy(s->cmd, "LOGIN ", sizeof(s->cmd));
    imap_quote(s->cmd, sizeof(s->cmd), a->user);
    sys_strncat(s->cmd, " ", 2);
    imap_quote(s->cmd, sizeof(s->cmd), a->pass);
    /* imap_command copies s->cmd into itself: use a temporary copy */
    {
      char *login = xstrdup(s->cmd);
      if (login == NULL) { r = MAIL_ERR_MEM; goto fail; }
      r = imap_command(s, login, NULL, NULL);
      xfree(login);
    }
    if (r != MAIL_OK) {
      if (r == MAIL_ERR_PROTO) {
        set_error(s->ctx, "Login rejected", s->result);
        r = MAIL_ERR_AUTH;
      }
      goto fail;
    }
  }

  s->fresh = 1;
  s->selected[0] = 0;
  s->exists = 0;
  return MAIL_OK;

fail:
  conn_close(s->conn);
  s->conn = NULL;
  return r;
}

/* make sure we have a live, logged-in connection */
static int session_ensure(mail_session_t *s) {
  int r;

  s->fresh = 0;
  if (s->conn) {
    /* check that the server is still there */
    r = imap_command(s, "NOOP", imap_untagged_exists, NULL);
    if (r == MAIL_OK) return MAIL_OK;
    if (r == MAIL_ERR_CANCEL) return r;
    debug(DEBUG_INFO, TAG, "IMAP connection is stale, reconnecting");
    mail_session_disconnect(s);
    if (s->ctx) s->ctx->error[0] = 0;
  }

  return imap_connect(s);
}

static int session_select(mail_session_t *s, const char *folder, int force) {
  int r;

  if (folder == NULL || folder[0] == 0) folder = "INBOX";
  if (!force && s->selected[0] && sys_strcmp(s->selected, folder) == 0) return MAIL_OK;

  if ((r = progress(s->ctx, "Opening mailbox...")) != MAIL_OK) return r;
  str_copy(s->cmd, "SELECT ", sizeof(s->cmd));
  imap_quote(s->cmd, sizeof(s->cmd), folder);
  s->exists = 0;
  {
    char *cmd = xstrdup(s->cmd);
    if (cmd == NULL) return MAIL_ERR_MEM;
    r = imap_command(s, cmd, imap_untagged_exists, NULL);
    xfree(cmd);
  }
  if (r != MAIL_OK) {
    s->selected[0] = 0;
    if (r == MAIL_ERR_PROTO) set_error(s->ctx, "Could not open mailbox", s->result);
    return r;
  }
  str_copy(s->selected, folder, sizeof(s->selected));

  return MAIL_OK;
}

/*
 * FETCH response parsing
 */

typedef struct {
  uint32_t uid;
  uint32_t size;
  int seen, answered, flagged, deleted;
  char *literal;          /* BODY[...] literal */
  int literal_len;
  int literal_cap;        /* keep at most this many bytes */
  int truncated;
  int have;
} fetch_item_t;

static void parse_fetch_items(const char *p, fetch_item_t *it) {
  const char *q, *e;

  if ((q = sys_strstr(p, "UID ")) != NULL && (q == p || q[-1] == ' ' || q[-1] == '(')) {
    it->uid = (uint32_t)sys_strtoul(q + 4, NULL, 10);
  }
  if ((q = sys_strstr(p, "RFC822.SIZE ")) != NULL) {
    it->size = (uint32_t)sys_strtoul(q + 12, NULL, 10);
  }
  if ((q = sys_strstr(p, "FLAGS (")) != NULL) {
    q += 7;
    e = sys_strchr(q, ')');
    if (e) {
      char flags[128];
      str_ncopy(flags, q, (int)(e - q), sizeof(flags));
      if (strstr_ci(flags, "\\Seen")) it->seen = 1;
      if (strstr_ci(flags, "\\Answered")) it->answered = 1;
      if (strstr_ci(flags, "\\Flagged")) it->flagged = 1;
      if (strstr_ci(flags, "\\Deleted")) it->deleted = 1;
    }
  }
}

/* untagged callback: handles "* n FETCH (...)" including one literal */
static int imap_untagged_fetch(mail_session_t *s, char *line, void *data) {
  fetch_item_t *it = (fetch_item_t *)data;
  char *p;
  int n, keep, r;

  if (line[0] != '*') return 0;
  p = line + 1;
  while (*p == ' ') p++;
  while (sys_isdigit((unsigned char)*p)) p++;
  if (!starts_with_ci(p, " FETCH ")) {
    /* other untagged data: EXISTS, EXPUNGE, FLAGS, ... */
    return imap_untagged_exists(s, line, NULL);
  }

  it->have = 1;
  parse_fetch_items(p, it);

  /* a literal follows? */
  while ((n = imap_literal_size(s->line)) >= 0) {
    if (it->literal == NULL && it->literal_cap > 0) {
      keep = n > it->literal_cap ? it->literal_cap : n;
      if ((it->literal = xmalloc(keep + 1)) == NULL) return MAIL_ERR_MEM;
      if ((r = conn_read_bytes(s->conn, it->literal, keep)) != MAIL_OK) return r;
      it->literal[keep] = 0;
      it->literal_len = keep;
      if (n > keep) {
        it->truncated = 1;
        if ((r = conn_read_bytes(s->conn, NULL, n - keep)) != MAIL_OK) return r;
      }
    } else {
      if ((r = conn_read_bytes(s->conn, NULL, n)) != MAIL_OK) return r;
    }
    /* the rest of the FETCH response (usually ")") */
    if ((r = conn_readline(s->conn, s->line, sizeof(s->line))) < 0) return r;
    parse_fetch_items(s->line, it);
  }

  return 0;
}

static void fetch_item_free(fetch_item_t *it) {
  if (it->literal) xfree(it->literal);
  sys_memset(it, 0, sizeof(fetch_item_t));
}

/*
 * Header list
 */

typedef struct {
  mail_header_t *headers;
  int n, cap;
  fetch_item_t item;
} header_collect_t;

static int imap_untagged_header(mail_session_t *s, char *line, void *data) {
  header_collect_t *hc = (header_collect_t *)data;
  mail_header_t *h, *tmp;
  char value[256];
  int r;

  hc->item.literal_cap = 4096;
  if ((r = imap_untagged_fetch(s, line, &hc->item)) < 0) return r;
  if (!hc->item.have) return 0;

  if (hc->n >= hc->cap) {
    tmp = xrealloc(hc->headers, (hc->cap + 32) * sizeof(mail_header_t));
    if (tmp == NULL) return MAIL_ERR_MEM;
    hc->headers = tmp;
    hc->cap += 32;
  }
  h = &hc->headers[hc->n++];
  sys_memset(h, 0, sizeof(mail_header_t));
  h->uid = hc->item.uid;
  h->size = hc->item.size;
  h->seen = hc->item.seen;
  h->answered = hc->item.answered;
  h->flagged = hc->item.flagged;
  h->deleted = hc->item.deleted;

  if (hc->item.literal) {
    if (header_get(hc->item.literal, hc->item.literal_len, "From", value, sizeof(value))) {
      mail_decode_header(value, h->from, sizeof(h->from));
    }
    if (header_get(hc->item.literal, hc->item.literal_len, "Subject", value, sizeof(value))) {
      mail_decode_header(value, h->subject, sizeof(h->subject));
    }
    if (header_get(hc->item.literal, hc->item.literal_len, "Date", value, sizeof(value))) {
      mail_short_date(value, h->date, sizeof(h->date));
    }
  }
  if (h->subject[0] == 0) str_copy(h->subject, "(no subject)", sizeof(h->subject));
  if (h->from[0] == 0) str_copy(h->from, "(unknown sender)", sizeof(h->from));

  fetch_item_free(&hc->item);
  return 0;
}

/* run an operation, reconnecting once if a reused connection turned out to be dead */
#define RETRY_ONCE(s, expr) \
  do { \
    int had_conn = (s)->conn != NULL; \
    r = (expr); \
    if (r == MAIL_ERR_IO && had_conn && !(s)->fresh) { \
      debug(DEBUG_INFO, TAG, "reused connection failed, reconnecting"); \
      mail_session_disconnect(s); \
      if ((s)->ctx) (s)->ctx->error[0] = 0; \
      r = (expr); \
    } \
  } while (0)

static int fetch_headers_once(mail_session_t *s, const char *folder, int max,
                              mail_header_t **headers, int *nheaders, int *total) {
  header_collect_t hc;
  char status[MAIL_STATUS_LEN];
  int r, start, i, j;
  mail_header_t tmp;

  if ((r = session_ensure(s)) != MAIL_OK) return r;
  if ((r = session_select(s, folder, 1)) != MAIL_OK) return r;
  *total = s->exists;
  if (s->exists <= 0) return MAIL_OK;

  if (max < 1) max = 1;
  start = s->exists - max + 1;
  if (start < 1) start = 1;

  sys_snprintf(status, sizeof(status), "Fetching %d headers...", s->exists - start + 1);
  if ((r = progress(s->ctx, status)) != MAIL_OK) return r;

  sys_memset(&hc, 0, sizeof(hc));
  sys_snprintf(s->cmd, sizeof(s->cmd),
    "FETCH %d:%d (UID FLAGS RFC822.SIZE BODY.PEEK[HEADER.FIELDS (FROM SUBJECT DATE)])", start, s->exists);
  {
    char *cmd = xstrdup(s->cmd);
    if (cmd == NULL) return MAIL_ERR_MEM;
    r = imap_command(s, cmd, imap_untagged_header, &hc);
    xfree(cmd);
  }
  fetch_item_free(&hc.item);
  if (r != MAIL_OK) {
    if (hc.headers) xfree(hc.headers);
    return r;
  }

  /* newest first */
  for (i = 0; i < hc.n; i++) {
    for (j = i + 1; j < hc.n; j++) {
      if (hc.headers[j].uid > hc.headers[i].uid) {
        tmp = hc.headers[i];
        hc.headers[i] = hc.headers[j];
        hc.headers[j] = tmp;
      }
    }
  }

  *headers = hc.headers;
  *nheaders = hc.n;
  return MAIL_OK;
}

int mail_imap_fetch_headers(mail_session_t *s, const char *folder, int max,
                            mail_header_t **headers, int *nheaders, int *total) {
  int r;

  if (s == NULL || headers == NULL || nheaders == NULL || total == NULL) return MAIL_ERR_ARG;
  *headers = NULL;
  *nheaders = 0;
  *total = 0;
  if (s->ctx) s->ctx->error[0] = 0;

  RETRY_ONCE(s, fetch_headers_once(s, folder, max, headers, nheaders, total));
  if (r != MAIL_OK && r != MAIL_ERR_PROTO && r != MAIL_ERR_AUTH) mail_session_disconnect(s);

  return r;
}

/*
 * Folder list
 */

typedef struct {
  mail_folder_t *folders;
  int n;
} folder_collect_t;

static int imap_untagged_list(mail_session_t *s, char *line, void *data) {
  folder_collect_t *fc = (folder_collect_t *)data;
  mail_folder_t *f;
  char *p, *q, name[MAIL_FOLDER_LEN];
  int selectable = 1, n, r, i;

  if (!starts_with_ci(line, "* LIST ") && !starts_with_ci(line, "* LSUB ")) {
    return imap_untagged_exists(s, line, NULL);
  }
  if (fc->n >= MAIL_MAX_FOLDERS) return 0;

  p = line + 7;
  while (*p == ' ') p++;
  if (*p != '(') return 0;
  q = sys_strchr(p, ')');
  if (q == NULL) return 0;
  *q = 0;
  if (strstr_ci(p, "\\Noselect") || strstr_ci(p, "\\NonExistent")) selectable = 0;
  p = q + 1;
  while (*p == ' ') p++;

  /* hierarchy delimiter: NIL or "x" */
  if (*p == '"') {
    p++;
    if (*p == '\\') p++;
    if (*p) p++;
    if (*p == '"') p++;
  } else {
    while (*p && *p != ' ') p++;
  }
  while (*p == ' ') p++;

  /* mailbox name: quoted, literal or atom */
  name[0] = 0;
  if (*p == '"') {
    p++;
    for (i = 0; *p && *p != '"' && i < MAIL_FOLDER_LEN - 1; p++) {
      if (*p == '\\' && p[1]) p++;
      name[i++] = *p;
    }
    name[i] = 0;
  } else if (*p == '{') {
    n = sys_atoi(p + 1);
    if (n > 0) {
      char *lit = xmalloc(n + 1);
      if (lit == NULL) return MAIL_ERR_MEM;
      if ((r = conn_read_bytes(s->conn, lit, n)) != MAIL_OK) { xfree(lit); return r; }
      lit[n] = 0;
      str_copy(name, lit, sizeof(name));
      xfree(lit);
      /* rest of the line */
      if ((r = conn_readline(s->conn, s->line, sizeof(s->line))) < 0) return r;
    }
  } else {
    str_copy(name, p, sizeof(name));
    str_trim(name);
  }
  if (name[0] == 0) return 0;

  /* skip duplicates (LIST and LSUB overlap) */
  for (i = 0; i < fc->n; i++) {
    if (sys_strcmp(fc->folders[i].name, name) == 0) return 0;
  }

  f = &fc->folders[fc->n++];
  str_copy(f->name, name, sizeof(f->name));
  f->selectable = selectable;

  return 0;
}

static int list_folders_once(mail_session_t *s, mail_folder_t **folders, int *nfolders) {
  folder_collect_t fc;
  mail_folder_t tmp;
  int r, i;

  if ((r = session_ensure(s)) != MAIL_OK) return r;
  if ((r = progress(s->ctx, "Listing folders...")) != MAIL_OK) return r;

  sys_memset(&fc, 0, sizeof(fc));
  if ((fc.folders = xcalloc(MAIL_MAX_FOLDERS, sizeof(mail_folder_t))) == NULL) return MAIL_ERR_MEM;

  if ((r = imap_command(s, "LIST \"\" \"*\"", imap_untagged_list, &fc)) != MAIL_OK) {
    xfree(fc.folders);
    return r;
  }

  /* INBOX first */
  for (i = 0; i < fc.n; i++) {
    if (sys_strcasecmp(fc.folders[i].name, "INBOX") == 0) {
      if (i > 0) {
        tmp = fc.folders[i];
        sys_memmove(&fc.folders[1], &fc.folders[0], i * sizeof(mail_folder_t));
        fc.folders[0] = tmp;
      }
      break;
    }
  }
  if (fc.n == 0) {
    str_copy(fc.folders[0].name, "INBOX", MAIL_FOLDER_LEN);
    fc.folders[0].selectable = 1;
    fc.n = 1;
  }

  *folders = fc.folders;
  *nfolders = fc.n;
  return MAIL_OK;
}

int mail_imap_list_folders(mail_session_t *s, mail_folder_t **folders, int *nfolders) {
  int r;

  if (s == NULL || folders == NULL || nfolders == NULL) return MAIL_ERR_ARG;
  *folders = NULL;
  *nfolders = 0;
  if (s->ctx) s->ctx->error[0] = 0;

  RETRY_ONCE(s, list_folders_once(s, folders, nfolders));
  if (r != MAIL_OK && r != MAIL_ERR_PROTO && r != MAIL_ERR_AUTH) mail_session_disconnect(s);

  return r;
}

/*
 * MIME body extraction
 */

typedef struct {
  char type[64];
  char boundary[80];
  char charset[32];
  char encoding[24];
  char filename[64];
  int attachment;
} part_info_t;

static void parse_part_headers(const char *hdr, int hlen, part_info_t *pi) {
  char value[512], *p;

  sys_memset(pi, 0, sizeof(part_info_t));
  str_copy(pi->type, "text/plain", sizeof(pi->type));

  if (header_get(hdr, hlen, "Content-Type", value, sizeof(value))) {
    p = sys_strchr(value, ';');
    if (p) *p = 0;
    str_trim(value);
    str_lower(value);
    if (value[0]) str_copy(pi->type, value, sizeof(pi->type));
    if (p) {
      *p = ';';
      header_param(value, "boundary", pi->boundary, sizeof(pi->boundary));
      header_param(value, "charset", pi->charset, sizeof(pi->charset));
      header_param(value, "name", pi->filename, sizeof(pi->filename));
    }
  }
  if (header_get(hdr, hlen, "Content-Transfer-Encoding", value, sizeof(value))) {
    str_lower(value);
    str_copy(pi->encoding, value, sizeof(pi->encoding));
  }
  if (header_get(hdr, hlen, "Content-Disposition", value, sizeof(value))) {
    if (starts_with_ci(value, "attachment")) pi->attachment = 1;
    if (header_param(value, "filename", pi->filename, sizeof(pi->filename))) {
      pi->attachment = 1;
    }
  }
  if (pi->filename[0]) {
    char decoded[64];
    mail_decode_header(pi->filename, decoded, sizeof(decoded));
    str_copy(pi->filename, decoded, sizeof(pi->filename));
  }
}

/* decode the transfer encoding of a body part into a new NUL terminated buffer */
static char *decode_body(const char *src, int len, const char *encoding, int *outlen) {
  char *out;

  if ((out = xmalloc(len + 1)) == NULL) return NULL;
  sys_memcpy(out, src, len);
  out[len] = 0;

  if (sys_strcmp(encoding, "base64") == 0) {
    len = base64_decode(out, len);
  } else if (sys_strcmp(encoding, "quoted-printable") == 0) {
    len = qp_decode(out, len, 0);
  }
  out[len] = 0;
  *outlen = len;

  return out;
}

typedef struct {
  char *text;
  int len, cap;
  char *html;
  int html_len;
  char attach[MAX_ATTACH_TEXT];
  int nattach;
} extract_t;

static int extract_append(extract_t *ex, const char *s, int n) {
  char *tmp;

  if (ex->len + n + 1 > ex->cap) {
    int cap = ex->cap ? ex->cap : 1024;
    while (cap < ex->len + n + 1) cap *= 2;
    if ((tmp = xrealloc(ex->text, cap)) == NULL) return MAIL_ERR_MEM;
    ex->text = tmp;
    ex->cap = cap;
  }
  sys_memcpy(ex->text + ex->len, s, n);
  ex->len += n;
  ex->text[ex->len] = 0;

  return MAIL_OK;
}

/* normalize line ends and control characters in place */
static int clean_text(char *s, int len) {
  char *in = s, *out = s, *end = s + len;

  while (in < end) {
    if (*in == '\r') { in++; continue; }
    if (*in == '\t') { *out++ = ' '; in++; continue; }
    if ((unsigned char)*in < 0x20 && *in != '\n') { in++; continue; }
    *out++ = *in++;
  }
  *out = 0;

  return (int)(out - s);
}

static int entity_value(const char *name, int n) {
  static const struct { const char *name; int c; } ents[] = {
    { "amp", '&' }, { "lt", '<' }, { "gt", '>' }, { "quot", '"' }, { "apos", '\'' },
    { "nbsp", ' ' }, { "copy", 0xA9 }, { "reg", 0xAE }, { "deg", 0xB0 }, { "eacute", 0xE9 },
    { "egrave", 0xE8 }, { "agrave", 0xE0 }, { "ccedil", 0xE7 }, { "uuml", 0xFC }, { "ouml", 0xF6 },
    { "auml", 0xE4 }, { "szlig", 0xDF }, { "ntilde", 0xF1 }, { "iexcl", 0xA1 }, { "iquest", 0xBF },
    { "hellip", '.' }, { "mdash", '-' }, { "ndash", '-' }, { "lsquo", '\'' }, { "rsquo", '\'' },
    { "ldquo", '"' }, { "rdquo", '"' }, { "bull", '*' }, { "middot", 0xB7 }, { "euro", 'E' },
    { "times", 'x' }, { "laquo", 0xAB }, { "raquo", 0xBB }, { NULL, 0 }
  };
  int i, v = 0;

  if (n <= 0) return -1;
  if (name[0] == '#') {
    if (n > 1 && (name[1] == 'x' || name[1] == 'X')) {
      for (i = 2; i < n; i++) {
        int h = hex_value(name[i]);
        if (h < 0) return -1;
        v = v * 16 + h;
      }
    } else {
      for (i = 1; i < n; i++) {
        if (!sys_isdigit((unsigned char)name[i])) return -1;
        v = v * 10 + (name[i] - '0');
      }
    }
    if (v == 0xA0) return ' ';
    if (v == 0x2019 || v == 0x2018) return '\'';
    if (v == 0x201C || v == 0x201D) return '"';
    if (v == 0x2013 || v == 0x2014) return '-';
    return v < 0x100 ? v : '?';
  }
  for (i = 0; ents[i].name; i++) {
    if ((int)sys_strlen(ents[i].name) == n && sys_strncasecmp(ents[i].name, name, n) == 0) return ents[i].c;
  }
  return -1;
}

/* very small HTML to text conversion (in place; output is never longer) */
static int html_to_text(char *s, int len) {
  char *in = s, *out = s, *end = s + len, *q;
  char tag[16];
  int n, i, v, newlines = 2, space = 0, closing;

  while (in < end) {
    if (*in == '<') {
      if (in + 3 < end && in[1] == '!' && in[2] == '-' && in[3] == '-') {
        q = sys_strstr(in + 4, "-->");
        in = q ? q + 3 : end;
        continue;
      }
      /* tag name */
      q = in + 1;
      closing = 0;
      if (q < end && *q == '/') { q++; closing = 1; }
      for (i = 0; q < end && i < 15 && (sys_isalpha((unsigned char)*q) || sys_isdigit((unsigned char)*q)); q++) tag[i++] = sys_tolower(*q);
      tag[i] = 0;
      /* skip to the end of the tag */
      while (q < end && *q != '>') {
        if (*q == '"' || *q == '\'') {
          char quote = *q++;
          while (q < end && *q != quote) q++;
        }
        if (q < end) q++;
      }
      if (q < end) q++;

      if (!closing && (sys_strcmp(tag, "script") == 0 || sys_strcmp(tag, "style") == 0 || sys_strcmp(tag, "head") == 0)) {
        /* skip the element content */
        char close[24];
        sys_snprintf(close, sizeof(close), "</%s", tag);
        for (;;) {
          char *e = strstr_ci_len(q, end, close);
          if (e == NULL) { q = end; break; }
          q = e;
          while (q < end && *q != '>') q++;
          if (q < end) q++;
          break;
        }
      }
      in = q;

      n = 0;
      if (sys_strcmp(tag, "br") == 0 || sys_strcmp(tag, "li") == 0 || sys_strcmp(tag, "tr") == 0 || sys_strcmp(tag, "dd") == 0 || sys_strcmp(tag, "dt") == 0) n = 1;
      else if (sys_strcmp(tag, "p") == 0 || sys_strcmp(tag, "div") == 0 || sys_strcmp(tag, "table") == 0 || sys_strcmp(tag, "blockquote") == 0 ||
               sys_strcmp(tag, "ul") == 0 || sys_strcmp(tag, "ol") == 0 || sys_strcmp(tag, "pre") == 0 || sys_strcmp(tag, "hr") == 0 ||
               (tag[0] == 'h' && sys_isdigit((unsigned char)tag[1]) && tag[2] == 0)) n = 2;
      else if (sys_strcmp(tag, "td") == 0 || sys_strcmp(tag, "th") == 0) space = 1;

      /* make sure at least n line breaks precede the next text */
      while (newlines < n) { *out++ = '\n'; newlines++; }
      if (newlines) space = 0;
      if (!closing && sys_strcmp(tag, "li") == 0) {
        *out++ = '-'; *out++ = ' '; newlines = 0;
      }
      continue;
    }
    if (*in == '&') {
      q = in + 1;
      for (n = 0; q < end && n < 10 && *q != ';' && *q != ' ' && *q != '<' && *q != '&'; q++, n++);
      if (q < end && *q == ';' && (v = entity_value(in + 1, n)) >= 0) {
        in = q + 1;
        if (v == ' ') { space = 1; continue; }
        if (space && newlines == 0) *out++ = ' ';
        space = 0;
        *out++ = (char)v;
        newlines = 0;
        continue;
      }
      /* not an entity */
    }
    if (*in == ' ' || *in == '\t' || *in == '\n' || *in == '\r') {
      space = 1;
      in++;
      continue;
    }
    if (space && newlines == 0) *out++ = ' ';
    space = 0;
    *out++ = *in++;
    newlines = 0;
  }
  *out = 0;

  return (int)(out - s);
}

static void extract_part(const char *data, int len, extract_t *ex, int depth);

static void extract_multipart(const char *body, int len, const char *boundary, extract_t *ex, int depth) {
  const char *p = body, *end = body + len, *start, *e;
  char delim[96];
  int dlen, closing;

  sys_snprintf(delim, sizeof(delim), "--%s", boundary);
  dlen = sys_strlen(delim);

  /* find the first delimiter */
  start = NULL;
  while (p < end) {
    if ((p == body || p[-1] == '\n') && end - p >= dlen && sys_memcmp(p, delim, dlen) == 0) {
      closing = (end - p >= dlen + 2 && p[dlen] == '-' && p[dlen + 1] == '-');
      if (start) {
        e = p;
        /* the CRLF before the delimiter belongs to it */
        if (e > start && e[-1] == '\n') e--;
        if (e > start && e[-1] == '\r') e--;
        extract_part(start, (int)(e - start), ex, depth + 1);
      }
      if (closing) break;
      /* skip the delimiter line */
      while (p < end && *p != '\n') p++;
      if (p < end) p++;
      start = p;
      continue;
    }
    while (p < end && *p != '\n') p++;
    if (p < end) p++;
  }
}

static void extract_attachment(extract_t *ex, part_info_t *pi, int size) {
  char line[128];
  int n;

  ex->nattach++;
  sys_snprintf(line, sizeof(line), "\n[Attachment: %s (%s, %d KB)]",
    pi->filename[0] ? pi->filename : "unnamed", pi->type, (size + 1023) / 1024);
  n = sys_strlen(ex->attach);
  if (n + (int)sys_strlen(line) < MAX_ATTACH_TEXT - 1) {
    sys_strcat(ex->attach, line);
  }
}

static void extract_part(const char *data, int len, extract_t *ex, int depth) {
  part_info_t pi;
  char *decoded;
  int hlen, boff, dlen;

  if (depth > MAX_DEPTH || len <= 0) return;

  boff = split_message(data, len, &hlen);
  parse_part_headers(data, hlen, &pi);

  if (sys_strncmp(pi.type, "multipart/", 10) == 0) {
    if (pi.boundary[0]) {
      extract_multipart(data + boff, len - boff, pi.boundary, ex, depth);
    } else {
      /* broken multipart: show it as text */
      extract_append(ex, data + boff, len - boff);
    }
    return;
  }

  if (sys_strcmp(pi.type, "message/rfc822") == 0 && !pi.attachment) {
    extract_part(data + boff, len - boff, ex, depth + 1);
    return;
  }

  if (sys_strcmp(pi.type, "text/plain") == 0 && !pi.attachment) {
    if ((decoded = decode_body(data + boff, len - boff, pi.encoding, &dlen)) != NULL) {
      dlen = convert_charset(decoded, pi.charset);
      dlen = clean_text(decoded, dlen);
      if (ex->len > 0) extract_append(ex, "\n\n", 2);
      extract_append(ex, decoded, dlen);
      xfree(decoded);
    }
    return;
  }

  if (sys_strcmp(pi.type, "text/html") == 0 && !pi.attachment) {
    if (ex->html == NULL) {
      if ((decoded = decode_body(data + boff, len - boff, pi.encoding, &dlen)) != NULL) {
        dlen = convert_charset(decoded, pi.charset);
        ex->html = decoded;
        ex->html_len = dlen;
      }
    }
    return;
  }

  extract_attachment(ex, &pi, len - boff);
}

void mail_message_free(mail_message_t *msg) {
  if (msg && msg->text) {
    xfree(msg->text);
    msg->text = NULL;
    msg->text_len = 0;
  }
}

/* parse a raw message into msg */
static int parse_message(const char *raw, int len, int truncated, mail_message_t *msg) {
  extract_t ex;
  char value[512];
  int hlen, n;

  sys_memset(msg, 0, sizeof(mail_message_t));
  msg->truncated = truncated;

  split_message(raw, len, &hlen);
  if (header_get(raw, hlen, "From", value, sizeof(value))) mail_decode_header(value, msg->from, sizeof(msg->from));
  if (header_get(raw, hlen, "Reply-To", value, sizeof(value))) mail_decode_header(value, msg->reply_to, sizeof(msg->reply_to));
  if (header_get(raw, hlen, "To", value, sizeof(value))) mail_decode_header(value, msg->to, sizeof(msg->to));
  if (header_get(raw, hlen, "Cc", value, sizeof(value))) mail_decode_header(value, msg->cc, sizeof(msg->cc));
  if (header_get(raw, hlen, "Subject", value, sizeof(value))) mail_decode_header(value, msg->subject, sizeof(msg->subject));
  if (header_get(raw, hlen, "Date", value, sizeof(value))) mail_short_date(value, msg->date, sizeof(msg->date));
  if (header_get(raw, hlen, "Message-ID", value, sizeof(value))) str_copy(msg->message_id, value, sizeof(msg->message_id));

  sys_memset(&ex, 0, sizeof(ex));
  extract_part(raw, len, &ex, 0);

  if (ex.len == 0 && ex.html) {
    n = html_to_text(ex.html, ex.html_len);
    n = clean_text(ex.html, n);
    extract_append(&ex, ex.html, n);
  }
  if (ex.html) xfree(ex.html);

  while (ex.len > 0 && (ex.text[ex.len - 1] == '\n' || ex.text[ex.len - 1] == ' ')) ex.text[--ex.len] = 0;

  if (ex.nattach) {
    extract_append(&ex, "\n", 1);
    extract_append(&ex, ex.attach, sys_strlen(ex.attach));
  }
  if (truncated) {
    extract_append(&ex, "\n\n[Message truncated]", 21);
  }
  if (ex.text == NULL) {
    extract_append(&ex, "", 0);
  }

  msg->text = ex.text;
  msg->text_len = ex.len;
  msg->nattach = ex.nattach;

  return MAIL_OK;
}

/*
 * Message download
 */

static int fetch_message_once(mail_session_t *s, const char *folder, uint32_t uid, int limit, mail_message_t *msg) {
  fetch_item_t it;
  int r;

  if ((r = session_ensure(s)) != MAIL_OK) return r;
  if ((r = session_select(s, folder, 0)) != MAIL_OK) return r;
  if ((r = progress(s->ctx, "Downloading message...")) != MAIL_OK) return r;

  sys_memset(&it, 0, sizeof(it));
  it.literal_cap = limit > 0 ? limit : 65536;
  sys_snprintf(s->cmd, sizeof(s->cmd), "UID FETCH %u (BODY.PEEK[]<0.%d>)", (unsigned int)uid, it.literal_cap);
  {
    char *cmd = xstrdup(s->cmd);
    if (cmd == NULL) return MAIL_ERR_MEM;
    r = imap_command(s, cmd, imap_untagged_fetch, &it);
    xfree(cmd);
  }
  if (r != MAIL_OK) {
    fetch_item_free(&it);
    return r;
  }
  if (!it.have || it.literal == NULL) {
    fetch_item_free(&it);
    set_error(s->ctx, "Message not found on server", NULL);
    return MAIL_ERR_PROTO;
  }

  /* the partial fetch tells us nothing about the total size: RFC822.SIZE does */
  parse_message(it.literal, it.literal_len, it.literal_len >= it.literal_cap, msg);
  fetch_item_free(&it);

  /* mark as read; failure is not fatal */
  sys_snprintf(s->cmd, sizeof(s->cmd), "UID STORE %u +FLAGS.SILENT (\\Seen)", (unsigned int)uid);
  {
    char *cmd = xstrdup(s->cmd);
    if (cmd) {
      imap_command(s, cmd, imap_untagged_exists, NULL);
      xfree(cmd);
    }
  }

  return MAIL_OK;
}

int mail_imap_fetch_message(mail_session_t *s, const char *folder, uint32_t uid, int limit, mail_message_t *msg) {
  int r;

  if (s == NULL || msg == NULL) return MAIL_ERR_ARG;
  sys_memset(msg, 0, sizeof(mail_message_t));
  if (s->ctx) s->ctx->error[0] = 0;

  RETRY_ONCE(s, fetch_message_once(s, folder, uid, limit, msg));
  if (r != MAIL_OK && r != MAIL_ERR_PROTO && r != MAIL_ERR_AUTH) mail_session_disconnect(s);

  return r;
}

/*
 * Delete
 */

static int delete_message_once(mail_session_t *s, const char *folder, uint32_t uid) {
  int r;

  if ((r = session_ensure(s)) != MAIL_OK) return r;
  if ((r = session_select(s, folder, 0)) != MAIL_OK) return r;
  if ((r = progress(s->ctx, "Deleting message...")) != MAIL_OK) return r;

  sys_snprintf(s->cmd, sizeof(s->cmd), "UID STORE %u +FLAGS.SILENT (\\Deleted)", (unsigned int)uid);
  {
    char *cmd = xstrdup(s->cmd);
    if (cmd == NULL) return MAIL_ERR_MEM;
    r = imap_command(s, cmd, imap_untagged_exists, NULL);
    xfree(cmd);
  }
  if (r != MAIL_OK) return r;

  /* UID EXPUNGE (UIDPLUS) only removes this message; fall back to EXPUNGE */
  sys_snprintf(s->cmd, sizeof(s->cmd), "UID EXPUNGE %u", (unsigned int)uid);
  {
    char *cmd = xstrdup(s->cmd);
    if (cmd == NULL) return MAIL_ERR_MEM;
    r = imap_command(s, cmd, imap_untagged_exists, NULL);
    xfree(cmd);
  }
  if (r == MAIL_ERR_PROTO) {
    if (s->ctx) s->ctx->error[0] = 0;
    r = imap_command(s, "EXPUNGE", imap_untagged_exists, NULL);
  }

  return r;
}

int mail_imap_delete_message(mail_session_t *s, const char *folder, uint32_t uid) {
  int r;

  if (s == NULL) return MAIL_ERR_ARG;
  if (s->ctx) s->ctx->error[0] = 0;

  RETRY_ONCE(s, delete_message_once(s, folder, uid));
  if (r != MAIL_OK && r != MAIL_ERR_PROTO && r != MAIL_ERR_AUTH) mail_session_disconnect(s);

  return r;
}


/*
 * POP3 (RFC 1939)
 */

/* read a status line; MAIL_OK for +OK, MAIL_ERR_PROTO for -ERR (text in s->result) */
static int pop3_reply(mail_session_t *s) {
  int r;

  if ((r = conn_readline(s->conn, s->line, sizeof(s->line))) < 0) return r;
  debug(DEBUG_INFO, TAG, "S: %.120s", s->line);
  if (starts_with_ci(s->line, "+OK")) {
    str_copy(s->result, skip_spaces(s->line + 3), sizeof(s->result));
    return MAIL_OK;
  }
  if (starts_with_ci(s->line, "-ERR")) {
    str_copy(s->result, skip_spaces(s->line + 4), sizeof(s->result));
    set_error(s->ctx, "Server said", s->result);
    return MAIL_ERR_PROTO;
  }
  set_error(s->ctx, "Malformed POP3 reply", s->line);
  return MAIL_ERR_PROTO;
}

static int pop3_command(mail_session_t *s, const char *cmd, int secret) {
  int r, n;

  n = sys_snprintf(s->cmd, sizeof(s->cmd), "%s\r\n", cmd);
  if (n >= (int)sizeof(s->cmd)) {
    set_error(s->ctx, "Command too long", NULL);
    return MAIL_ERR_ARG;
  }
  debug(DEBUG_INFO, TAG, "C: %s", secret ? "PASS ****" : cmd);
  if ((r = conn_write(s->conn, s->cmd, n)) != MAIL_OK) return r;

  return pop3_reply(s);
}

typedef int (*pop3_line_f)(mail_session_t *s, const char *line, void *data);

typedef struct {
  char *buf;
  int len, size, cap;
  int truncated;
} pop3_body_t;

static int pop3_body_append(pop3_body_t *b, const char *data, int n) {
  char *tmp;
  int keep = n;

  if (b->cap > 0 && b->len + keep > b->cap) {
    keep = b->cap - b->len;
    b->truncated = 1;
  }
  if (keep <= 0) return MAIL_OK;
  if (b->len + keep + 1 > b->size) {
    int size = b->size ? b->size : 4096;
    while (size < b->len + keep + 1) size *= 2;
    if (b->cap > 0 && size > b->cap + 1) size = b->cap + 1;
    if ((tmp = xrealloc(b->buf, size)) == NULL) return MAIL_ERR_MEM;
    b->buf = tmp;
    b->size = size;
  }
  sys_memcpy(b->buf + b->len, data, keep);
  b->len += keep;
  b->buf[b->len] = 0;

  return MAIL_OK;
}

/* read a multi-line response (after its +OK line) up to the ".\r\n" terminator,
   removing byte stuffing. With `body` the data is collected (at most body->cap
   bytes, the rest is read and dropped); with `cb` every complete line is
   passed to the callback instead (over-long lines arrive in pieces). */
static int pop3_read_multiline(mail_session_t *s, pop3_body_t *body, pop3_line_f cb, void *data) {
  conn_t *c = s->conn;
  char *line = NULL;
  int at_start = 1, i, n, r, avail;

  for (;;) {
    avail = c->len - c->pos;
    if (avail <= 0) {
      if ((r = conn_fill(c)) <= 0) {
        if (r == 0) set_error(s->ctx, "Connection closed by", c->host);
        return r == 0 ? MAIL_ERR_IO : r;
      }
      continue;
    }

    for (i = c->pos; i < c->len && c->buf[i] != '\n'; i++);

    if (i >= c->len) {
      /* no complete line in the buffer */
      if (avail < CONN_BUFLEN) {
        if ((r = conn_fill(c)) <= 0) {
          if (r == 0) set_error(s->ctx, "Connection closed by", c->host);
          return r == 0 ? MAIL_ERR_IO : r;
        }
        continue;
      }
      /* buffer full with a very long line: flush it as data (it cannot be the terminator) */
      n = avail;
      if (at_start && c->buf[c->pos] == '.') { c->pos++; n--; }
      if (body) {
        if ((r = pop3_body_append(body, c->buf + c->pos, n)) != MAIL_OK) return r;
      } else if (cb) {
        if ((line = xmalloc(n + 1)) == NULL) return MAIL_ERR_MEM;
        sys_memcpy(line, c->buf + c->pos, n);
        line[n] = 0;
        r = cb(s, line, data);
        xfree(line);
        if (r < 0) return r;
      }
      c->pos = c->len;
      at_start = 0;
      continue;
    }

    /* complete line c->buf[c->pos .. i] */
    n = i - c->pos + 1;
    if (at_start && c->buf[c->pos] == '.') {
      if (n == 2 || (n == 3 && c->buf[c->pos + 1] == '\r')) {
        c->pos = i + 1;   /* terminator */
        return MAIL_OK;
      }
      c->pos++;           /* byte stuffing */
      n--;
    }
    if (body) {
      if ((r = pop3_body_append(body, c->buf + c->pos, n)) != MAIL_OK) return r;
    } else if (cb) {
      int m = n;
      while (m > 0 && (c->buf[c->pos + m - 1] == '\n' || c->buf[c->pos + m - 1] == '\r')) m--;
      if ((line = xmalloc(m + 1)) == NULL) return MAIL_ERR_MEM;
      sys_memcpy(line, c->buf + c->pos, m);
      line[m] = 0;
      r = cb(s, line, data);
      xfree(line);
      if (r < 0) return r;
    }
    c->pos = i + 1;
    at_start = 1;
  }
}

static int pop3_connect(mail_session_t *s) {
  mail_account_t *a = &s->account;
  int r;

  if ((s->conn = conn_new(s->ctx)) == NULL) return MAIL_ERR_MEM;

  if ((r = conn_open(s->conn, a->imap_host, a->imap_port, a->imap_sec)) != MAIL_OK) goto fail;

  if ((r = pop3_reply(s)) != MAIL_OK) {
    if (r == MAIL_ERR_PROTO) set_error(s->ctx, "Unexpected POP3 greeting", s->line);
    goto fail;
  }

  if (a->imap_sec == MAIL_SEC_STARTTLS) {
    if ((r = pop3_command(s, "STLS", 0)) != MAIL_OK) {
      if (r == MAIL_ERR_PROTO) set_error(s->ctx, "Server refused STLS", s->result);
      goto fail;
    }
    if ((r = conn_tls(s->conn)) != MAIL_OK) goto fail;
  }

  if ((r = progress(s->ctx, "Logging in...")) != MAIL_OK) goto fail;
  sys_snprintf(s->line, sizeof(s->line), "USER %s", a->user);
  {
    char *cmd = xstrdup(s->line);
    if (cmd == NULL) { r = MAIL_ERR_MEM; goto fail; }
    r = pop3_command(s, cmd, 0);
    xfree(cmd);
  }
  if (r == MAIL_OK) {
    sys_snprintf(s->line, sizeof(s->line), "PASS %s", a->pass);
    char *cmd = xstrdup(s->line);
    if (cmd == NULL) { r = MAIL_ERR_MEM; goto fail; }
    r = pop3_command(s, cmd, 1);
    xfree(cmd);
  }
  if (r != MAIL_OK) {
    if (r == MAIL_ERR_PROTO) {
      set_error(s->ctx, "Login rejected", s->result);
      r = MAIL_ERR_AUTH;
    }
    goto fail;
  }

  /* mail drop size */
  if ((r = pop3_command(s, "STAT", 0)) != MAIL_OK) goto fail;
  s->pop3_count = sys_atoi(s->result);
  s->exists = s->pop3_count;
  s->fresh = 1;
  return MAIL_OK;

fail:
  conn_close(s->conn);
  s->conn = NULL;
  return r;
}

/* live, logged-in POP3 connection; `renew` forces a new session (fresh mail drop snapshot) */
static int pop3_ensure(mail_session_t *s, int renew) {
  int r;

  s->fresh = 0;
  if (s->conn) {
    if (renew) {
      /* QUIT commits pending deletions and ends this snapshot */
      pop3_command(s, "QUIT", 0);
      mail_session_disconnect(s);
      if (s->ctx) s->ctx->error[0] = 0;
    } else {
      r = pop3_command(s, "NOOP", 0);
      if (r == MAIL_OK) return MAIL_OK;
      if (r == MAIL_ERR_CANCEL) return r;
      debug(DEBUG_INFO, TAG, "POP3 connection is stale, reconnecting");
      mail_session_disconnect(s);
      if (s->ctx) s->ctx->error[0] = 0;
    }
  }

  return pop3_connect(s);
}

typedef struct {
  mail_header_t *headers;
  int n;
  int first;              /* lowest message number in headers */
  int sizes;              /* 1: LIST response, 0: UIDL response */
} pop3_list_t;

/* "num uidl" / "num size" lines of UIDL and LIST */
static int pop3_list_line(mail_session_t *s, const char *line, void *data) {
  pop3_list_t *pl = (pop3_list_t *)data;
  const char *p;
  int num, idx;

  num = sys_atoi(line);
  idx = num - pl->first;
  if (num <= 0 || idx < 0 || idx >= pl->n) return 0;
  p = line;
  while (sys_isdigit((unsigned char)*p)) p++;
  p = skip_spaces(p);
  if (pl->sizes) {
    pl->headers[idx].size = (uint32_t)sys_strtoul(p, NULL, 10);
  } else {
    str_copy(pl->headers[idx].uidl, p, MAIL_UIDL_LEN);
    str_trim(pl->headers[idx].uidl);
  }
  (void)s;
  return 0;
}

static int pop3_fetch_headers_once(mail_session_t *s, int max, mail_header_t **headers, int *nheaders, int *total) {
  pop3_list_t pl;
  pop3_body_t body;
  mail_header_t *h, tmp;
  char status[MAIL_STATUS_LEN], value[256];
  int r, first, n, i, j;

  if ((r = pop3_ensure(s, 1)) != MAIL_OK) return r;
  *total = s->pop3_count;
  if (s->pop3_count <= 0) return MAIL_OK;

  if (max < 1) max = 1;
  first = s->pop3_count - max + 1;
  if (first < 1) first = 1;
  n = s->pop3_count - first + 1;

  sys_memset(&pl, 0, sizeof(pl));
  if ((pl.headers = xcalloc(n, sizeof(mail_header_t))) == NULL) return MAIL_ERR_MEM;
  pl.n = n;
  pl.first = first;
  for (i = 0; i < n; i++) pl.headers[i].uid = first + i;

  /* unique ids and sizes */
  if ((r = pop3_command(s, "UIDL", 0)) != MAIL_OK) goto fail;
  if ((r = pop3_read_multiline(s, NULL, pop3_list_line, &pl)) != MAIL_OK) goto fail;
  pl.sizes = 1;
  if ((r = pop3_command(s, "LIST", 0)) == MAIL_OK) {
    if ((r = pop3_read_multiline(s, NULL, pop3_list_line, &pl)) != MAIL_OK) goto fail;
  } else if (r != MAIL_ERR_PROTO) {
    goto fail;
  }

  /* remember number <-> UIDL for later RETR / DELE */
  pop3_cache_free(s);
  s->pop3_count = *total;
  if ((s->pop3_cache = xcalloc(n, sizeof(s->pop3_cache[0]))) != NULL) {
    for (i = 0; i < n; i++) {
      s->pop3_cache[i].num = pl.headers[i].uid;
      str_copy(s->pop3_cache[i].uidl, pl.headers[i].uidl, MAIL_UIDL_LEN);
    }
    s->pop3_ncache = n;
  }

  /* headers, newest first */
  for (i = n - 1; i >= 0; i--) {
    h = &pl.headers[i];
    sys_snprintf(status, sizeof(status), "Fetching header %d of %d...", n - i, n);
    if ((r = progress(s->ctx, status)) != MAIL_OK) goto fail;
    sys_snprintf(s->line, sizeof(s->line), "TOP %u 0", (unsigned int)h->uid);
    {
      char *cmd = xstrdup(s->line);
      if (cmd == NULL) { r = MAIL_ERR_MEM; goto fail; }
      r = pop3_command(s, cmd, 0);
      xfree(cmd);
    }
    if (r == MAIL_ERR_PROTO) {
      /* TOP not supported: leave the header empty rather than failing */
      if (s->ctx) s->ctx->error[0] = 0;
      str_copy(h->subject, "(headers not available)", sizeof(h->subject));
      continue;
    }
    if (r != MAIL_OK) goto fail;
    sys_memset(&body, 0, sizeof(body));
    body.cap = 16384;
    r = pop3_read_multiline(s, &body, NULL, NULL);
    if (r == MAIL_OK && body.buf) {
      if (header_get(body.buf, body.len, "From", value, sizeof(value))) mail_decode_header(value, h->from, sizeof(h->from));
      if (header_get(body.buf, body.len, "Subject", value, sizeof(value))) mail_decode_header(value, h->subject, sizeof(h->subject));
      if (header_get(body.buf, body.len, "Date", value, sizeof(value))) mail_short_date(value, h->date, sizeof(h->date));
    }
    if (body.buf) xfree(body.buf);
    if (r != MAIL_OK) goto fail;
    if (h->subject[0] == 0) str_copy(h->subject, "(no subject)", sizeof(h->subject));
    if (h->from[0] == 0) str_copy(h->from, "(unknown sender)", sizeof(h->from));
  }

  /* newest (highest number) first */
  for (i = 0; i < n / 2; i++) {
    j = n - 1 - i;
    tmp = pl.headers[i];
    pl.headers[i] = pl.headers[j];
    pl.headers[j] = tmp;
  }

  *headers = pl.headers;
  *nheaders = n;
  return MAIL_OK;

fail:
  xfree(pl.headers);
  return r;
}

int mail_pop3_fetch_headers(mail_session_t *s, int max, mail_header_t **headers, int *nheaders, int *total) {
  int r;

  if (s == NULL || headers == NULL || nheaders == NULL || total == NULL) return MAIL_ERR_ARG;
  *headers = NULL;
  *nheaders = 0;
  *total = 0;
  if (s->ctx) s->ctx->error[0] = 0;

  r = pop3_fetch_headers_once(s, max, headers, nheaders, total);
  if (r != MAIL_OK && r != MAIL_ERR_PROTO && r != MAIL_ERR_AUTH) mail_session_disconnect(s);

  return r;
}

typedef struct {
  const char *uidl;
  uint32_t num;
} pop3_find_t;

static int pop3_find_line(mail_session_t *s, const char *line, void *data) {
  pop3_find_t *f = (pop3_find_t *)data;
  const char *p = line;
  char uidl[MAIL_UIDL_LEN];

  while (sys_isdigit((unsigned char)*p)) p++;
  str_copy(uidl, skip_spaces(p), sizeof(uidl));
  str_trim(uidl);
  if (sys_strcmp(uidl, f->uidl) == 0) f->num = (uint32_t)sys_atoi(line);
  (void)s;
  return 0;
}

/* message number of a UIDL in the current session (0 = gone) */
static int pop3_resolve(mail_session_t *s, const char *uidl, uint32_t *num) {
  pop3_find_t f;
  int i, r;

  *num = 0;
  if (uidl == NULL || uidl[0] == 0) {
    set_error(s->ctx, "Message has no unique id", NULL);
    return MAIL_ERR_ARG;
  }
  for (i = 0; i < s->pop3_ncache; i++) {
    if (sys_strcmp(s->pop3_cache[i].uidl, uidl) == 0) {
      *num = s->pop3_cache[i].num;
      return MAIL_OK;
    }
  }

  f.uidl = uidl;
  f.num = 0;
  if ((r = pop3_command(s, "UIDL", 0)) != MAIL_OK) return r;
  if ((r = pop3_read_multiline(s, NULL, pop3_find_line, &f)) != MAIL_OK) return r;
  if (f.num == 0) {
    set_error(s->ctx, "Message is no longer on the server", NULL);
    return MAIL_ERR_PROTO;
  }
  *num = f.num;
  return MAIL_OK;
}

static int pop3_fetch_message_once(mail_session_t *s, const char *uidl, int limit, mail_message_t *msg) {
  pop3_body_t body;
  uint32_t num;
  int r;

  if ((r = pop3_ensure(s, 0)) != MAIL_OK) return r;
  if ((r = pop3_resolve(s, uidl, &num)) != MAIL_OK) return r;
  if ((r = progress(s->ctx, "Downloading message...")) != MAIL_OK) return r;

  sys_snprintf(s->line, sizeof(s->line), "RETR %u", (unsigned int)num);
  {
    char *cmd = xstrdup(s->line);
    if (cmd == NULL) return MAIL_ERR_MEM;
    r = pop3_command(s, cmd, 0);
    xfree(cmd);
  }
  if (r != MAIL_OK) return r;

  sys_memset(&body, 0, sizeof(body));
  body.cap = limit > 0 ? limit : 65536;
  r = pop3_read_multiline(s, &body, NULL, NULL);
  if (r != MAIL_OK) {
    if (body.buf) xfree(body.buf);
    return r;
  }
  if (body.buf == NULL) {
    if ((body.buf = xcalloc(1, 1)) == NULL) return MAIL_ERR_MEM;
  }
  parse_message(body.buf, body.len, body.truncated, msg);
  xfree(body.buf);

  return MAIL_OK;
}

int mail_pop3_fetch_message(mail_session_t *s, const char *uidl, int limit, mail_message_t *msg) {
  int r;

  if (s == NULL || msg == NULL) return MAIL_ERR_ARG;
  sys_memset(msg, 0, sizeof(mail_message_t));
  if (s->ctx) s->ctx->error[0] = 0;

  RETRY_ONCE(s, pop3_fetch_message_once(s, uidl, limit, msg));
  if (r != MAIL_OK && r != MAIL_ERR_PROTO && r != MAIL_ERR_AUTH) mail_session_disconnect(s);

  return r;
}

static int pop3_delete_message_once(mail_session_t *s, const char *uidl) {
  uint32_t num;
  int r;

  if ((r = pop3_ensure(s, 0)) != MAIL_OK) return r;
  if ((r = pop3_resolve(s, uidl, &num)) != MAIL_OK) return r;
  if ((r = progress(s->ctx, "Deleting message...")) != MAIL_OK) return r;

  sys_snprintf(s->line, sizeof(s->line), "DELE %u", (unsigned int)num);
  {
    char *cmd = xstrdup(s->line);
    if (cmd == NULL) return MAIL_ERR_MEM;
    r = pop3_command(s, cmd, 0);
    xfree(cmd);
  }
  if (r != MAIL_OK) return r;

  /* only QUIT makes the deletion permanent */
  r = pop3_command(s, "QUIT", 0);
  mail_session_disconnect(s);
  if (r == MAIL_ERR_CANCEL) return r;
  return r == MAIL_OK ? MAIL_OK : MAIL_ERR_PROTO;
}

int mail_pop3_delete_message(mail_session_t *s, const char *uidl) {
  int r;

  if (s == NULL) return MAIL_ERR_ARG;
  if (s->ctx) s->ctx->error[0] = 0;

  RETRY_ONCE(s, pop3_delete_message_once(s, uidl));
  if (r != MAIL_OK) mail_session_disconnect(s);

  return r;
}

/*
 * Protocol independent entry points
 */

int mail_list_folders(mail_session_t *s, mail_folder_t **folders, int *nfolders) {
  if (s == NULL || folders == NULL || nfolders == NULL) return MAIL_ERR_ARG;
  if (s->account.proto == MAIL_PROTO_POP3) {
    if ((*folders = xcalloc(1, sizeof(mail_folder_t))) == NULL) return MAIL_ERR_MEM;
    str_copy((*folders)[0].name, "INBOX", MAIL_FOLDER_LEN);
    (*folders)[0].selectable = 1;
    *nfolders = 1;
    return MAIL_OK;
  }
  return mail_imap_list_folders(s, folders, nfolders);
}

int mail_fetch_headers(mail_session_t *s, const char *folder, int max, mail_header_t **headers, int *nheaders, int *total) {
  if (s == NULL) return MAIL_ERR_ARG;
  if (s->account.proto == MAIL_PROTO_POP3) return mail_pop3_fetch_headers(s, max, headers, nheaders, total);
  return mail_imap_fetch_headers(s, folder, max, headers, nheaders, total);
}

int mail_fetch_message(mail_session_t *s, const char *folder, const mail_header_t *hdr, int limit, mail_message_t *msg) {
  if (s == NULL || hdr == NULL) return MAIL_ERR_ARG;
  if (s->account.proto == MAIL_PROTO_POP3) return mail_pop3_fetch_message(s, hdr->uidl, limit, msg);
  return mail_imap_fetch_message(s, folder, hdr->uid, limit, msg);
}

int mail_delete_message(mail_session_t *s, const char *folder, const mail_header_t *hdr) {
  if (s == NULL || hdr == NULL) return MAIL_ERR_ARG;
  if (s->account.proto == MAIL_PROTO_POP3) return mail_pop3_delete_message(s, hdr->uidl);
  return mail_imap_delete_message(s, folder, hdr->uid);
}

/*
 * SMTP
 */

typedef struct {
  conn_t *c;
  mail_ctx_t *ctx;
  char line[LINE_LEN];
  char caps[512];         /* EHLO capability lines, upper case */
  unsigned char plain[MAIL_USER_LEN * 2 + MAIL_PASS_LEN + 4];
  char cmd[16 + ((MAIL_USER_LEN * 2 + MAIL_PASS_LEN + 4) * 4 / 3 + 8)];
} smtp_t;

/* read a (possibly multi-line) reply; returns the 3 digit code or <0 */
static int smtp_reply(smtp_t *m, int collect_caps) {
  int r, code = 0, more;

  if (collect_caps) m->caps[0] = 0;
  for (;;) {
    if ((r = conn_readline(m->c, m->line, sizeof(m->line))) < 0) return r;
    debug(DEBUG_INFO, TAG, "S: %.120s", m->line);
    if (r < 3 || !sys_isdigit((unsigned char)m->line[0])) {
      set_error(m->ctx, "Malformed SMTP reply", m->line);
      return MAIL_ERR_PROTO;
    }
    code = sys_atoi(m->line);
    more = m->line[3] == '-';
    if (collect_caps && sys_strlen(m->caps) + r + 2 < sizeof(m->caps)) {
      char *p;
      sys_strcat(m->caps, " ");
      sys_strcat(m->caps, m->line + 4);
      for (p = m->caps; *p; p++) *p = sys_toupper(*p);
    }
    if (!more) break;
  }

  return code;
}

static int smtp_cmd(smtp_t *m, const char *cmd, int expect, int collect_caps, int secret) {
  char *buf;
  int r, n = sys_strlen(cmd);

  if ((buf = xmalloc(n + 3)) == NULL) return MAIL_ERR_MEM;
  sys_memcpy(buf, cmd, n);
  buf[n] = '\r'; buf[n + 1] = '\n'; buf[n + 2] = 0;
  debug(DEBUG_INFO, TAG, "C: %s", secret ? "****" : cmd);
  r = conn_write(m->c, buf, n + 2);
  xfree(buf);
  if (r != MAIL_OK) return r;

  if ((r = smtp_reply(m, collect_caps)) < 0) return r;
  if (expect > 0 && r != expect) {
    return r;   /* caller decides */
  }
  return r;
}

static int smtp_expect(smtp_t *m, int code, int expect, const char *what) {
  if (code < 0) return code;
  if (code != expect && !(expect == 250 && code == 251)) {
    set_error(m->ctx, what, m->line);
    return (code == 535 || code == 534 || code == 530) ? MAIL_ERR_AUTH : MAIL_ERR_PROTO;
  }
  return MAIL_OK;
}

static int smtp_ehlo(smtp_t *m, const char *domain) {
  char cmd[160];
  int code;

  sys_snprintf(cmd, sizeof(cmd), "EHLO %s", domain);
  code = smtp_cmd(m, cmd, 250, 1, 0);
  if (code < 0) return code;
  if (code != 250) {
    sys_snprintf(cmd, sizeof(cmd), "HELO %s", domain);
    code = smtp_cmd(m, cmd, 250, 0, 0);
    return smtp_expect(m, code, 250, "Server rejected HELO");
  }
  return MAIL_OK;
}

static int smtp_auth(smtp_t *m, mail_account_t *a) {
  unsigned char *plain = m->plain;
  char *b64, *cmd = m->cmd;
  int n, ul, pl, code, r;
  int has_plain = m->caps[0] == 0 || sys_strstr(m->caps, "PLAIN") != NULL;
  int has_login = m->caps[0] == 0 || sys_strstr(m->caps, "LOGIN") != NULL;

  if (a->user[0] == 0) return MAIL_OK;
  if ((r = progress(m->ctx, "Logging in...")) != MAIL_OK) return r;

  ul = sys_strlen(a->user);
  pl = sys_strlen(a->pass);
  b64 = cmd + 11;

  if (has_plain) {
    /* AUTH PLAIN: \0user\0pass */
    n = 0;
    plain[n++] = 0;
    sys_memcpy(plain + n, a->user, ul); n += ul;
    plain[n++] = 0;
    sys_memcpy(plain + n, a->pass, pl); n += pl;
    sys_memcpy(cmd, "AUTH PLAIN ", 11);
    base64_encode(plain, n, b64);
    code = smtp_cmd(m, cmd, 235, 0, 1);
    if (code < 0) return code;
    if (code == 235) return MAIL_OK;
    /* 535/534/530 = bad credentials: trying another mechanism is pointless */
    if (!has_login || code == 535 || code == 534 || code == 530) return smtp_expect(m, code, 235, "Login rejected");
  }

  /* AUTH LOGIN */
  code = smtp_cmd(m, "AUTH LOGIN", 334, 0, 0);
  if (code < 0) return code;
  if (code != 334) return smtp_expect(m, code, 334, "Login rejected");
  base64_encode((const unsigned char *)a->user, ul, cmd);
  code = smtp_cmd(m, cmd, 334, 0, 1);
  if (code < 0) return code;
  if (code != 334) return smtp_expect(m, code, 334, "Login rejected");
  base64_encode((const unsigned char *)a->pass, pl, cmd);
  code = smtp_cmd(m, cmd, 235, 0, 1);
  return smtp_expect(m, code, 235, "Login rejected");
}

int mail_smtp_send(mail_ctx_t *ctx, mail_account_t *a, const char *rcpts, const char *data, int len) {
  smtp_t *m;
  char (*addrs)[MAIL_ADDR_LEN];
  char cmd[MAIL_ADDR_LEN + 24], from[MAIL_ADDR_LEN], domain[MAIL_ADDR_LEN];
  const char *p, *eol;
  int r, i, n, code, nrcpt;

  if (ctx) ctx->error[0] = 0;
  if (a == NULL || rcpts == NULL || data == NULL) return MAIL_ERR_ARG;
  if (a->smtp_host[0] == 0 || a->smtp_port == 0) {
    set_error(ctx, "SMTP server is not configured", NULL);
    return MAIL_ERR_ARG;
  }
  if (!mail_extract_address(a->email, from, sizeof(from))) {
    set_error(ctx, "The account has no valid e-mail address", NULL);
    return MAIL_ERR_ARG;
  }

  if ((addrs = xcalloc(MAX_RCPTS, MAIL_ADDR_LEN)) == NULL) return MAIL_ERR_MEM;
  nrcpt = split_recipients(rcpts, addrs, MAX_RCPTS);
  if (nrcpt == 0) {
    xfree(addrs);
    set_error(ctx, "No valid recipient address", NULL);
    return MAIL_ERR_ARG;
  }

  if ((m = xcalloc(1, sizeof(smtp_t))) == NULL) {
    xfree(addrs);
    return MAIL_ERR_MEM;
  }
  m->ctx = ctx;
  if ((m->c = conn_new(ctx)) == NULL) {
    xfree(addrs);
    xfree(m);
    return MAIL_ERR_MEM;
  }

  /* the domain we announce in EHLO */
  p = sys_strchr(from, '@');
  str_copy(domain, p ? p + 1 : "pumpkinos.local", sizeof(domain));

  if ((r = conn_open(m->c, a->smtp_host, a->smtp_port, a->smtp_sec)) != MAIL_OK) goto done;

  code = smtp_reply(m, 0);
  if ((r = smtp_expect(m, code, 220, "Unexpected SMTP greeting")) != MAIL_OK) goto done;

  if ((r = smtp_ehlo(m, domain)) != MAIL_OK) goto done;

  if (a->smtp_sec == MAIL_SEC_STARTTLS) {
    if (m->caps[0] && sys_strstr(m->caps, "STARTTLS") == NULL) {
      set_error(ctx, "Server does not offer STARTTLS", NULL);
      r = MAIL_ERR_PROTO;
      goto done;
    }
    code = smtp_cmd(m, "STARTTLS", 220, 0, 0);
    if ((r = smtp_expect(m, code, 220, "Server refused STARTTLS")) != MAIL_OK) goto done;
    if ((r = conn_tls(m->c)) != MAIL_OK) goto done;
    if ((r = smtp_ehlo(m, domain)) != MAIL_OK) goto done;
  }

  if ((r = smtp_auth(m, a)) != MAIL_OK) goto done;

  if ((r = progress(ctx, "Sending message...")) != MAIL_OK) goto done;

  sys_snprintf(cmd, sizeof(cmd), "MAIL FROM:<%s>", from);
  code = smtp_cmd(m, cmd, 250, 0, 0);
  if ((r = smtp_expect(m, code, 250, "Sender rejected")) != MAIL_OK) goto done;

  for (i = 0; i < nrcpt; i++) {
    sys_snprintf(cmd, sizeof(cmd), "RCPT TO:<%s>", addrs[i]);
    code = smtp_cmd(m, cmd, 250, 0, 0);
    if ((r = smtp_expect(m, code, 250, "Recipient rejected")) != MAIL_OK) goto done;
  }

  code = smtp_cmd(m, "DATA", 354, 0, 0);
  if ((r = smtp_expect(m, code, 354, "DATA rejected")) != MAIL_OK) goto done;

  /* send the message line by line with dot stuffing */
  p = data;
  while (p < data + len) {
    eol = p;
    while (eol < data + len && *eol != '\n') eol++;
    n = (int)(eol - p);
    if (n > 0 && p[n - 1] == '\r') n--;
    if (n > 0 && p[0] == '.') {
      if ((r = conn_write(m->c, ".", 1)) != MAIL_OK) goto done;
    }
    if (n > 0 && (r = conn_write(m->c, p, n)) != MAIL_OK) goto done;
    if ((r = conn_write(m->c, "\r\n", 2)) != MAIL_OK) goto done;
    p = eol < data + len ? eol + 1 : eol;
  }
  if ((r = conn_write(m->c, ".\r\n", 3)) != MAIL_OK) goto done;

  code = smtp_reply(m, 0);
  if ((r = smtp_expect(m, code, 250, "Message rejected")) != MAIL_OK) goto done;

  debug(DEBUG_INFO, TAG, "message accepted: %s", m->line);
  conn_writes(m->c, "QUIT\r\n");
  r = MAIL_OK;

done:
  conn_close(m->c);
  xfree(m);
  xfree(addrs);
  return r;
}

/*
 * Message construction
 */

static const char *day_names[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
static const char *month_names[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };

static void rfc_date(char *out, int size, uint64_t *epoch) {
  sys_tm_t tm;
  uint64_t t = sys_time();

  sys_memset(&tm, 0, sizeof(tm));
  sys_gmtime(&t, &tm);
  if (epoch) *epoch = t;
  sys_snprintf(out, size, "%s, %d %s %d %02d:%02d:%02d +0000",
    day_names[tm.tm_wday >= 0 && tm.tm_wday < 7 ? tm.tm_wday : 0], tm.tm_mday,
    month_names[tm.tm_mon >= 0 && tm.tm_mon < 12 ? tm.tm_mon : 0],
    tm.tm_year < 1900 ? tm.tm_year + 1900 : tm.tm_year, tm.tm_hour, tm.tm_min, tm.tm_sec);
}

typedef struct {
  char *buf;
  int len, cap;
} strbuf_t;

static int sb_append(strbuf_t *sb, const char *s, int n) {
  char *tmp;

  if (n < 0) n = sys_strlen(s);
  if (sb->len + n + 1 > sb->cap) {
    int cap = sb->cap ? sb->cap : 2048;
    while (cap < sb->len + n + 1) cap *= 2;
    if ((tmp = xrealloc(sb->buf, cap)) == NULL) return -1;
    sb->buf = tmp;
    sb->cap = cap;
  }
  sys_memcpy(sb->buf + sb->len, s, n);
  sb->len += n;
  sb->buf[sb->len] = 0;

  return 0;
}

/* quoted-printable encode a Latin-1 text ('\n' line ends) as CRLF lines of at most 76 chars */
static int qp_encode_body(strbuf_t *sb, const char *body) {
  static const char hex[] = "0123456789ABCDEF";
  const unsigned char *p = (const unsigned char *)body;
  char enc[4];
  int col = 0, n;

  while (*p) {
    if (*p == '\n') {
      if (sb_append(sb, "\r\n", 2) < 0) return -1;
      col = 0;
      p++;
      continue;
    }
    if (*p == '\r') { p++; continue; }

    if ((*p == ' ' || *p == '\t') && (p[1] == '\n' || p[1] == 0)) {
      /* trailing whitespace must be encoded */
      enc[0] = '='; enc[1] = hex[*p >> 4]; enc[2] = hex[*p & 15];
      n = 3;
    } else if ((*p >= 33 && *p <= 126 && *p != '=') || *p == ' ' || *p == '\t') {
      enc[0] = *p;
      n = 1;
    } else {
      enc[0] = '='; enc[1] = hex[*p >> 4]; enc[2] = hex[*p & 15];
      n = 3;
    }
    if (col + n > 75) {
      if (sb_append(sb, "=\r\n", 3) < 0) return -1;
      col = 0;
    }
    if (sb_append(sb, enc, n) < 0) return -1;
    col += n;
    p++;
  }
  if (col > 0) {
    if (sb_append(sb, "\r\n", 2) < 0) return -1;
  }

  return 0;
}

char *mail_build_message(mail_account_t *a, const char *to, const char *cc,
                         const char *subject, const char *in_reply_to,
                         const char *body, int *len) {
  strbuf_t sb;
  char date[64], from[MAIL_ADDR_LEN], encoded[MAIL_SUBJECT_LEN * 4 + 64], line[512];
  char domain[MAIL_ADDR_LEN];
  const char *p;
  uint64_t t = 0;

  sys_memset(&sb, 0, sizeof(sb));
  *len = 0;

  rfc_date(date, sizeof(date), &t);
  mail_extract_address(a->email, from, sizeof(from));
  p = sys_strchr(from, '@');
  str_copy(domain, p ? p + 1 : "pumpkinos.local", sizeof(domain));

  sys_snprintf(line, sizeof(line), "Date: %s\r\n", date);
  sb_append(&sb, line, -1);

  if (a->name[0]) {
    encode_header_value(a->name, encoded, sizeof(encoded));
    if (encoded[0] == '=' || sys_strchr(encoded, '"') || sys_strchr(encoded, ',') || sys_strchr(encoded, '<') || sys_strchr(encoded, '@')) {
      sys_snprintf(line, sizeof(line), "From: %s <%s>\r\n", encoded, from);
    } else {
      sys_snprintf(line, sizeof(line), "From: \"%s\" <%s>\r\n", encoded, from);
    }
  } else {
    sys_snprintf(line, sizeof(line), "From: <%s>\r\n", from);
  }
  sb_append(&sb, line, -1);

  sb_append(&sb, "To: ", 4);
  sb_append(&sb, to, -1);
  sb_append(&sb, "\r\n", 2);
  if (cc && cc[0]) {
    sb_append(&sb, "Cc: ", 4);
    sb_append(&sb, cc, -1);
    sb_append(&sb, "\r\n", 2);
  }

  encode_header_value(subject && subject[0] ? subject : "(no subject)", encoded, sizeof(encoded));
  sb_append(&sb, "Subject: ", 9);
  sb_append(&sb, encoded, -1);
  sb_append(&sb, "\r\n", 2);

  sys_snprintf(line, sizeof(line), "Message-ID: <%u.%u.pumpkin@%s>\r\n", (unsigned int)t, (unsigned int)(sys_rand() & 0xFFFF), domain);
  sb_append(&sb, line, -1);

  if (in_reply_to && in_reply_to[0]) {
    sys_snprintf(line, sizeof(line), "In-Reply-To: %s\r\nReferences: %s\r\n", in_reply_to, in_reply_to);
    sb_append(&sb, line, -1);
  }

  sb_append(&sb, "MIME-Version: 1.0\r\n", -1);
  sb_append(&sb, "Content-Type: text/plain; charset=ISO-8859-1\r\n", -1);
  sb_append(&sb, "Content-Transfer-Encoding: quoted-printable\r\n", -1);
  sb_append(&sb, "X-Mailer: PumpkinOS Mail\r\n", -1);
  sb_append(&sb, "\r\n", 2);

  if (qp_encode_body(&sb, body ? body : "") < 0 || sb.buf == NULL) {
    if (sb.buf) xfree(sb.buf);
    return NULL;
  }

  *len = sb.len;
  return sb.buf;
}
