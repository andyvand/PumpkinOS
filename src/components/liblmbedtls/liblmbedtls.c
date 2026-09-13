/*
 * liblmbedtls - TLS provider for the ESP32 PumpkinOS firmware, built on the
 * mbedTLS library that ships with ESP-IDF and its certificate bundle
 * (CONFIG_MBEDTLS_CERTIFICATE_BUNDLE).
 *
 * It registers a secure_provider_t under SECURE_PROVIDER so that libos hands
 * it to the HTTP client, which makes https:// URLs work in pumpkin_http_get()
 * (used by the Browser application).
 */

#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/socket.h>

#include "mbedtls/build_info.h"
#include "mbedtls/ssl.h"
#include "mbedtls/net_sockets.h"
#include "mbedtls/error.h"
#include "esp_crt_bundle.h"

/* mbedTLS 4.x (ESP-IDF v6) removed the entropy/CTR-DRBG public API and
   mbedtls_ssl_conf_rng(): TLS now always uses the PSA Crypto RNG, which only
   needs psa_crypto_init(). mbedTLS 3.x (ESP-IDF v5) still needs the explicit
   RNG plumbing, so keep both paths. */
#if MBEDTLS_VERSION_MAJOR >= 4
#define LIBLMBEDTLS_PSA_RNG 1
#include "psa/crypto.h"
#else
#define LIBLMBEDTLS_PSA_RNG 0
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#endif

#include "script.h"
#include "thread.h"
#include "sys.h"
#include "secure.h"
#include "debug.h"
#include "xalloc.h"

#define TAG "SECURE"

/* how long one wait slice is; thread_must_end() is checked between slices */
#define WAIT_SLICE_MS 200

struct secure_config_t {
  mbedtls_ssl_config conf;
#if !LIBLMBEDTLS_PSA_RNG
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;
#endif
  int verify;
};

struct secure_t {
  mbedtls_ssl_context ssl;
  mbedtls_net_context net;
  int fd;
};

static secure_provider_t provider;
static int opt_verify = 1;

static void liblmbedtls_error(const char *what, int ret) {
  char buf[128];

  mbedtls_strerror(ret, buf, sizeof(buf));
  debug(DEBUG_ERROR, TAG, "%s: -0x%04x (%s)", what, (unsigned int)-ret, buf);
}

/* wait until fd is readable (want == 0) or writable (want == 1); returns
   1 when ready, 0 on timeout (us == 0 means "forever"), -1 on error or
   when the thread was asked to end */
static int wait_fd(int fd, int want, uint32_t us) {
  uint32_t left = us;
  struct timeval tv;
  fd_set fds;
  int slice, r;

  for (;;) {
    if (thread_must_end()) return -1;
    slice = WAIT_SLICE_MS;
    if (us && (uint32_t)slice * 1000 > left) slice = (left + 999) / 1000;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);
    tv.tv_sec = slice / 1000;
    tv.tv_usec = (slice % 1000) * 1000;
    r = select(fd + 1, want ? NULL : &fds, want ? &fds : NULL, NULL, &tv);
    if (r < 0 && errno == EINTR) continue;
    if (r < 0) {
      debug_errno(TAG, "select");
      return -1;
    }
    if (r > 0) return 1;

    if (us) {
      if (left <= (uint32_t)slice * 1000) return 0;
      left -= slice * 1000;
    }
  }
}

/* handle the result of a non-blocking mbedTLS call: returns 1 to retry,
   0 for a clean shutdown by the peer, -1 for a real error */
static int handle_ssl_result(secure_t *s, int ret, const char *what) {
  switch (ret) {
    case MBEDTLS_ERR_SSL_WANT_READ:
      return wait_fd(s->fd, 0, 0) > 0 ? 1 : -1;
    case MBEDTLS_ERR_SSL_WANT_WRITE:
      return wait_fd(s->fd, 1, 0) > 0 ? 1 : -1;
    case 0:
    case MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY:
    case MBEDTLS_ERR_NET_CONN_RESET:
      return 0;
    default:
      liblmbedtls_error(what, ret);
      return -1;
  }
}

