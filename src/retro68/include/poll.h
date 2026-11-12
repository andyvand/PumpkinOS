// poll() for classic Mac OS (Retro68), on the descriptors of
// libpit/macsys.c (files are always ready, MacTCP streams are polled).

#ifndef MACCLASSIC_POLL_H
#define MACCLASSIC_POLL_H

#ifdef __cplusplus
extern "C" {
#endif

#define POLLIN   0x0001
#define POLLPRI  0x0002
#define POLLOUT  0x0004
#define POLLERR  0x0008
#define POLLHUP  0x0010
#define POLLNVAL 0x0020

typedef unsigned int nfds_t;

struct pollfd {
  int fd;
  short events;
  short revents;
};

int poll(struct pollfd *fds, nfds_t nfds, int timeout);

#ifdef __cplusplus
}
#endif

#endif
