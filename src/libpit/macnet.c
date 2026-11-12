// BSD sockets for classic Mac OS (Retro68) on top of MacTCP.
//
// MacTCP is a driver (".IPP") driven by PBControl calls with mt_TCPiopb /
// mt_UDPiopb parameter blocks; Open Transport (Mac OS 7.6 and later, and the
// "Open Transport" installs of 7.5) keeps that interface for compatibility,
// so this works with both. Every call is made asynchronously without a
// completion routine and its parameter block polled until ioResult is no
// longer MT_inProgress: the PumpkinOS threads are coroutines, they yield to
// each other (and to the other applications) while a call is pending.
//
// A TCP socket is a MacTCP stream (MT_TCPCreate) with its own receive buffer.
// connect() is MT_TCPActiveOpen; listen() puts a stream in MT_TCPPassiveOpen, and
// when a connection arrives accept() hands that stream to a new descriptor
// and starts a fresh passive open for the next one (MacTCP streams accept
// one connection each). A UDP socket is a UDP stream with one read always
// outstanding.
//
// Host names are resolved by the MacTCP domain name resolver: the 'dnrp'
// code resource of the MacTCP control panel (or of Open Transport's TCP/IP
// control panel, or the System file), called like Apple's dnr.c glue does.
//
// IPv4 only. Sockets and files share the descriptor table of macsys.c.

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>

#include <MacTypes.h>
#include <Devices.h>
#include <Files.h>
#include <Resources.h>
#include <MacMemory.h>
#include <Events.h>

#include "sys.h"
#include "macsys.h"
#include "thread.h"
#include "debug.h"

#undef open
#undef close
#undef read
#undef write
#undef select
#undef fcntl

#define TAG "NET"

// --- MacTCP interface (Universal Interfaces MacTCP.h, 68K alignment) ---

#pragma pack(push, 2)

typedef uint32_t mt_ip_addr;
typedef uint32_t mt_StreamPtr;

typedef struct {
  uint16_t length;
  Ptr ptr;
} mt_wdsEntry;

typedef struct {
  Ptr rcvBuff;
  uint32_t rcvBuffLen;
  void *notifyProc;
  Ptr userDataPtr;
} mt_TCPCreatePB;

typedef struct {
  int8_t ulpTimeoutValue;
  int8_t ulpTimeoutAction;
  int8_t validityFlags;
  int8_t commandTimeoutValue;
  mt_ip_addr remoteHost;
  uint16_t remotePort;
  mt_ip_addr localHost;
  uint16_t localPort;
  int8_t tosFlags;
  int8_t precedence;
  Boolean dontFrag;
  int8_t timeToLive;
  int8_t security;
  int8_t optionCnt;
  int8_t options[40];
  Ptr userDataPtr;
} mt_TCPOpenPB;

typedef struct {
  int8_t ulpTimeoutValue;
  int8_t ulpTimeoutAction;
  int8_t validityFlags;
  Boolean pushFlag;
  Boolean urgentFlag;
  int8_t filler;
  Ptr wdsPtr;
  uint32_t sendFree;
  uint16_t sendLength;
  Ptr userDataPtr;
} mt_TCPSendPB;

typedef struct {
  int8_t commandTimeoutValue;
  Boolean markFlag;
  Boolean urgentFlag;
  int8_t filler;
  Ptr rcvBuff;
  uint16_t rcvBuffLen;
  Ptr rdsPtr;
  uint16_t rdsLength;
  uint16_t secondTimeStamp;
  Ptr userDataPtr;
} mt_TCPReceivePB;

typedef struct {
  int8_t ulpTimeoutValue;
  int8_t ulpTimeoutAction;
  int8_t validityFlags;
  int8_t filler;
  Ptr userDataPtr;
} mt_TCPClosePB;

typedef struct {
  int8_t ulpTimeoutValue;
  int8_t ulpTimeoutAction;
  int32_t unused;
  mt_ip_addr remoteHost;
  uint16_t remotePort;
  mt_ip_addr localHost;
  uint16_t localPort;
  int8_t tosFlags;
  int8_t precedence;
  int8_t connectionState;
  int8_t filler;
  uint16_t sendWindow;
  uint16_t rcvWindow;
  uint16_t amtUnackedData;
  uint16_t amtUnreadData;
  Ptr securityLevelPtr;
  uint32_t sendUnacked;
  uint32_t sendNext;
  uint32_t congestionWindow;
  uint32_t rcvNext;
  uint32_t srtt;
  uint32_t lastRTT;
  uint32_t sendMaxSegSize;
  void *connStatPtr;
  Ptr userDataPtr;
} mt_TCPStatusPB;

typedef struct {
  int8_t fill12[12];
  void *ioCompletion;
  volatile int16_t ioResult;
  Ptr ioNamePtr;
  int16_t ioVRefNum;
  int16_t ioCRefNum;
  int16_t csCode;
  mt_StreamPtr tcpStream;
  union {
    mt_TCPCreatePB create;
    mt_TCPOpenPB open;
    mt_TCPSendPB send;
    mt_TCPReceivePB receive;
    mt_TCPClosePB close;
    mt_TCPStatusPB status;
  } csParam;
} mt_TCPiopb;

typedef struct {
  Ptr rcvBuff;
  uint32_t rcvBuffLen;
  void *notifyProc;
  uint16_t localPort;
  Ptr userDataPtr;
  uint16_t endingPort;
} mt_UDPCreatePB;

typedef struct {
  uint16_t reserved;
  mt_ip_addr remoteHost;
  uint16_t remotePort;
  Ptr wdsPtr;
  Boolean checkSum;
  int8_t filler;
  uint16_t sendLength;
  Ptr userDataPtr;
  uint16_t localPort;
} mt_UDPSendPB;

typedef struct {
  uint16_t timeOut;
  mt_ip_addr remoteHost;
  uint16_t remotePort;
  Ptr rcvBuff;
  uint16_t rcvBuffLen;
  uint16_t secondTimeStamp;
  Ptr userDataPtr;
  mt_ip_addr destHost;
  uint16_t destPort;
} mt_UDPReceivePB;

typedef struct {
  int8_t fill12[12];
  void *ioCompletion;
  volatile int16_t ioResult;
  Ptr ioNamePtr;
  int16_t ioVRefNum;
  int16_t ioCRefNum;
  int16_t csCode;
  mt_StreamPtr udpStream;
  union {
    mt_UDPCreatePB create;
    mt_UDPSendPB send;
    mt_UDPReceivePB receive;
  } csParam;
} mt_UDPiopb;

