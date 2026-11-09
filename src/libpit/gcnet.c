// Network glue for the Nintendo GameCube build (libogc + libbba).
//
// libogc implements the BSD socket API (socket/connect/recv/send/poll/
// fcntl/getsockopt...) on top of its own lwIP stack driving the Broadband
// Adapter (the expansion bay ethernet adapter, "libbba"); the sockets are
// newlib file descriptors, so read/write/close work on them too. The stack
// has to be brought up by the program: if_config() initializes the adapter,
// lwIP and (with DHCP) the address; done once at startup, bounded so that a
// console without an adapter or a cable still boots quickly (offline).
//
// Three POSIX functions the generic code uses are missing from libogc and
// supplied here: select() (on top of libogc's poll(), which only knows
// sockets; the callers only ever select on sockets), getaddrinfo() /
// freeaddrinfo() (an IPv4 version on top of gethostbyname(): the header
// declares them but the library does not define them) and gai_strerror().

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>

#include <network.h>

#include "sys.h"
#include "debug.h"

static int net_ready = 0;

// the name server: the gateway unless told otherwise (sys_gc_net_set_dns)
static char dns_server[16] = "";
static char dns_option[16] = "";

void sys_gc_net_set_dns(const char *ip) {
  strncpy(dns_option, ip, sizeof(dns_option) - 1);
}

int sys_gc_net_init(int timeout_ms) {
  char ip[16], netmask[16], gateway[16];
  int retries, r;

  if (net_ready) return 0;

  // if_config() retries the DHCP exchange this many times (a few seconds
  // each); a console without an adapter fails at once
  retries = timeout_ms > 0 ? (timeout_ms + 4999) / 5000 : 1;
  if (retries < 1) retries = 1;

  ip[0] = netmask[0] = gateway[0] = 0;
  if ((r = if_config(ip, netmask, gateway, true, retries)) < 0) {
    debug(DEBUG_ERROR, "SYS", "network initialization failed (%d): no Broadband Adapter, no link or no DHCP answer; network unavailable", r);
    return -1;
  }

  strncpy(dns_server, dns_option[0] ? dns_option : gateway, sizeof(dns_server) - 1);
  debug(DEBUG_INFO, "SYS", "network up: address %s netmask %s gateway %s, name server %s", ip, netmask, gateway, dns_server);
  net_ready = 1;

  return 0;
}

int sys_gc_net_ready(void) {
  return net_ready;
}

void sys_gc_net_finish(void) {
  // (the GameCube stack has nothing to tear down)
  net_ready = 0;
}

// --- what libogc's BSD layer expects from the stack but the GameCube
// --- stack (libbba) does not provide

s32 net_getsockname(s32 s, struct sockaddr *addr, socklen_t *addrlen) {
  errno = ENOSYS;
  return -1;
}

// A minimal DNS client (A records over UDP, see RFC 1035) for the
// gethostbyname() libogc's wrapper forwards here: the lwIP in libbba has no
// resolver. The server is the gateway (home routers forward DNS) unless
// -dns says otherwise. The result is kept in static storage like the real
// one.
#define DNS_PORT       53
#define DNS_TIMEOUT_MS 3000
#define DNS_RETRIES    2
#define DNS_MAX_ADDRS  8

static struct hostent dns_hostent;
static char dns_name[256];
static uint32_t dns_addrs[DNS_MAX_ADDRS];
static char *dns_addr_list[DNS_MAX_ADDRS + 1];
static char *dns_aliases[1];
static uint16_t dns_id = 0x5041;

// skips a (possibly compressed) name; returns the next offset or -1
static int dns_skip_name(const uint8_t *msg, int len, int off) {
  int n;

  while (off < len) {
    n = msg[off];
    if (n == 0) return off + 1;
    if ((n & 0xC0) == 0xC0) return off + 2 <= len ? off + 2 : -1;
    off += n + 1;
  }

  return -1;
}

