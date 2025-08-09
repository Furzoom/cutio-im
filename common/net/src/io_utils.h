// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <string>

namespace cutio::net {

class IOUtils {
 public:
  IOUtils() = delete;

  static int SetNonBlock(int fd);
  static int SetBlock(int fd);

  static int CloseSocket(int fd);

  static int CreatePipe(int *fds);

  static int Accept(int sock, std::string *remote_addr, std::string *remote_port);

  static int TCPListen(const char *addr, uint16_t port, int *fd, int blocking = 1);

};

}  // namespace cutio::net