typedef struct {
  int8_t fill12[12];
  void *ioCompletion;
  volatile int16_t ioResult;
  Ptr ioNamePtr;
  int16_t ioVRefNum;
  int16_t ioCRefNum;
  int16_t csCode;
  mt_ip_addr ourAddress;
  int32_t ourNetMask;
} mt_GetAddrParamBlock;

// the resolver's answer (AddressXlation.h)
#define NUM_ALT_ADDRS 4
struct mt_hostInfo {
  int32_t rtnCode;
  char cname[255];
  int8_t filler;
  uint32_t addr[NUM_ALT_ADDRS];
};

#pragma pack(pop)

enum {
  MT_inProgress = 1,
  MT_connectionClosing = -23005,
  MT_connectionDoesntExist = -23008,
  MT_connectionTerminated = -23012,
  MT_commandTimeout = -23016,
  MT_duplicateSocket = -23017,
  MT_openFailed = -23015,
  MT_cacheFault = -23042,
  MT_noNameServer = -23044,
  MT_authNameErr = -23045,
  MT_noAnsErr = -23046,
  MT_dnrErr = -23047,
  MT_outOfMemory = -23048,
  MT_nameSyntaxErr = -23041
};

enum {
  MT_ipctlGetAddr = 15,
  MT_UDPCreate = 20,
  MT_UDPRead = 21,
  MT_UDPBfrReturn = 22,
  MT_UDPWrite = 23,
  MT_UDPRelease = 24,
  MT_TCPCreate = 30,
  MT_TCPPassiveOpen = 31,
  MT_TCPActiveOpen = 32,
  MT_TCPSend = 34,
  MT_TCPRcv = 37,
  MT_TCPClose = 38,
  MT_TCPAbort = 39,
  MT_TCPStatus = 40,
  MT_TCPRelease = 42
};

// validityFlags
#define MT_timeoutValue  0x80
#define MT_timeoutAction 0x40

// connectionState (MT_TCPStatus)
#define STATE_CLOSED      0
#define STATE_LISTEN      2
#define STATE_SYN_RCVD    4
#define STATE_SYN_SENT    6
#define STATE_ESTABLISHED 8
#define STATE_FIN_WAIT1   10
#define STATE_FIN_WAIT2   12
#define STATE_CLOSE_WAIT  14

// resolver selectors (dnr.c)
#define OPENRESOLVER  1
#define CLOSERESOLVER 2
#define STRTOADDR     3

// --- sockets ---

#define TCP_RCVBUF  (16 * 1024)
#define UDP_RCVBUF  (8 * 1024)
#define SEND_CHUNK  8192
#define CLOSE_WAIT_US 3000000

typedef enum {
  S_NEW,          // created, nothing on the network yet
  S_CONNECTING,   // MT_TCPActiveOpen pending
  S_CONNECTED,
  S_LISTENING,    // MT_TCPPassiveOpen pending
  S_CLOSED,       // the connection is gone (error or EOF)
} sock_state_t;

typedef struct {
  int type;
  int nonblock;
  sock_state_t state;
  int error;          // SO_ERROR
  int wr_shut;
  mt_StreamPtr stream;
  Ptr rcvbuf;
  mt_ip_addr lhost, rhost;
  uint16_t lport, rport;
  mt_TCPiopb openpb;     // the pending active / passive open
  mt_TCPiopb closepb;    // half close (shutdown)
  // UDP: the outstanding read
  mt_UDPiopb readpb;
  int read_pending;
} mac_sock_t;

static mac_sock_t *socks[FD_SETSIZE];
static int16_t ipp_ref;
static int net_ready = -1;
static mt_ip_addr our_addr;

extern int mac_fd_alloc_socket(void);
extern void mac_fd_release(int fd);

const struct in6_addr in6addr_any = {{ 0 }};

static void wait_pb(volatile int16_t *result) {
  while (*result == MT_inProgress) {
    thread_yield(0);
    mac_yield(0);
  }
}

// the part every MacTCP parameter block starts with
#pragma pack(push, 2)
typedef struct {
  int8_t fill12[12];
  void *ioCompletion;
  volatile int16_t ioResult;
  Ptr ioNamePtr;
  int16_t ioVRefNum;
  int16_t ioCRefNum;
  int16_t csCode;
} mt_iohdr;
#pragma pack(pop)

static OSErr control(void *pb, int16_t cscode, int async) {
  mt_iohdr *p = (mt_iohdr *)pb;
  OSErr err;

  p->ioCompletion = NULL;
  p->ioCRefNum = ipp_ref;
  p->csCode = cscode;
  err = PBControlAsync((ParmBlkPtr)pb);
  if (err != noErr) return err;
  if (!async) {
    wait_pb(&p->ioResult);
    err = p->ioResult;
  }

  return err;
}

int mac_net_init(void) {
  mt_GetAddrParamBlock pb;
  OSErr err;

  if (net_ready != -1) return net_ready ? 0 : -1;
  net_ready = 0;

  if ((err = OpenDriver("\p.IPP", &ipp_ref)) != noErr) {
    debug(DEBUG_ERROR, TAG, "MacTCP is not available (OpenDriver .IPP: %d)", err);
    return -1;
  }

  memset(&pb, 0, sizeof(pb));
  pb.ioCRefNum = ipp_ref;
  pb.csCode = MT_ipctlGetAddr;
  if ((err = PBControlSync((ParmBlkPtr)&pb)) != noErr) {
    debug(DEBUG_ERROR, TAG, "MacTCP: no address (%d)", err);
    return -1;
  }
  our_addr = pb.ourAddress;
  net_ready = 1;
  debug(DEBUG_INFO, TAG, "MacTCP ready, address %d.%d.%d.%d", (int)(our_addr >> 24), (int)((our_addr >> 16) & 0xFF), (int)((our_addr >> 8) & 0xFF), (int)(our_addr & 0xFF));

  return 0;
}

int mac_net_ready(void) {
  return net_ready == 1;
}

static mac_sock_t *get_sock(int fd) {
  if (fd < 0 || fd >= FD_SETSIZE || socks[fd] == NULL) {
    errno = mac_fd_is_socket(fd) ? EBADF : ENOTSOCK;
    return NULL;
  }
  return socks[fd];
}

