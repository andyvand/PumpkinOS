// Network initialization for the PlayStation Portable build.
//
// The pspsdk newlib glue implements the BSD socket API (socket/connect/
// select/...) on top of sceNetInet, but the network stack itself has to be
// brought up by the program: the network PRX modules must be loaded, the
// library initialized and the console joined to an access point (one of the
// infrastructure connections configured in the system settings). Done once
// at startup; the connection attempt is bounded by a timeout so that a PSP
// without Wi-Fi (or with the switch off) still boots quickly.
//
// The glue also lacks getaddrinfo(): a small IPv4 version on top of its
// gethostbyname() (sceNetResolver) lives here as well (see pspnetdb.h).

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <pspkerneltypes.h>
#include <pspthreadman.h>
#include <psputility.h>
#include <pspnet.h>
#include <pspnet_inet.h>
#include <pspnet_apctl.h>
#include <pspnet_resolver.h>

#include "sys.h"
#include "debug.h"
#include "pspnetdb.h"

// the first infrastructure connection configured in the system settings
#define PSP_NET_CONNECTION 1

static int modules_loaded = 0;
static int net_ready = 0;
static int ap_connected = 0;

static const char *apctl_state_name(int state) {
  switch (state) {
    case PSP_NET_APCTL_STATE_DISCONNECTED: return "disconnected";
    case PSP_NET_APCTL_STATE_SCANNING:     return "scanning";
    case PSP_NET_APCTL_STATE_JOINING:      return "joining";
    case PSP_NET_APCTL_STATE_GETTING_IP:   return "getting IP address";
    case PSP_NET_APCTL_STATE_GOT_IP:       return "connected";
    case PSP_NET_APCTL_STATE_EAP_AUTH:     return "EAP authentication";
    case PSP_NET_APCTL_STATE_KEY_EXCHANGE: return "key exchange";
  }
  return "unknown";
}

int sys_psp_net_init(int timeout_ms) {
  union SceNetApctlInfo info;
  int r, state, last, started, i;

  if (net_ready) return 0;

  if ((r = sceUtilityLoadNetModule(PSP_NET_MODULE_COMMON)) < 0) {
    debug(DEBUG_ERROR, "SYS", "loading the common network module failed (0x%08X), network unavailable", (unsigned int)r);
    return -1;
  }
  if ((r = sceUtilityLoadNetModule(PSP_NET_MODULE_INET)) < 0) {
    debug(DEBUG_ERROR, "SYS", "loading the inet network module failed (0x%08X), network unavailable", (unsigned int)r);
    sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
    return -1;
  }
  modules_loaded = 1;

  // 128KB network memory pool, the usual callout / interrupt thread settings
  if ((r = sceNetInit(128*1024, 42, 4*1024, 42, 4*1024)) < 0) {
    debug(DEBUG_ERROR, "SYS", "sceNetInit failed (0x%08X), network unavailable", (unsigned int)r);
    return -1;
  }
  if ((r = sceNetInetInit()) < 0) {
    debug(DEBUG_ERROR, "SYS", "sceNetInetInit failed (0x%08X), network unavailable", (unsigned int)r);
    sceNetTerm();
    return -1;
  }
  if ((r = sceNetApctlInit(0x8000, 48)) < 0) {
    debug(DEBUG_ERROR, "SYS", "sceNetApctlInit failed (0x%08X), network unavailable", (unsigned int)r);
    sceNetInetTerm();
    sceNetTerm();
    return -1;
  }
  if ((r = sceNetResolverInit()) < 0) {
    debug(DEBUG_ERROR, "SYS", "sceNetResolverInit failed (0x%08X), host names will not resolve", (unsigned int)r);
  }
  net_ready = 1;

  if (sceUtilityCheckNetParam(PSP_NET_CONNECTION) != 0) {
    debug(DEBUG_INFO, "SYS", "network initialized, no connection configured in the system settings");
    return 0;
  }

  if ((r = sceNetApctlConnect(PSP_NET_CONNECTION)) < 0) {
    debug(DEBUG_ERROR, "SYS", "connecting to the access point failed (0x%08X)", (unsigned int)r);
    return 0;
  }

  // wait for the connection to come up (or to fall back to disconnected)
  last = -1;
  started = 0;
  state = PSP_NET_APCTL_STATE_DISCONNECTED;
  for (i = 0; i < timeout_ms; i += 100) {
    if (sceNetApctlGetState(&state) < 0) break;
    if (state != last) {
      debug(DEBUG_INFO, "SYS", "access point: %s", apctl_state_name(state));
      last = state;
    }
    if (state == PSP_NET_APCTL_STATE_GOT_IP) break;
    if (state != PSP_NET_APCTL_STATE_DISCONNECTED) {
      started = 1;
    } else if (started) {
      // the attempt was given up by the system
      break;
    }
    sceKernelDelayThread(100*1000);
  }

  if (state == PSP_NET_APCTL_STATE_GOT_IP) {
    ap_connected = 1;
    sys_memset(&info, 0, sizeof(info));
    if (sceNetApctlGetInfo(PSP_NET_APCTL_INFO_IP, &info) == 0) {
      debug(DEBUG_INFO, "SYS", "network initialized, IP address %s", info.ip);
    } else {
      debug(DEBUG_INFO, "SYS", "network initialized, connected to the access point");
    }
  } else {
    debug(DEBUG_INFO, "SYS", "network initialized, no access point connection (%s)", apctl_state_name(state));
  }

  return 0;
}

int sys_psp_net_ready(void) {
  return net_ready && ap_connected;
}

void sys_psp_net_finish(void) {
  if (net_ready) {
    if (ap_connected) {
      sceNetApctlDisconnect();
      ap_connected = 0;
    }
    sceNetResolverTerm();
    sceNetApctlTerm();
    sceNetInetTerm();
    sceNetTerm();
    net_ready = 0;
  }
  if (modules_loaded) {
    sceUtilityUnloadNetModule(PSP_NET_MODULE_INET);
    sceUtilityUnloadNetModule(PSP_NET_MODULE_COMMON);
    modules_loaded = 0;
  }
}

// dotted quad parser (the glue has no inet_aton / inet_addr either)
static int parse_ipv4(const char *s, uint32_t *addr) {
  uint32_t part[4];
  int i, n;

  for (i = 0; i < 4; i++) {
    if (*s < '0' || *s > '9') return 0;
    for (part[i] = 0, n = 0; *s >= '0' && *s <= '9' && n < 3; s++, n++) {
      part[i] = part[i] * 10 + (*s - '0');
    }
    if (part[i] > 255) return 0;
    if (i < 3) {
      if (*s != '.') return 0;
      s++;
    }
  }
  if (*s) return 0;

  *addr = htonl((part[0] << 24) | (part[1] << 16) | (part[2] << 8) | part[3]);
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
