// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/net/src/io_utils.h"

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

namespace cutio::net {

int IOUtils::SetNonBlock(int fd) {
  int flags = fcntl(fd, F_GETFL);
  if (flags < 0) {
    return flags;
  }

  flags |= O_NONBLOCK;
  if (fcntl(fd, F_SETFL, flags) < 0) {
    return -1;
  }

  return 0;
}

int IOUtils::SetBlock(int fd) {
  int flags = fcntl(fd, F_GETFL);
  if (flags < 0) {
    return false;
  }

  flags &= ~O_NONBLOCK;
  if (fcntl(fd, F_SETFL, flags) < 0) {
    return -1;
  }

  return 0;
}

int IOUtils::CloseSocket(int fd) {
  return close(fd);
}

int IOUtils::CreatePipe(int* fds) {
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0) {
    perror("can't create notify pipe");
    return -1;
  }

  SetNonBlock(fds[0]);
  SetNonBlock(fds[1]);

  return 0;
}

int IOUtils::Accept(int sock, std::string* remote_addr, std::string* remote_port) {
  return -1;
}

int IOUtils::TCPListen(const char* addr, uint16_t port, int* fd, int blocking) {
  return -1;
}

}  // namespace cutio::net