/*
 * liblopenssl - TLS provider for PumpkinOS built on OpenSSL (1.1.0 or newer).
 *
 * Registers a secure_provider_t under SECURE_PROVIDER so that libos passes it
 * to the HTTP client, which makes https:// URLs work in pumpkin_http_get()
 * and therefore in applications such as the Browser.
 *
 * Lua interface (all optional, call before the first https request):
 *   s = pit.loadlib("liblopenssl")
 *   s.verify(false)        -- disable certificate verification (default: on)
 *   s.cacert("/path.pem")  -- CA bundle to use instead of the system default
 *   s.cert(pem)            -- client certificate (PEM text)
 *   s.key(pem)             -- client private key (PEM text)
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#ifdef WINDOWS
#include <winsock2.h>
#else
#ifndef WINDOWS
#ifndef SSIZE_MAX
#define SSIZE_MAX LONG_MAX
#endif
#endif
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#ifdef ANDROID
#include <dirent.h>
#endif
#endif

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/x509v3.h>
#include <openssl/opensslv.h>

#include "script.h"
#include "thread.h"
#include "sys.h"
#include "secure.h"
#include "debug.h"
#include "xalloc.h"

#if OPENSSL_VERSION_NUMBER < 0x10100000L
#error "liblopenssl requires OpenSSL 1.1.0 or newer"
#endif

#define TAG "SECURE"

/* how long one wait slice is; thread_must_end() is checked between slices */
#define WAIT_SLICE_MS 200

/* candidate CA bundles used when the OpenSSL default location has none */
static const char *ca_bundles[] = {
  "/etc/ssl/cert.pem",                                  /* macOS, FreeBSD, Alpine */
  "/etc/ssl/certs/ca-certificates.crt",                 /* Debian, Ubuntu, Arch */
  "/etc/pki/tls/certs/ca-bundle.crt",                   /* Fedora, RHEL */
  "/etc/ssl/ca-bundle.pem",                             /* openSUSE */
  "/opt/homebrew/etc/openssl@3/cert.pem",               /* Homebrew (Apple Silicon) */
  "/usr/local/etc/openssl@3/cert.pem",                  /* Homebrew (Intel) */
  "/usr/local/etc/openssl/cert.pem",
  NULL
};

#ifdef ANDROID
/* Android has no PEM bundle. The system trust store is a directory of
 * individual PEM files named after the legacy (MD5) subject hash, which the
 * OpenSSL hashed-directory lookup (SHA1 based) cannot resolve, so every file
 * is loaded explicitly. Newer releases update the store through the
 * conscrypt APEX module; it is preferred when present. */
static const char *android_ca_dirs[] = {
  "/apex/com.android.conscrypt/cacerts",
  "/system/etc/security/cacerts",
  NULL
};
#endif

struct secure_config_t {
  SSL_CTX *ctx;
  int verify;
};

struct secure_t {
  SSL *ssl;
  int fd;
};

static secure_provider_t provider;
static int opt_verify = 1;
static char *opt_cacert = NULL;

static void liblopenssl_error(const char *what) {
  char buf[256];
  unsigned long e;
  int n = 0;

  while ((e = ERR_get_error()) != 0) {
    ERR_error_string_n(e, buf, sizeof(buf));
    debug(DEBUG_ERROR, TAG, "%s: %s", what, buf);
    n++;
  }
  if (n == 0) debug(DEBUG_ERROR, TAG, "%s", what);
}

/* wait until fd is readable (want == 0) or writable (want == 1); returns
   1 when ready, 0 on timeout (us == 0 means "forever"), -1 on error or
   when the thread was asked to end */