static int sock_errno(OSErr err) {
  switch (err) {
    case MT_connectionClosing:
    case MT_connectionTerminated:
    case MT_connectionDoesntExist: return ECONNRESET;
    case MT_commandTimeout:        return ETIMEDOUT;
    case MT_openFailed:            return ECONNREFUSED;
    case MT_duplicateSocket:       return EADDRINUSE;
    case memFullErr:
    case MT_outOfMemory:           return ENOMEM;
    default:                    return EIO;
  }
}

int socket(int domain, int type, int protocol) {
  mac_sock_t *s;
  int fd;

  (void)protocol;
  if (domain != AF_INET) {
    errno = EAFNOSUPPORT;
    return -1;
  }
  if (type != SOCK_STREAM && type != SOCK_DGRAM) {
    errno = EPROTONOSUPPORT;
    return -1;
  }
  if (mac_net_init() != 0) {
    errno = ENETDOWN;
    return -1;
  }
  if ((s = calloc(1, sizeof(mac_sock_t))) == NULL) {
    errno = ENOMEM;
    return -1;
  }
  if ((fd = mac_fd_alloc_socket()) == -1) {
    free(s);
    return -1;
  }
  s->type = type;
  s->state = S_NEW;
  socks[fd] = s;

  return fd;
}

int socketpair(int domain, int type, int protocol, int sv[2]) {
  (void)domain; (void)type; (void)protocol; (void)sv;
  errno = EOPNOTSUPP;
  return -1;
}

static int tcp_create(mac_sock_t *s) {
  mt_TCPiopb pb;
  OSErr err;

  if ((s->rcvbuf = NewPtr(TCP_RCVBUF)) == NULL) {
    errno = ENOMEM;
    return -1;
  }
  memset(&pb, 0, sizeof(pb));
  pb.csParam.create.rcvBuff = s->rcvbuf;
  pb.csParam.create.rcvBuffLen = TCP_RCVBUF;
  if ((err = control(&pb, MT_TCPCreate, 0)) != noErr) {
    DisposePtr(s->rcvbuf);
    s->rcvbuf = NULL;
    errno = sock_errno(err);
    return -1;
  }
  s->stream = pb.tcpStream;

  return 0;
}

static void tcp_release(mac_sock_t *s) {
  mt_TCPiopb pb;

  if (s->stream) {
    memset(&pb, 0, sizeof(pb));
    pb.tcpStream = s->stream;
    control(&pb, MT_TCPRelease, 0);
    s->stream = 0;
  }
  if (s->rcvbuf) {
    DisposePtr(s->rcvbuf);
    s->rcvbuf = NULL;
  }
}

static int udp_create(mac_sock_t *s) {
  mt_UDPiopb pb;
  OSErr err;

  if ((s->rcvbuf = NewPtr(UDP_RCVBUF)) == NULL) {
    errno = ENOMEM;
    return -1;
  }
  memset(&pb, 0, sizeof(pb));
  pb.csParam.create.rcvBuff = s->rcvbuf;
  pb.csParam.create.rcvBuffLen = UDP_RCVBUF;
  pb.csParam.create.localPort = s->lport;
  if ((err = control(&pb, MT_UDPCreate, 0)) != noErr) {
    DisposePtr(s->rcvbuf);
    s->rcvbuf = NULL;
    errno = sock_errno(err);
    return -1;
  }
  s->stream = pb.udpStream;
  s->lport = pb.csParam.create.localPort;

  return 0;
}

static void udp_start_read(mac_sock_t *s) {
  memset(&s->readpb, 0, sizeof(mt_UDPiopb));
  s->readpb.udpStream = s->stream;
  s->readpb.csParam.receive.timeOut = 0;   // wait for ever
  if (control(&s->readpb, MT_UDPRead, 1) == noErr) {
    s->read_pending = 1;
  }
}

static void udp_release(mac_sock_t *s) {
  mt_UDPiopb pb;

  if (s->stream) {
    if (s->read_pending && s->readpb.ioResult == noErr) {
      memset(&pb, 0, sizeof(pb));
      pb.udpStream = s->stream;
      pb.csParam.receive.rcvBuff = s->readpb.csParam.receive.rcvBuff;
      control(&pb, MT_UDPBfrReturn, 0);
    }
    // (releasing the stream completes a pending read with an error)
    memset(&pb, 0, sizeof(pb));
    pb.udpStream = s->stream;
    control(&pb, MT_UDPRelease, 0);
    s->stream = 0;
  }
  if (s->rcvbuf) {
    DisposePtr(s->rcvbuf);
    s->rcvbuf = NULL;
  }
}

static int udp_ensure(mac_sock_t *s) {
  if (s->stream == 0) {
    if (udp_create(s) != 0) return -1;
    udp_start_read(s);
  }
  return 0;
}

// is a pending open finished ? updates the state
static void check_open(mac_sock_t *s) {
  if ((s->state == S_CONNECTING || s->state == S_LISTENING) && s->openpb.ioResult != MT_inProgress) {
    if (s->openpb.ioResult == noErr) {
      if (s->state == S_CONNECTING) {
        s->state = S_CONNECTED;
        s->lhost = s->openpb.csParam.open.localHost;
        s->lport = s->openpb.csParam.open.localPort;
      }
      // (a finished passive open is picked up by accept())
    } else {
      s->error = sock_errno(s->openpb.ioResult);
      if (s->state == S_CONNECTING) s->state = S_CLOSED;
    }
  }
}

int connect(int fd, const struct sockaddr *addr, socklen_t len) {
  const struct sockaddr_in *sin = (const struct sockaddr_in *)addr;
  mac_sock_t *s;
  OSErr err;

  if ((s = get_sock(fd)) == NULL) return -1;
  if (addr == NULL || addr->sa_family != AF_INET || len < sizeof(struct sockaddr_in)) {
    errno = EAFNOSUPPORT;
    return -1;
  }

  if (s->type == SOCK_DGRAM) {
    if (udp_ensure(s) != 0) return -1;
    s->rhost = sin->sin_addr.s_addr;
    s->rport = sin->sin_port;
    s->state = S_CONNECTED;
    return 0;
  }

  if (s->state == S_CONNECTING) {
    check_open(s);
    if (s->state == S_CONNECTING) {
      errno = EALREADY;
      return -1;
    }
    if (s->state == S_CONNECTED) {
      errno = EISCONN;
      return -1;
    }
    errno = s->error;
    return -1;
  }
  if (s->state != S_NEW) {
    errno = EISCONN;
    return -1;
  }

  if (s->stream == 0 && tcp_create(s) != 0) return -1;

  s->rhost = sin->sin_addr.s_addr;
  s->rport = sin->sin_port;
  memset(&s->openpb, 0, sizeof(mt_TCPiopb));
  s->openpb.tcpStream = s->stream;
  s->openpb.csParam.open.validityFlags = MT_timeoutValue | MT_timeoutAction;
  s->openpb.csParam.open.ulpTimeoutValue = 60;
  s->openpb.csParam.open.ulpTimeoutAction = 1;   // abort
  s->openpb.csParam.open.commandTimeoutValue = 30;
  s->openpb.csParam.open.remoteHost = s->rhost;
  s->openpb.csParam.open.remotePort = s->rport;
  s->openpb.csParam.open.localPort = s->lport;
  if ((err = control(&s->openpb, MT_TCPActiveOpen, 1)) != noErr) {
    errno = sock_errno(err);
    return -1;
  }
  s->state = S_CONNECTING;

  if (s->nonblock) {
    errno = EINPROGRESS;
    return -1;
  }

  wait_pb(&s->openpb.ioResult);
  check_open(s);
  if (s->state != S_CONNECTED) {
    errno = s->error ? s->error : ECONNREFUSED;
    return -1;
  }

  return 0;
}

