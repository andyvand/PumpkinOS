// Network glue for the Sony PlayStation 3 build (PSL1GHT libnet).
//
// sys_ps3_net_init() loads the network system modules, initializes libnet
// and waits (a bounded time) for the console's connection, the one set up
// in the XMB network settings, to obtain an address. libnet speaks IPv4
// only and has gethostbyname() (the LV2 resolver) but no getaddrinfo(),
// freeaddrinfo() or gai_strerror(): this file supplies IPv4 versions of
// them for libpit's sys.c.

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <net/net.h>
#include <net/netctl.h>
#include <net/netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sysmodule/sysmodule.h>

#include "sys.h"
#include "debug.h"

#ifndef EAI_NONAME
#define EAI_AGAIN    -3
#define EAI_FAIL     -4
#define EAI_FAMILY   -6
#define EAI_MEMORY   -10
#define EAI_NONAME   -2
#define EAI_SOCKTYPE -7
#endif

int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
void freeaddrinfo(struct addrinfo *res);
const char *gai_strerror(int errcode);

static int net_loaded = 0;   // libnet initialized (sockets usable)
static int net_ready = 0;    // the connection has an address
static int netctl_up = 0;

int sys_ps3_net_init(int timeout_ms) {
  union net_ctl_info info;
  uint64_t t0;
  int32_t state;
  int r;

  if (net_ready) return 0;

  if (!net_loaded) {
    sysModuleLoad(SYSMODULE_NET);
    sysModuleLoad(SYSMODULE_NETCTL);
    if ((r = netInitialize()) < 0) {
      debug(DEBUG_ERROR, "SYS", "netInitialize failed (0x%08X): network unavailable", r);
      return -1;
    }
    net_loaded = 1;
    if ((r = netCtlInit()) < 0) {
      debug(DEBUG_ERROR, "SYS", "netCtlInit failed (0x%08X)", r);
    } else {
      netctl_up = 1;
    }
  }

  if (!netctl_up) {
    // no connection manager: try the sockets anyway
    net_ready = 1;
    return 0;
  }

  // the console connects by itself (XMB settings): wait for an address
  t0 = sys_get_clock();
  for (;;) {
    state = NET_CTL_STATE_Disconnected;
    if (netCtlGetState(&state) < 0) state = NET_CTL_STATE_Disconnected;
    if (state == NET_CTL_STATE_IPObtained) break;
    // no cable / no access point configured: the state stays Disconnected
    if ((sys_get_clock() - t0) / 1000 >= (uint64_t)timeout_ms ||
        (state == NET_CTL_STATE_Disconnected && (sys_get_clock() - t0) / 1000 >= 3000)) {
      debug(DEBUG_ERROR, "SYS", "no network connection after %d ms (state %d): check the network settings; network unavailable", timeout_ms, (int)state);
      return -1;
    }
    usleep(100000);
  }

  memset(&info, 0, sizeof(info));
  if (netCtlGetInfo(NET_CTL_INFO_IP_ADDRESS, &info) >= 0) {
    debug(DEBUG_INFO, "SYS", "network up: address %s", info.ip_address);
  } else {
    debug(DEBUG_INFO, "SYS", "network up");
  }
  net_ready = 1;

  return 0;
}

int sys_ps3_net_ready(void) {
  return net_ready;
}

void sys_ps3_net_finish(void) {
  if (netctl_up) netCtlTerm();
  if (net_loaded) {
    netDeinitialize();
    sysModuleUnload(SYSMODULE_NETCTL);
    sysModuleUnload(SYSMODULE_NET);
  }
  netctl_up = net_loaded = net_ready = 0;
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

  if (!net_loaded) return EAI_FAIL;
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