static int wait_fd(int fd, int want, uint32_t us) {
  uint32_t left = us;
  int slice, r;
#ifdef WINDOWS
  fd_set fds;
  struct timeval tv;
#else
  struct pollfd pfd;
#endif

  for (;;) {
    if (thread_must_end()) return -1;
    slice = WAIT_SLICE_MS;
    if (us && (uint32_t)slice * 1000 > left) slice = (left + 999) / 1000;

#ifdef WINDOWS
    FD_ZERO(&fds);
    FD_SET((SOCKET)fd, &fds);
    tv.tv_sec = slice / 1000;
    tv.tv_usec = (slice % 1000) * 1000;
    r = select(fd + 1, want ? NULL : &fds, want ? &fds : NULL, NULL, &tv);
    if (r < 0 && WSAGetLastError() == WSAEINTR) continue;
#else
    pfd.fd = fd;
    pfd.events = want ? POLLOUT : POLLIN;
    pfd.revents = 0;
    r = poll(&pfd, 1, slice);
    if (r < 0 && errno == EINTR) continue;
#endif
    if (r < 0) {
      debug_errno(TAG, "poll");
      return -1;
    }
    if (r > 0) return 1;

    if (us) {
      if (left <= (uint32_t)slice * 1000) return 0;
      left -= slice * 1000;
    }
  }
}

/* handle the result of a non-blocking SSL call: returns 1 to retry,
   0 for a clean shutdown by the peer, -1 for a real error */
static int handle_ssl_result(secure_t *s, int r, const char *what) {
  int err = SSL_get_error(s->ssl, r);

  switch (err) {
    case SSL_ERROR_WANT_READ:
      return wait_fd(s->fd, 0, 0) > 0 ? 1 : -1;
    case SSL_ERROR_WANT_WRITE:
      return wait_fd(s->fd, 1, 0) > 0 ? 1 : -1;
    case SSL_ERROR_ZERO_RETURN:
      return 0;
    case SSL_ERROR_SYSCALL:
      if (r == 0) return 0;   /* EOF without close_notify, common on the web */
      ERR_clear_error();
      debug(DEBUG_INFO, TAG, "%s: connection closed (%d)", what, sys_errno());
      return -1;
    default:
      liblopenssl_error(what);
      return -1;
  }
}

static int load_pem_cert_key(SSL_CTX *ctx, char *cert, char *key) {
  BIO *bio;
  X509 *x509 = NULL;
  EVP_PKEY *pkey = NULL;
  int r = -1;

  if ((bio = BIO_new_mem_buf(cert, -1)) != NULL) {
    x509 = PEM_read_bio_X509(bio, NULL, NULL, NULL);
    BIO_free(bio);
  }
  if ((bio = BIO_new_mem_buf(key, -1)) != NULL) {
    pkey = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);
    BIO_free(bio);
  }

  if (x509 && pkey && SSL_CTX_use_certificate(ctx, x509) == 1 && SSL_CTX_use_PrivateKey(ctx, pkey) == 1) {
    r = SSL_CTX_check_private_key(ctx) == 1 ? 0 : -1;
  }
  if (r != 0) liblopenssl_error("error loading certificate/key");

  if (x509) X509_free(x509);
  if (pkey) EVP_PKEY_free(pkey);

  return r;
}

static int has_ca_certs(SSL_CTX *ctx) {
  X509_STORE *store = SSL_CTX_get_cert_store(ctx);
  STACK_OF(X509_OBJECT) *objs;

  if (store == NULL) return 0;
  objs = X509_STORE_get0_objects(store);
  return objs && sk_X509_OBJECT_num(objs) > 0;
}

#ifdef ANDROID
static int load_ca_dir(SSL_CTX *ctx, const char *dir) {
  X509_STORE *store = SSL_CTX_get_cert_store(ctx);
  struct dirent *e;
  DIR *d;
  BIO *bio;
  X509 *x;
  char path[512];
  int n = 0;

  if (store == NULL || (d = opendir(dir)) == NULL) return 0;

  while ((e = readdir(d)) != NULL) {
    if (e->d_name[0] == '.') continue;
    snprintf(path, sizeof(path), "%s/%s", dir, e->d_name);
    if ((bio = BIO_new_file(path, "r")) == NULL) {
      ERR_clear_error();
      continue;
    }
    /* each file holds one PEM certificate followed by a textual dump */
    while ((x = PEM_read_bio_X509(bio, NULL, NULL, NULL)) != NULL) {
      if (X509_STORE_add_cert(store, x) == 1) n++;
      X509_free(x);
    }
    ERR_clear_error();
    BIO_free(bio);
  }
  closedir(d);

  return n;
}
#endif