int bind(int fd, const struct sockaddr *addr, socklen_t len) {
  const struct sockaddr_in *sin = (const struct sockaddr_in *)addr;
  mac_sock_t *s;

  if ((s = get_sock(fd)) == NULL) return -1;
  if (addr == NULL || addr->sa_family != AF_INET || len < sizeof(struct sockaddr_in)) {
    errno = EAFNOSUPPORT;
    return -1;
  }
  s->lhost = sin->sin_addr.s_addr;
  s->lport = sin->sin_port;

  if (s->type == SOCK_DGRAM && s->stream == 0) {
    return udp_ensure(s);
  }

  return 0;
}

static int start_passive(mac_sock_t *s) {
  OSErr err;

  if (s->stream == 0 && tcp_create(s) != 0) return -1;
  memset(&s->openpb, 0, sizeof(mt_TCPiopb));
  s->openpb.tcpStream = s->stream;
  s->openpb.csParam.open.validityFlags = MT_timeoutValue | MT_timeoutAction;
  s->openpb.csParam.open.ulpTimeoutValue = 60;
  s->openpb.csParam.open.ulpTimeoutAction = 1;
  s->openpb.csParam.open.commandTimeoutValue = 0;  // wait for ever
  s->openpb.csParam.open.localPort = s->lport;
  if ((err = control(&s->openpb, MT_TCPPassiveOpen, 1)) != noErr) {
    errno = sock_errno(err);
    return -1;
  }
  s->state = S_LISTENING;

  return 0;
}

int listen(int fd, int backlog) {
  mac_sock_t *s;

  (void)backlog;
  if ((s = get_sock(fd)) == NULL) return -1;
  if (s->type != SOCK_STREAM) {
    errno = EOPNOTSUPP;
    return -1;
  }
  if (s->state == S_LISTENING) return 0;

  return start_passive(s);
}