static int dns_query(const char *name, uint32_t *addrs, int max) {
  uint8_t msg[512];
  struct sockaddr_in sin;
  struct timeval tv;
  fd_set fds;
  socklen_t slen;
  const char *p, *q;
  int s, len, n, off, i, qd, an, type, rdlen, count = -1, attempt;
  uint16_t id;

  // the query
  id = dns_id++;
  memset(msg, 0, 12);
  msg[0] = id >> 8;
  msg[1] = id & 0xFF;
  msg[2] = 0x01;  // recursion desired
  msg[5] = 1;     // one question
  len = 12;
  for (p = name; *p;) {
    for (q = p; *q && *q != '.'; q++);
    n = q - p;
    if (n == 0 || n > 63 || len + n + 1 >= (int)sizeof(msg) - 5) return -1;
    msg[len++] = n;
    memcpy(&msg[len], p, n);
    len += n;
    p = *q ? q + 1 : q;
  }
  msg[len++] = 0;
  msg[len++] = 0; msg[len++] = 1;  // type A
  msg[len++] = 0; msg[len++] = 1;  // class IN

  memset(&sin, 0, sizeof(sin));
  sin.sin_len = sizeof(sin);
  sin.sin_family = AF_INET;
  sin.sin_port = htons(DNS_PORT);
  sin.sin_addr.s_addr = inet_addr(dns_server);
  if (sin.sin_addr.s_addr == INADDR_NONE || sin.sin_addr.s_addr == 0) return -1;

  if ((s = net_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) return -1;

  for (attempt = 0; attempt < DNS_RETRIES && count < 0; attempt++) {
    if (net_sendto(s, msg, len, 0, (struct sockaddr *)&sin, sizeof(sin)) != len) break;

    FD_ZERO(&fds);
    FD_SET(s, &fds);
    tv.tv_sec = DNS_TIMEOUT_MS / 1000;
    tv.tv_usec = (DNS_TIMEOUT_MS % 1000) * 1000;
    if (net_select(s + 1, &fds, NULL, NULL, &tv) <= 0) continue;

    slen = sizeof(sin);
    if ((n = net_recvfrom(s, msg, sizeof(msg), 0, (struct sockaddr *)&sin, &slen)) < 12) continue;
    if (msg[0] != (id >> 8) || msg[1] != (id & 0xFF)) continue;
    if ((msg[3] & 0x0F) != 0) {
      // an error from the server (3 = no such name)
      count = 0;
      break;
    }
    qd = (msg[4] << 8) | msg[5];
    an = (msg[6] << 8) | msg[7];
    off = 12;
    for (i = 0; i < qd && off >= 0; i++) {
      if ((off = dns_skip_name(msg, n, off)) >= 0) off += 4;
    }
    count = 0;
    for (i = 0; i < an && off >= 0 && off < n; i++) {
      if ((off = dns_skip_name(msg, n, off)) < 0 || off + 10 > n) break;
      type = (msg[off] << 8) | msg[off+1];
      rdlen = (msg[off+8] << 8) | msg[off+9];
      off += 10;
      if (off + rdlen > n) break;
      if (type == 1 && rdlen == 4 && count < max) {
        memcpy(&addrs[count++], &msg[off], 4);
      }
      off += rdlen;
    }
  }
  net_close(s);

  return count;
}

struct hostent *net_gethostbyname(const char *name) {
  uint32_t addr;
  int n, i;

  if (name == NULL || name[0] == 0) return NULL;

  if ((addr = inet_addr(name)) != INADDR_NONE) {
    dns_addrs[0] = addr;
    n = 1;
  } else {
    if (!net_ready) return NULL;
    if ((n = dns_query(name, dns_addrs, DNS_MAX_ADDRS)) <= 0) {
      debug(DEBUG_ERROR, "SYS", "DNS lookup of \"%s\" failed (server %s)", name, dns_server);
      return NULL;
    }
  }

  strncpy(dns_name, name, sizeof(dns_name) - 1);
  dns_name[sizeof(dns_name) - 1] = 0;
  for (i = 0; i < n; i++) {
    dns_addr_list[i] = (char *)&dns_addrs[i];
  }
  dns_addr_list[n] = NULL;
  dns_aliases[0] = NULL;

  dns_hostent.h_name = dns_name;
  dns_hostent.h_aliases = dns_aliases;
  dns_hostent.h_addrtype = AF_INET;
  dns_hostent.h_length = 4;
  dns_hostent.h_addr_list = dns_addr_list;

  return &dns_hostent;
}

// select() on top of poll(): only sockets are ever passed here (see
// vita_is_socket() in sys.c), which is all libogc's poll() handles
int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) {
  struct pollfd pfd[FD_SETSIZE];
  int fd, n, i, r, timeout_ms;

  for (fd = 0, n = 0; fd < nfds && fd < FD_SETSIZE; fd++) {
    pfd[n].events = 0;
    if (readfds && FD_ISSET(fd, readfds)) pfd[n].events |= POLLIN;
    if (writefds && FD_ISSET(fd, writefds)) pfd[n].events |= POLLOUT;
    if (exceptfds && FD_ISSET(fd, exceptfds)) pfd[n].events |= POLLPRI;
    if (pfd[n].events) {
      pfd[n].fd = fd;
      pfd[n].revents = 0;
      n++;
    }
  }

  if (timeout == NULL) {
    timeout_ms = -1;
  } else {
    timeout_ms = timeout->tv_sec * 1000 + (timeout->tv_usec + 999) / 1000;
  }

  if (n == 0) {
    // nothing to watch: just wait
    if (timeout_ms > 0) usleep(timeout_ms * 1000);
    return 0;
  }

  if ((r = poll(pfd, n, timeout_ms)) < 0) {
    return -1;
  }

  if (readfds) FD_ZERO(readfds);
  if (writefds) FD_ZERO(writefds);
  if (exceptfds) FD_ZERO(exceptfds);

  for (i = 0, r = 0; i < n; i++) {
    if (readfds && (pfd[i].revents & (POLLIN | POLLHUP | POLLERR))) {
      FD_SET(pfd[i].fd, readfds);
      r++;
    }
    if (writefds && (pfd[i].revents & (POLLOUT | POLLERR))) {
      FD_SET(pfd[i].fd, writefds);
      r++;
    }
    if (exceptfds && (pfd[i].revents & (POLLPRI | POLLERR))) {
      FD_SET(pfd[i].fd, exceptfds);
      r++;
    }
  }

  return r;
}