static void load_ca_certs(SSL_CTX *ctx) {
  int i;

  if (opt_cacert) {
    if (SSL_CTX_load_verify_locations(ctx, opt_cacert, NULL) == 1) {
      debug(DEBUG_INFO, TAG, "using CA bundle %s", opt_cacert);
      return;
    }
    liblopenssl_error("error loading CA bundle");
  }

  /* SSL_CERT_FILE / SSL_CERT_DIR or the compiled-in OpenSSL directory */
  if (SSL_CTX_set_default_verify_paths(ctx) == 1 && has_ca_certs(ctx)) return;
  ERR_clear_error();

#ifdef ANDROID
  for (i = 0; android_ca_dirs[i]; i++) {
    int n = load_ca_dir(ctx, android_ca_dirs[i]);
    if (n > 0) {
      debug(DEBUG_INFO, TAG, "loaded %d CA certificates from %s", n, android_ca_dirs[i]);
      return;
    }
  }
#endif

  for (i = 0; ca_bundles[i]; i++) {
    if (SSL_CTX_load_verify_locations(ctx, ca_bundles[i], NULL) == 1) {
      debug(DEBUG_INFO, TAG, "using CA bundle %s", ca_bundles[i]);
      return;
    }
    ERR_clear_error();
  }

  debug(DEBUG_ERROR, TAG, "no CA certificate bundle found, certificate verification will fail");
}

static secure_config_t *liblopenssl_new(char *cert, char *key) {
  secure_config_t *c;
  SSL_CTX *ctx;

  if ((ctx = SSL_CTX_new(TLS_client_method())) == NULL) {
    liblopenssl_error("error creating SSL context");
    return NULL;
  }

  SSL_CTX_set_min_proto_version(ctx, TLS1_2_VERSION);
  SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER | SSL_MODE_ENABLE_PARTIAL_WRITE);
  SSL_CTX_set_options(ctx, SSL_OP_NO_COMPRESSION);

  if (cert && key && load_pem_cert_key(ctx, cert, key) != 0) {
    SSL_CTX_free(ctx);
    return NULL;
  }

  if (opt_verify) {
    load_ca_certs(ctx);
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
  } else {
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
  }

  if ((c = xcalloc(1, sizeof(secure_config_t))) == NULL) {
    SSL_CTX_free(ctx);
    return NULL;
  }

  c->ctx = ctx;
  c->verify = opt_verify;
  debug(DEBUG_INFO, TAG, "configuration created (verify %s)", c->verify ? "on" : "off");

  return c;
}

static int liblopenssl_destroy(secure_config_t *c) {
  if (c == NULL) return -1;
  if (c->ctx) SSL_CTX_free(c->ctx);
  xfree(c);
  return 0;
}

static secure_t *liblopenssl_connect(secure_config_t *c, char *host, int port, int fd) {
  secure_t *s;
  SSL *ssl;
  long vr;
  int r, w;
#ifdef SO_NOSIGPIPE
  int one = 1;
#endif

  if (c == NULL || c->ctx == NULL) return NULL;

#ifdef SO_NOSIGPIPE
  setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one));
#endif

  if ((ssl = SSL_new(c->ctx)) == NULL) {
    liblopenssl_error("error creating SSL connection");
    return NULL;
  }

  if (host && host[0]) {
    /* SNI and hostname verification */
    SSL_set_tlsext_host_name(ssl, host);
    if (c->verify) {
      SSL_set_hostflags(ssl, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
      if (SSL_set1_host(ssl, host) != 1) {
        liblopenssl_error("error setting host name");
        SSL_free(ssl);
        return NULL;
      }
    }
  }

  if (SSL_set_fd(ssl, fd) != 1) {
    liblopenssl_error("error setting file descriptor");
    SSL_free(ssl);
    return NULL;
  }

  if ((s = xcalloc(1, sizeof(secure_t))) == NULL) {
    SSL_free(ssl);
    return NULL;
  }
  s->ssl = ssl;
  s->fd = fd;

  debug(DEBUG_INFO, TAG, "negotiating with %s:%d ...", host ? host : "", port);
  SSL_set_connect_state(ssl);

  for (;;) {
    r = SSL_do_handshake(ssl);
    if (r == 1) break;
    if ((w = handle_ssl_result(s, r, "handshake failed")) <= 0) {
      if (w == 0) debug(DEBUG_ERROR, TAG, "connection closed during handshake");
      if (c->verify && (vr = SSL_get_verify_result(ssl)) != X509_V_OK) {
        debug(DEBUG_ERROR, TAG, "certificate verification failed for %s: %s", host ? host : "", X509_verify_cert_error_string(vr));
      }
      SSL_free(ssl);
      xfree(s);
      return NULL;
    }
  }

  if (c->verify && (vr = SSL_get_verify_result(ssl)) != X509_V_OK) {
    debug(DEBUG_ERROR, TAG, "certificate verification failed for %s: %s", host ? host : "", X509_verify_cert_error_string(vr));
    SSL_free(ssl);
    xfree(s);
    return NULL;
  }

  debug(DEBUG_INFO, TAG, "%s, cipher %s, connected on fd %d", SSL_get_version(ssl), SSL_get_cipher_name(ssl), fd);

  return s;
}