int accept(int fd, struct sockaddr *addr, socklen_t *len) {
  struct sockaddr_in *sin = (struct sockaddr_in *)addr;
  mac_sock_t *s, *c;
  int cfd;

  if ((s = get_sock(fd)) == NULL) return -1;
  if (s->state != S_LISTENING) {
    errno = EINVAL;
    return -1;
  }

  for (;;) {
    if (s->openpb.ioResult != MT_inProgress) break;
    if (s->nonblock) {
      errno = EWOULDBLOCK;
      return -1;
    }
    thread_yield(0);
    mac_yield(0);
    if (thread_must_end()) {
      errno = EINTR;
      return -1;
    }
  }

  if (s->openpb.ioResult != noErr) {
    // the passive open failed (timeout, aborted): listen again
    debug(DEBUG_ERROR, TAG, "passive open on port %u failed (%d)", s->lport, s->openpb.ioResult);
    errno = ECONNABORTED;
    start_passive(s);
    return -1;
  }

  // the connection is on the listening stream: give it to a new socket
  if ((cfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) return -1;
  c = socks[cfd];
  c->stream = s->stream;
  c->rcvbuf = s->rcvbuf;
  c->state = S_CONNECTED;
  c->rhost = s->openpb.csParam.open.remoteHost;
  c->rport = s->openpb.csParam.open.remotePort;
  c->lhost = s->openpb.csParam.open.localHost;
  c->lport = s->openpb.csParam.open.localPort;

  if (sin && len && *len >= sizeof(struct sockaddr_in)) {
    memset(sin, 0, sizeof(struct sockaddr_in));
    sin->sin_len = sizeof(struct sockaddr_in);
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = c->rhost;
    sin->sin_port = c->rport;
    *len = sizeof(struct sockaddr_in);
  }

  // a fresh stream for the next connection
  s->stream = 0;
  s->rcvbuf = NULL;
  if (start_passive(s) != 0) {
    s->state = S_CLOSED;
  }

  return cfd;
}

// MT_TCPStatus: unread bytes and the connection state; -1 when the stream is gone
static int tcp_status(mac_sock_t *s, int *unread, int *state) {
  mt_TCPiopb pb;
  OSErr err;

  memset(&pb, 0, sizeof(pb));
  pb.tcpStream = s->stream;
  if ((err = control(&pb, MT_TCPStatus, 0)) != noErr) {
    *unread = 0;
    *state = STATE_CLOSED;
    return -1;
  }
  *unread = pb.csParam.status.amtUnreadData;
  *state = pb.csParam.status.connectionState;

  return 0;
}

static int remote_done(int state) {
  return state != STATE_SYN_SENT && state != STATE_SYN_RCVD && state != STATE_ESTABLISHED &&
         state != STATE_FIN_WAIT1 && state != STATE_FIN_WAIT2;
}

int mac_sock_ready(int fd, int want) {
  mac_sock_t *s;
  int unread, state;

  if ((s = get_sock(fd)) == NULL) return -1;

  if (s->type == SOCK_DGRAM) {
    if (want) return 1;
    if (udp_ensure(s) != 0) return 1;
    if (!s->read_pending) udp_start_read(s);
    return s->read_pending && s->readpb.ioResult != MT_inProgress;
  }

  check_open(s);
  switch (s->state) {
    case S_NEW:
      return 0;
    case S_CONNECTING:
      return 0;
    case S_LISTENING:
      // readable: a connection to accept
      return want ? 0 : (s->openpb.ioResult != MT_inProgress);
    case S_CLOSED:
      return 1;
    case S_CONNECTED:
      if (want) return 1;
      if (tcp_status(s, &unread, &state) != 0) return 1;
      return unread > 0 || remote_done(state);
  }

  return 1;
}

int mac_sock_set_nonblock(int fd, int on) {
  mac_sock_t *s;

  if ((s = get_sock(fd)) == NULL) return -1;
  s->nonblock = on;
  return 0;
}

static ssize_t tcp_recv(mac_sock_t *s, void *buf, size_t len, int flags) {
  mt_TCPiopb pb;
  int unread, state;
  OSErr err;

  if (len == 0) return 0;
  check_open(s);
  if (s->state == S_CONNECTING) {
    if (s->nonblock || (flags & MSG_DONTWAIT)) {
      errno = EWOULDBLOCK;
      return -1;
    }
    wait_pb(&s->openpb.ioResult);
    check_open(s);
  }
  if (s->state == S_CLOSED) return 0;
  if (s->state != S_CONNECTED) {
    errno = ENOTCONN;
    return -1;
  }

  for (;;) {
    if (tcp_status(s, &unread, &state) != 0) return 0;
    if (unread > 0) break;
    if (remote_done(state)) return 0;
    if (s->nonblock || (flags & MSG_DONTWAIT)) {
      errno = EWOULDBLOCK;
      return -1;
    }
    thread_yield(0);
    mac_yield(0);
    if (thread_must_end()) {
      errno = EINTR;
      return -1;
    }
  }

  if (flags & MSG_PEEK) {
    // MacTCP cannot peek: say how much there is, without the data
    errno = EOPNOTSUPP;
    return -1;
  }

  if (len > (size_t)unread) len = unread;
  if (len > 32767) len = 32767;

  memset(&pb, 0, sizeof(pb));
  pb.tcpStream = s->stream;
  pb.csParam.receive.commandTimeoutValue = 1;
  pb.csParam.receive.rcvBuff = buf;
  pb.csParam.receive.rcvBuffLen = (uint16_t)len;
  err = control(&pb, MT_TCPRcv, 0);
  if (err == MT_connectionClosing || err == MT_connectionTerminated || err == MT_connectionDoesntExist) {
    return 0;
  }
  if (err != noErr && err != MT_commandTimeout) {
    errno = sock_errno(err);
    return -1;
  }

  return pb.csParam.receive.rcvBuffLen;
}

static ssize_t tcp_send(mac_sock_t *s, const void *buf, size_t len) {
  mt_TCPiopb pb;
  mt_wdsEntry wds[2];
  size_t sent = 0, n;
  OSErr err;

  check_open(s);
  if (s->state == S_CONNECTING) {
    wait_pb(&s->openpb.ioResult);
    check_open(s);
  }
  if (s->state != S_CONNECTED || s->wr_shut) {
    errno = s->state == S_CLOSED ? EPIPE : ENOTCONN;
    return -1;
  }

  // (the data must not change until MacTCP is done with it: the call is
  // waited for before returning)
  while (sent < len) {
    n = len - sent;
    if (n > SEND_CHUNK) n = SEND_CHUNK;
    wds[0].length = (uint16_t)n;
    wds[0].ptr = (Ptr)((const uint8_t *)buf + sent);
    wds[1].length = 0;
    wds[1].ptr = NULL;
    memset(&pb, 0, sizeof(pb));
    pb.tcpStream = s->stream;
    pb.csParam.send.validityFlags = MT_timeoutValue | MT_timeoutAction;
    pb.csParam.send.ulpTimeoutValue = 60;
    pb.csParam.send.ulpTimeoutAction = 1;
    pb.csParam.send.pushFlag = 1;
    pb.csParam.send.wdsPtr = (Ptr)wds;
    if ((err = control(&pb, MT_TCPSend, 0)) != noErr) {
      if (sent > 0) break;
      s->state = S_CLOSED;
      errno = err == MT_connectionClosing || err == MT_connectionTerminated || err == MT_connectionDoesntExist ? EPIPE : sock_errno(err);
      return -1;
    }
    sent += n;
  }

  return sent;
}

ssize_t recv(int fd, void *buf, size_t len, int flags) {
  mac_sock_t *s;

  if ((s = get_sock(fd)) == NULL) return -1;
  if (s->type == SOCK_DGRAM) return recvfrom(fd, buf, len, flags, NULL, NULL);

  return tcp_recv(s, buf, len, flags);
}

ssize_t send(int fd, const void *buf, size_t len, int flags) {
  mac_sock_t *s;
  struct sockaddr_in sin;

  (void)flags;
  if ((s = get_sock(fd)) == NULL) return -1;
  if (s->type == SOCK_DGRAM) {
    if (s->state != S_CONNECTED) {
      errno = EDESTADDRREQ;
      return -1;
    }
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = s->rhost;
    sin.sin_port = s->rport;
    return sendto(fd, buf, len, flags, (struct sockaddr *)&sin, sizeof(sin));
  }

  return tcp_send(s, buf, len);
}

ssize_t sendto(int fd, const void *buf, size_t len, int flags, const struct sockaddr *to, socklen_t tolen) {
  const struct sockaddr_in *sin = (const struct sockaddr_in *)to;
  mt_UDPiopb pb;
  mt_wdsEntry wds[2];
  mac_sock_t *s;
  OSErr err;

  (void)flags;
  if ((s = get_sock(fd)) == NULL) return -1;
  if (s->type != SOCK_DGRAM) return send(fd, buf, len, flags);
  if (to == NULL || to->sa_family != AF_INET || tolen < sizeof(struct sockaddr_in)) {
    errno = EAFNOSUPPORT;
    return -1;
  }
  if (len > 1472) {
    errno = EMSGSIZE;
    return -1;
  }
  if (udp_ensure(s) != 0) return -1;

  wds[0].length = (uint16_t)len;
  wds[0].ptr = (Ptr)buf;
  wds[1].length = 0;
  wds[1].ptr = NULL;
  memset(&pb, 0, sizeof(pb));
  pb.udpStream = s->stream;
  pb.csParam.send.remoteHost = sin->sin_addr.s_addr;
  pb.csParam.send.remotePort = sin->sin_port;
  pb.csParam.send.wdsPtr = (Ptr)wds;
  pb.csParam.send.checkSum = 1;
  if ((err = control(&pb, MT_UDPWrite, 0)) != noErr) {
    errno = sock_errno(err);
    return -1;
  }

  return len;
}

ssize_t recvfrom(int fd, void *buf, size_t len, int flags, struct sockaddr *from, socklen_t *fromlen) {
  struct sockaddr_in *sin = (struct sockaddr_in *)from;
  mt_UDPiopb pb;
  mac_sock_t *s;
  size_t n;

  if ((s = get_sock(fd)) == NULL) return -1;
  if (s->type != SOCK_DGRAM) {
    if (from && fromlen) *fromlen = 0;
    return recv(fd, buf, len, flags);
  }
  if (udp_ensure(s) != 0) return -1;
  if (!s->read_pending) udp_start_read(s);

  for (;;) {
    if (!s->read_pending) {
      errno = EIO;
      return -1;
    }
    if (s->readpb.ioResult != MT_inProgress) break;
    if (s->nonblock || (flags & MSG_DONTWAIT)) {
      errno = EWOULDBLOCK;
      return -1;
    }
    thread_yield(0);
    mac_yield(0);
    if (thread_must_end()) {
      errno = EINTR;
      return -1;
    }
  }

  s->read_pending = 0;
  if (s->readpb.ioResult != noErr) {
    errno = sock_errno(s->readpb.ioResult);
    udp_start_read(s);
    return -1;
  }

  n = s->readpb.csParam.receive.rcvBuffLen;
  if (n > len) n = len;
  memcpy(buf, s->readpb.csParam.receive.rcvBuff, n);
  if (sin && fromlen && *fromlen >= sizeof(struct sockaddr_in)) {
    memset(sin, 0, sizeof(struct sockaddr_in));
    sin->sin_len = sizeof(struct sockaddr_in);
    sin->sin_family = AF_INET;
    sin->sin_addr.s_addr = s->readpb.csParam.receive.remoteHost;
    sin->sin_port = s->readpb.csParam.receive.remotePort;
    *fromlen = sizeof(struct sockaddr_in);
  }

  // give the buffer back to MacTCP and wait for the next datagram
  memset(&pb, 0, sizeof(pb));
  pb.udpStream = s->stream;
  pb.csParam.receive.rcvBuff = s->readpb.csParam.receive.rcvBuff;
  control(&pb, MT_UDPBfrReturn, 0);
  udp_start_read(s);

  return n;
}

int shutdown(int fd, int how) {
  mac_sock_t *s;

  if ((s = get_sock(fd)) == NULL) return -1;
  if (s->type == SOCK_STREAM && s->state == S_CONNECTED && (how == SHUT_WR || how == SHUT_RDWR) && !s->wr_shut) {
    memset(&s->closepb, 0, sizeof(mt_TCPiopb));
    s->closepb.tcpStream = s->stream;
    s->closepb.csParam.close.validityFlags = MT_timeoutValue | MT_timeoutAction;
    s->closepb.csParam.close.ulpTimeoutValue = 30;
    s->closepb.csParam.close.ulpTimeoutAction = 1;
    control(&s->closepb, MT_TCPClose, 1);
    s->wr_shut = 1;
  }

  return 0;
}

int mac_sock_close(int fd) {
  mac_sock_t *s;
  uint64_t end;

  if ((s = get_sock(fd)) == NULL) return -1;

  if (s->type == SOCK_DGRAM) {
    udp_release(s);
  } else {
    check_open(s);
    if (s->state == S_CONNECTED && !s->wr_shut) {
      shutdown(fd, SHUT_WR);
    }
    if (s->wr_shut) {
      // give the close a moment to reach the peer (MT_TCPRelease aborts what
      // is still in flight)
      end = mac_microseconds() + CLOSE_WAIT_US;
      while (s->closepb.ioResult == MT_inProgress && mac_microseconds() < end) {
        thread_yield(0);
        mac_yield(0);
      }
    }
    tcp_release(s);
  }

  socks[fd] = NULL;
  free(s);
  mac_fd_release(fd);

  return 0;
}

ssize_t mac_sock_read(int fd, void *buf, size_t len) {
  return recv(fd, buf, len, 0);
}

ssize_t mac_sock_write(int fd, const void *buf, size_t len) {
  return send(fd, buf, len, 0);
}

int setsockopt(int fd, int level, int name, const void *value, socklen_t len) {
  (void)level; (void)name; (void)value; (void)len;
  if (get_sock(fd) == NULL) return -1;
  // nothing to set: MacTCP has no such options (TCP_NODELAY: every send is
  // pushed; SO_REUSEADDR, SO_BROADCAST: always allowed)
  return 0;
}

int getsockopt(int fd, int level, int name, void *value, socklen_t *len) {
  mac_sock_t *s;

  if ((s = get_sock(fd)) == NULL) return -1;
  if (value == NULL || len == NULL || *len < sizeof(int)) {
    errno = EINVAL;
    return -1;
  }

  if (level == SOL_SOCKET) {
    switch (name) {
      case SO_TYPE:
        *(int *)value = s->type;
        *len = sizeof(int);
        return 0;
      case SO_ERROR:
        check_open(s);
        *(int *)value = s->error;
        s->error = 0;
        *len = sizeof(int);
        return 0;
    }
  }

  *(int *)value = 0;
  *len = sizeof(int);
  return 0;
}

static int fill_sin(struct sockaddr *addr, socklen_t *len, mt_ip_addr host, uint16_t port) {
  struct sockaddr_in *sin = (struct sockaddr_in *)addr;

  if (addr == NULL || len == NULL || *len < sizeof(struct sockaddr_in)) {
    errno = EINVAL;
    return -1;
  }
  memset(sin, 0, sizeof(struct sockaddr_in));
  sin->sin_len = sizeof(struct sockaddr_in);
  sin->sin_family = AF_INET;
  sin->sin_addr.s_addr = host;
  sin->sin_port = port;
  *len = sizeof(struct sockaddr_in);

  return 0;
}

int getsockname(int fd, struct sockaddr *addr, socklen_t *len) {
  mac_sock_t *s;

  if ((s = get_sock(fd)) == NULL) return -1;
  check_open(s);
  return fill_sin(addr, len, s->lhost ? s->lhost : our_addr, s->lport);
}

int getpeername(int fd, struct sockaddr *addr, socklen_t *len) {
  mac_sock_t *s;

  if ((s = get_sock(fd)) == NULL) return -1;
  if (s->state != S_CONNECTED) {
    errno = ENOTCONN;
    return -1;
  }
  return fill_sin(addr, len, s->rhost, s->rport);
}

int poll(struct pollfd *pfds, nfds_t nfds, int timeout) {
  uint64_t end = 0;
  nfds_t i;
  int n, r;

  if (timeout > 0) end = mac_microseconds() + (uint64_t)timeout * 1000;

  for (;;) {
    n = 0;
    for (i = 0; i < nfds; i++) {
      pfds[i].revents = 0;
      if (pfds[i].fd < 0) continue;
      if (!mac_fd_is_socket(pfds[i].fd)) {
        // files never block
        pfds[i].revents = pfds[i].events & (POLLIN | POLLOUT);
      } else {
        if (pfds[i].events & POLLIN) {
          r = mac_sock_ready(pfds[i].fd, 0);
          if (r < 0) pfds[i].revents |= POLLNVAL;
          else if (r > 0) pfds[i].revents |= POLLIN;
        }
        if (pfds[i].events & POLLOUT) {
          r = mac_sock_ready(pfds[i].fd, 1);
          if (r < 0) pfds[i].revents |= POLLNVAL;
          else if (r > 0) pfds[i].revents |= POLLOUT;
        }
        if (socks[pfds[i].fd] && socks[pfds[i].fd]->state == S_CLOSED && socks[pfds[i].fd]->error) {
          pfds[i].revents |= POLLERR;
        }
      }
      if (pfds[i].revents) n++;
    }
    if (n > 0 || timeout == 0) return n;
    if (timeout > 0 && mac_microseconds() >= end) return 0;
    thread_yield(0);
    mac_yield(0);
  }
}

// --- address conversions ---

int inet_pton(int af, const char *src, void *dst) {
  uint32_t a = 0, v;
  int i, digits;

  if (af != AF_INET) {
    errno = EAFNOSUPPORT;
    return -1;
  }
  for (i = 0; i < 4; i++) {
    v = 0;
    digits = 0;
    while (*src >= '0' && *src <= '9') {
      v = v * 10 + (*src++ - '0');
      if (++digits > 3 || v > 255) return 0;
    }
    if (digits == 0) return 0;
    a = (a << 8) | v;
    if (i < 3) {
      if (*src != '.') return 0;
      src++;
    }
  }
  if (*src) return 0;
  ((struct in_addr *)dst)->s_addr = a;

  return 1;
}

const char *inet_ntop(int af, const void *src, char *dst, socklen_t size) {
  uint32_t a;
  char buf[16];

  if (af != AF_INET) {
    errno = EAFNOSUPPORT;
    return NULL;
  }
  a = ((const struct in_addr *)src)->s_addr;
  snprintf(buf, sizeof(buf), "%u.%u.%u.%u", (unsigned)(a >> 24), (unsigned)((a >> 16) & 0xFF), (unsigned)((a >> 8) & 0xFF), (unsigned)(a & 0xFF));
  if (strlen(buf) + 1 > size) {
    errno = ENOSPC;
    return NULL;
  }
  strcpy(dst, buf);

  return dst;
}

in_addr_t inet_addr(const char *cp) {
  struct in_addr a;

  return inet_pton(AF_INET, cp, &a) == 1 ? a.s_addr : INADDR_NONE;
}

char *inet_ntoa(struct in_addr in) {
  static char buf[16];

  inet_ntop(AF_INET, &in, buf, sizeof(buf));
  return buf;
}

// --- the name resolver ---

#ifdef __m68k__
typedef OSErr (*dnr_open_t)(long selector, char *fileName);
typedef OSErr (*dnr_close_t)(long selector);
typedef OSErr (*dnr_str_t)(long selector, char *hostName, struct mt_hostInfo *rtnStruct, void *resultproc, char *userDataPtr);
#else
// the 'dnrp' code is 68K code: called through the Mixed Mode Manager
// (CallUniversalProc(), declared by the Multiversal interfaces)
#define DNR_OPEN_PROCINFO  0x000003E1  // C, OSErr (long, char *)
#define DNR_CLOSE_PROCINFO 0x000000E1  // C, OSErr (long)
#define DNR_STR_PROCINFO   0x0000FFE1  // C, OSErr (long, char *, hostInfo *, ResultUPP, char *)
#define RESULT_PROCINFO    0x000003C0  // Pascal, void (hostInfo *, char *)
#endif

static Handle dnr_code;
static void *dnr_entry;
static int dnr_failed;

static pascal void dnr_result(struct mt_hostInfo *info, char *userData) {
  (void)info;
  *(volatile char *)userData = 1;
}

static void *dnr_result_upp;

// the MacTCP control panel (or Open Transport's TCP/IP one) in folder
// (vref, dirid): a file of type 'cdev' whose creator is 'ztcp' or 'mtcp'
static int16_t open_tcp_panel(int16_t vref, int32_t dirid) {
  CInfoPBRec pb;
  unsigned char name[64];
  int i;

  for (i = 1;; i++) {
    memset(&pb, 0, sizeof(pb));
    name[0] = 0;
    pb.hFileInfo.ioNamePtr = name;
    pb.hFileInfo.ioVRefNum = vref;
    pb.hFileInfo.ioDirID = dirid;
    pb.hFileInfo.ioFDirIndex = i;
    if (PBGetCatInfoSync(&pb) != noErr) break;
    if (pb.hFileInfo.ioFlAttrib & 0x10) continue;
    if (pb.hFileInfo.ioFlFndrInfo.fdType == 'cdev' &&
        (pb.hFileInfo.ioFlFndrInfo.fdCreator == 'ztcp' || pb.hFileInfo.ioFlFndrInfo.fdCreator == 'mtcp')) {
      return HOpenResFile(vref, dirid, name, fsRdPerm);
    }
  }

  return -1;
}

static int dnr_open(void) {
  static const OSType folders[] = { kSystemFolderType, kControlPanelFolderType, kExtensionFolderType };
  int16_t vref, refnum = -1, saved;
  long dirid;
  OSErr err;
  int i;

  if (dnr_entry) return 0;
  if (dnr_failed) return -1;

  saved = CurResFile();
  for (i = 0; i < 3 && refnum == -1; i++) {
    if (FindFolder((int16_t)-32768, folders[i], false, &vref, &dirid) == noErr) {
      refnum = open_tcp_panel(vref, dirid);
    }
  }

  // (the System file is in the resource chain: a resolver installed there
  // is found as well)
  dnr_code = GetIndResource('dnrp', 1);
  if (dnr_code) DetachResource(dnr_code);
  if (refnum != -1) CloseResFile(refnum);
  UseResFile(saved);

  if (dnr_code == NULL) {
    debug(DEBUG_ERROR, TAG, "no MacTCP name resolver ('dnrp' resource) found");
    dnr_failed = 1;
    return -1;
  }
  MoveHHi(dnr_code);
  HLock(dnr_code);
  dnr_entry = *dnr_code;

#ifdef __m68k__
  dnr_result_upp = (void *)dnr_result;
  err = ((dnr_open_t)dnr_entry)(OPENRESOLVER, NULL);
#else
  dnr_result_upp = NewRoutineDescriptor((ProcPtr)dnr_result, RESULT_PROCINFO, GetCurrentArchitecture());
  err = (OSErr)CallUniversalProc((UniversalProcPtr)dnr_entry, DNR_OPEN_PROCINFO, (long)OPENRESOLVER, (char *)NULL);
#endif
  if (err != noErr) {
    debug(DEBUG_ERROR, TAG, "OpenResolver failed (%d)", err);
    HUnlock(dnr_code);
    DisposeHandle(dnr_code);
    dnr_code = NULL;
    dnr_entry = NULL;
    dnr_failed = 1;
    return -1;
  }

  return 0;
}

static int dnr_lookup(const char *host, uint32_t *addr) {
  struct mt_hostInfo *info;
  volatile char done = 0;
  char name[256];
  uint64_t end;
  OSErr err;
  int r = -1;

  if (dnr_open() != 0) return -1;
  if ((info = (struct mt_hostInfo *)NewPtrClear(sizeof(struct mt_hostInfo))) == NULL) return -1;
  strncpy(name, host, sizeof(name) - 1);
  name[sizeof(name) - 1] = 0;

#ifdef __m68k__
  err = ((dnr_str_t)dnr_entry)(STRTOADDR, name, info, dnr_result_upp, (char *)&done);
#else
  err = (OSErr)CallUniversalProc((UniversalProcPtr)dnr_entry, DNR_STR_PROCINFO, (long)STRTOADDR, name, info, dnr_result_upp, (char *)&done);
#endif

  if (err == MT_cacheFault) {
    // the answer comes from the network: dnr_result says when
    end = mac_microseconds() + 30000000ULL;
    while (!done && mac_microseconds() < end) {
      thread_yield(0);
      mac_yield(0);
    }
    err = done ? (OSErr)info->rtnCode : MT_commandTimeout;
  }

  if (err == noErr && info->addr[0]) {
    *addr = info->addr[0];
    r = 0;
  } else {
    debug(DEBUG_ERROR, TAG, "cannot resolve \"%s\" (%d)", host, err);
  }

  if (done || err != MT_cacheFault) {
    DisposePtr((Ptr)info);
  }
  // (on a timeout the resolver may still write into info: it is leaked)

  return r;
}

void mac_net_finish(void) {
  int i;

  for (i = 0; i < FD_SETSIZE; i++) {
    if (socks[i]) mac_sock_close(i);
  }
  if (dnr_entry) {
#ifdef __m68k__
    ((dnr_close_t)dnr_entry)(CLOSERESOLVER);
#else
    CallUniversalProc((UniversalProcPtr)dnr_entry, DNR_CLOSE_PROCINFO, (long)CLOSERESOLVER);
#endif
    HUnlock(dnr_code);
    DisposeHandle(dnr_code);
    dnr_code = NULL;
    dnr_entry = NULL;
  }
}

int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) {
  struct addrinfo *ai;
  struct sockaddr_in *sin;
  struct in_addr a;
  uint32_t addr;
  int port = 0, socktype = 0, protocol = 0;

  if (res == NULL) return EAI_FAIL;
  *res = NULL;

  if (hints) {
    if (hints->ai_family != AF_UNSPEC && hints->ai_family != AF_INET) return EAI_FAMILY;
    socktype = hints->ai_socktype;
    protocol = hints->ai_protocol;
  }
  if (service) port = atoi(service);

  if (node == NULL || node[0] == 0) {
    addr = (hints && (hints->ai_flags & AI_PASSIVE)) ? INADDR_ANY : INADDR_LOOPBACK;
  } else if (inet_pton(AF_INET, node, &a) == 1) {
    addr = a.s_addr;
  } else if (!strcmp(node, "localhost")) {
    addr = INADDR_LOOPBACK;
  } else {
    if (hints && (hints->ai_flags & AI_NUMERICHOST)) return EAI_NONAME;
    if (mac_net_init() != 0) return EAI_FAIL;
    if (dnr_lookup(node, &addr) != 0) return EAI_NONAME;
  }

  if ((ai = calloc(1, sizeof(struct addrinfo) + sizeof(struct sockaddr_in))) == NULL) return EAI_MEMORY;
  sin = (struct sockaddr_in *)(ai + 1);
  sin->sin_len = sizeof(struct sockaddr_in);
  sin->sin_family = AF_INET;
  sin->sin_port = (in_port_t)port;
  sin->sin_addr.s_addr = addr;
  ai->ai_family = AF_INET;
  ai->ai_socktype = socktype ? socktype : SOCK_STREAM;
  ai->ai_protocol = protocol;
  ai->ai_addrlen = sizeof(struct sockaddr_in);
  ai->ai_addr = (struct sockaddr *)sin;
  *res = ai;

  return 0;
}