static secure_config_t *liblmbedtls_new(char *cert, char *key) {
  secure_config_t *c;
  int ret;

  if ((c = xcalloc(1, sizeof(secure_config_t))) == NULL) {
    return NULL;
  }

  mbedtls_ssl_config_init(&c->conf);

#if LIBLMBEDTLS_PSA_RNG
  /* ESP-IDF already runs psa_crypto_init() during system init; calling it
     again is cheap and idempotent, and makes this file self-contained */
  {
    psa_status_t status = psa_crypto_init();
    if (status != PSA_SUCCESS) {
      debug(DEBUG_ERROR, TAG, "psa_crypto_init failed: %d", (int)status);
      provider.destroy(c);
      return NULL;
    }
  }
#else
  mbedtls_entropy_init(&c->entropy);
  mbedtls_ctr_drbg_init(&c->ctr_drbg);

  if ((ret = mbedtls_ctr_drbg_seed(&c->ctr_drbg, mbedtls_entropy_func, &c->entropy, (const unsigned char *)"PumpkinOS", 9)) != 0) {
    liblmbedtls_error("mbedtls_ctr_drbg_seed failed", ret);
    provider.destroy(c);
    return NULL;
  }
#endif

  if ((ret = mbedtls_ssl_config_defaults(&c->conf, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM, MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
    liblmbedtls_error("mbedtls_ssl_config_defaults failed", ret);
    provider.destroy(c);
    return NULL;
  }

#if !LIBLMBEDTLS_PSA_RNG
  mbedtls_ssl_conf_rng(&c->conf, mbedtls_ctr_drbg_random, &c->ctr_drbg);
#endif
  c->verify = opt_verify;

  if (c->verify) {
    if (esp_crt_bundle_attach(&c->conf) != ESP_OK) {
      debug(DEBUG_ERROR, TAG, "esp_crt_bundle_attach failed, certificate verification will fail");
    }
    mbedtls_ssl_conf_authmode(&c->conf, MBEDTLS_SSL_VERIFY_REQUIRED);
  } else {
    mbedtls_ssl_conf_authmode(&c->conf, MBEDTLS_SSL_VERIFY_NONE);
  }

  /* client certificates (cert/key) are not supported by this provider */
  debug(DEBUG_INFO, TAG, "configuration created (verify %s)", c->verify ? "on" : "off");

  return c;
}

static int liblmbedtls_destroy(secure_config_t *c) {
  if (c == NULL) return -1;

  if (c->verify) esp_crt_bundle_detach(&c->conf);
  mbedtls_ssl_config_free(&c->conf);
#if !LIBLMBEDTLS_PSA_RNG
  mbedtls_ctr_drbg_free(&c->ctr_drbg);
  mbedtls_entropy_free(&c->entropy);
#endif
  xfree(c);

  return 0;
}

static secure_t *liblmbedtls_connect(secure_config_t *c, char *host, int port, int fd) {
  secure_t *s;
  uint32_t flags;
  char buf[256];
  int ret, w;

  if (c == NULL) return NULL;

  if ((s = xcalloc(1, sizeof(secure_t))) == NULL) {
    return NULL;
  }

  mbedtls_ssl_init(&s->ssl);
  mbedtls_net_init(&s->net);
  s->net.fd = fd;
  s->fd = fd;

  if ((ret = mbedtls_ssl_setup(&s->ssl, &c->conf)) != 0) {
    liblmbedtls_error("mbedtls_ssl_setup failed", ret);
    mbedtls_ssl_free(&s->ssl);
    xfree(s);
    return NULL;
  }

  /* mbedTLS 4.x refuses a verified handshake unless the hostname was set
     (possibly to NULL, meaning "I know what I am doing"); always call it */
  if ((ret = mbedtls_ssl_set_hostname(&s->ssl, (host && host[0]) ? host : NULL)) != 0) {
    liblmbedtls_error("mbedtls_ssl_set_hostname failed", ret);
    mbedtls_ssl_free(&s->ssl);
    xfree(s);
    return NULL;
  }

  mbedtls_ssl_set_bio(&s->ssl, &s->net, mbedtls_net_send, mbedtls_net_recv, NULL);

  debug(DEBUG_INFO, TAG, "negotiating with %s:%d ...", host ? host : "", port);

  for (;;) {
    ret = mbedtls_ssl_handshake(&s->ssl);
    if (ret == 0) break;
    if (ret == MBEDTLS_ERR_X509_CERT_VERIFY_FAILED) {
      flags = mbedtls_ssl_get_verify_result(&s->ssl);
      mbedtls_x509_crt_verify_info(buf, sizeof(buf), "", flags);
      debug(DEBUG_ERROR, TAG, "certificate verification failed for %s: %s", host ? host : "", buf);
      mbedtls_ssl_free(&s->ssl);
      xfree(s);
      return NULL;
    }
    if ((w = handle_ssl_result(s, ret, "handshake failed")) <= 0) {
      if (w == 0) debug(DEBUG_ERROR, TAG, "connection closed during handshake");
      mbedtls_ssl_free(&s->ssl);
      xfree(s);
      return NULL;
    }
  }

  debug(DEBUG_INFO, TAG, "%s, cipher %s, connected on fd %d", mbedtls_ssl_get_version(&s->ssl), mbedtls_ssl_get_ciphersuite(&s->ssl), fd);

  return s;
}

static int liblmbedtls_peek(secure_t *s, uint32_t us) {
  if (s == NULL) return -1;
  if (mbedtls_ssl_get_bytes_avail(&s->ssl) > 0) return 1;

  return sys_select(s->fd, us);
}

static int liblmbedtls_read(secure_t *s, char *buf, int len) {
  int ret, w;

  if (s == NULL || len <= 0) return -1;

  for (;;) {
    ret = mbedtls_ssl_read(&s->ssl, (unsigned char *)buf, len);
    if (ret > 0) return ret;
    if ((w = handle_ssl_result(s, ret, "read failed")) <= 0) return w;
  }
}

static int liblmbedtls_write(secure_t *s, char *buf, int len) {
  int nwritten = 0;
  int ret, w;

  if (s == NULL) return -1;

  while (nwritten < len) {
    ret = mbedtls_ssl_write(&s->ssl, (const unsigned char *)buf + nwritten, len - nwritten);
    if (ret > 0) {
      nwritten += ret;
      continue;
    }
    if ((w = handle_ssl_result(s, ret, "write failed")) <= 0) break;
  }

  return nwritten;
}

static int liblmbedtls_close(secure_t *s) {
  if (s == NULL) return -1;

  /* one-shot close_notify: do not wait for the peer */
  mbedtls_ssl_close_notify(&s->ssl);
  mbedtls_ssl_free(&s->ssl);
  xfree(s);

  return 0;
}

static int liblmbedtls_verify(int pe) {
  int b;

  if (script_get_boolean(pe, 0, &b) == 0) {
    opt_verify = b ? 1 : 0;
    return script_push_boolean(pe, 1);
  }

  return script_push_boolean(pe, 0);
}

int liblmbedtls_load(void) {
  provider.new = liblmbedtls_new;
  provider.destroy = liblmbedtls_destroy;
  provider.connect = liblmbedtls_connect;
  provider.peek = liblmbedtls_peek;
  provider.read = liblmbedtls_read;
  provider.write = liblmbedtls_write;
  provider.close = liblmbedtls_close;

  return 0;
}

int liblmbedtls_init(int pe, script_ref_t obj) {
  debug(DEBUG_INFO, TAG, "registering provider %s", SECURE_PROVIDER);
  script_set_pointer(pe, SECURE_PROVIDER, &provider);
  script_add_function(pe, obj, "verify", liblmbedtls_verify);

  return 0;
}