static int liblopenssl_peek(secure_t *s, uint32_t us) {
  if (s == NULL) return -1;
  if (SSL_pending(s->ssl) > 0) return 1;

  return sys_select(s->fd, us);
}

static int liblopenssl_read(secure_t *s, char *buf, int len) {
  int r, w;

  if (s == NULL || len <= 0) return -1;

  for (;;) {
    r = SSL_read(s->ssl, buf, len);
    if (r > 0) return r;
    if ((w = handle_ssl_result(s, r, "read failed")) <= 0) return w;
  }
}

static int liblopenssl_write(secure_t *s, char *buf, int len) {
  int nwritten = 0;
  int r, w;

  if (s == NULL) return -1;

  while (nwritten < len) {
    r = SSL_write(s->ssl, buf + nwritten, len - nwritten);
    if (r > 0) {
      nwritten += r;
      continue;
    }
    if ((w = handle_ssl_result(s, r, "write failed")) <= 0) break;
  }

  return nwritten;
}

static int liblopenssl_close(secure_t *s) {
  if (s == NULL) return -1;

  /* one-shot shutdown: send close_notify but do not wait for the peer */
  if (SSL_shutdown(s->ssl) < 0) ERR_clear_error();
  SSL_free(s->ssl);
  xfree(s);

  return 0;
}

static int set_string_option(int pe, char **opt) {
  char *s = NULL;
  int len, r = -1;

  if (script_get_lstring(pe, 0, &s, &len) == 0 && s) {
    if (*opt) xfree(*opt);
    *opt = xstrdup(s);
    r = 0;
  }
  if (s) xfree(s);

  return script_push_boolean(pe, r == 0);
}

static int liblopenssl_cert(int pe) {
  return set_string_option(pe, &provider.cert);
}

static int liblopenssl_key(int pe) {
  return set_string_option(pe, &provider.key);
}

static int liblopenssl_cacert(int pe) {
  return set_string_option(pe, &opt_cacert);
}

static int liblopenssl_verify(int pe) {
  int b;

  if (script_get_boolean(pe, 0, &b) == 0) {
    opt_verify = b ? 1 : 0;
    debug(DEBUG_INFO, TAG, "certificate verification %s", opt_verify ? "enabled" : "disabled");
    return script_push_boolean(pe, 1);
  }

  return script_push_boolean(pe, 0);
}

int liblopenssl_load(void) {
  if (OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS, NULL) != 1) {
    liblopenssl_error("OPENSSL_init_ssl failed");
    return -1;
  }

  provider.new = liblopenssl_new;
  provider.destroy = liblopenssl_destroy;
  provider.connect = liblopenssl_connect;
  provider.peek = liblopenssl_peek;
  provider.read = liblopenssl_read;
  provider.write = liblopenssl_write;
  provider.close = liblopenssl_close;

  debug(DEBUG_INFO, TAG, "%s", OpenSSL_version(OPENSSL_VERSION));

  return 0;
}

int liblopenssl_init(int pe, script_ref_t obj) {
  debug(DEBUG_INFO, TAG, "registering provider %s", SECURE_PROVIDER);
  script_set_pointer(pe, SECURE_PROVIDER, &provider);

  script_add_function(pe, obj, "cert",   liblopenssl_cert);
  script_add_function(pe, obj, "key",    liblopenssl_key);
  script_add_function(pe, obj, "cacert", liblopenssl_cacert);
  script_add_function(pe, obj, "verify", liblopenssl_verify);

  return 0;
}