void freeaddrinfo(struct addrinfo *res) {
  struct addrinfo *next;

  for (; res; res = next) {
    next = res->ai_next;
    free(res);
  }
}

const char *gai_strerror(int errcode) {
  switch (errcode) {
    case EAI_NONAME: return "host not found";
    case EAI_FAMILY: return "address family not supported";
    case EAI_MEMORY: return "out of memory";
    case EAI_FAIL:   return "the network is not available";
  }
  return "name resolution error";
}

struct hostent *gethostbyname(const char *name) {
  static struct hostent he;
  static char *list[2];
  static uint32_t addr;
  static char hname[256];
  struct in_addr a;

  if (inet_pton(AF_INET, name, &a) == 1) {
    addr = a.s_addr;
  } else if (mac_net_init() != 0 || dnr_lookup(name, &addr) != 0) {
    return NULL;
  }
  strncpy(hname, name, sizeof(hname) - 1);
  list[0] = (char *)&addr;
  list[1] = NULL;
  he.h_name = hname;
  he.h_aliases = NULL;
  he.h_addrtype = AF_INET;
  he.h_length = 4;
  he.h_addr_list = list;

  return &he;
}

int gethostname(char *name, size_t len) {
  strncpy(name, "macintosh", len);
  if (len) name[len - 1] = 0;
  return 0;
}