// "a.b.c.d" -> network order address, 1 on success
static int parse_ipv4(const char *s, uint32_t *addr) {
  uint32_t a = 0, part = 0;
  int digits = 0, parts = 0;

  for (;; s++) {
    if (*s >= '0' && *s <= '9') {
      part = part * 10 + (*s - '0');
      if (part > 255) return 0;
      digits++;
    } else if (*s == '.' || *s == 0) {
      if (digits == 0) return 0;
      a = (a << 8) | part;
      parts++;
      part = 0;
      digits = 0;
      if (*s == 0) break;
      if (parts == 4) return 0;
    } else {
      return 0;
    }
  }
  if (parts != 4) return 0;
  *addr = htonl(a);

  return 1;
}

static struct addrinfo *new_addrinfo(const struct addrinfo *hints, uint32_t addr, int port) {
  struct addrinfo *ai;
  struct sockaddr_in *sin;

  if ((ai = calloc(1, sizeof(struct addrinfo))) == NULL) return NULL;
  if ((sin = calloc(1, sizeof(struct sockaddr_in))) == NULL) {
    free(ai);
    return NULL;
  }

  sin->sin_len = sizeof(struct sockaddr_in);
  sin->sin_family = AF_INET;
  sin->sin_port = htons(port);
  sin->sin_addr.s_addr = addr;

  ai->ai_family = AF_INET;
  ai->ai_socktype = hints ? hints->ai_socktype : SOCK_STREAM;
  ai->ai_protocol = hints ? hints->ai_protocol : 0;
  ai->ai_addrlen = sizeof(struct sockaddr_in);
  ai->ai_addr = (struct sockaddr *)sin;

  return ai;
}

int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) {
  struct addrinfo *head = NULL, *tail = NULL, *ai;
  struct hostent *he;
  uint32_t addr;
  int port, i;

  if (res == NULL) return EAI_FAIL;
  *res = NULL;

  if (hints && hints->ai_family != AF_UNSPEC && hints->ai_family != AF_INET) return EAI_FAMILY;
  port = service ? atoi(service) : 0;

  if (node == NULL) {
    // the wildcard / loopback address
    addr = (hints && (hints->ai_flags & AI_PASSIVE)) ? htonl(INADDR_ANY) : htonl(INADDR_LOOPBACK);
    if ((head = new_addrinfo(hints, addr, port)) == NULL) return EAI_MEMORY;
    *res = head;
    return 0;
  }

  if (parse_ipv4(node, &addr)) {
    if ((head = new_addrinfo(hints, addr, port)) == NULL) return EAI_MEMORY;
    *res = head;
    return 0;
  }

  if (hints && (hints->ai_flags & AI_NUMERICHOST)) return EAI_NONAME;

  if (!net_ready) return EAI_FAIL;
  if ((he = gethostbyname(node)) == NULL || he->h_addrtype != AF_INET || he->h_addr_list == NULL) return EAI_NONAME;

  for (i = 0; he->h_addr_list[i]; i++) {
    memcpy(&addr, he->h_addr_list[i], sizeof(addr));
    if (addr == htonl(INADDR_ANY)) continue;
    if ((ai = new_addrinfo(hints, addr, port)) == NULL) {
      freeaddrinfo(head);
      return EAI_MEMORY;
    }
    if (tail) tail->ai_next = ai; else head = ai;
    tail = ai;
  }
  if (head == NULL) return EAI_NONAME;

  *res = head;
  return 0;
}

void freeaddrinfo(struct addrinfo *res) {
  struct addrinfo *next;

  for (; res; res = next) {
    next = res->ai_next;
    free(res->ai_addr);
    free(res);
  }
}

const char *gai_strerror(int errcode) {
  switch (errcode) {
    case EAI_AGAIN:    return "temporary failure in name resolution";
    case EAI_FAIL:     return "non-recoverable failure in name resolution";
    case EAI_FAMILY:   return "address family not supported";
    case EAI_MEMORY:   return "memory allocation failure";
    case EAI_NONAME:   return "name or service not known";
    case EAI_SOCKTYPE: return "socket type not supported";
  }
  return "unknown error";
}
