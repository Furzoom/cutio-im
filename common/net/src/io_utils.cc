// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/net/src/io_utils.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>  // struct sockaddr_in
#include <netinet/tcp.h>  // TCP_NODELAY
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "common/log/inc/log_controller.h"

namespace cutio::net {

bool IOUtils::SetNonBlock(int fd) {
  int flags = fcntl(fd, F_GETFL);
  if (flags < 0) {
    return false;
  }

  flags |= O_NONBLOCK;
  if (fcntl(fd, F_SETFL, flags) < 0) {
    return false;
  }

  return true;
}

bool IOUtils::SetBlock(int fd) {
  int flags = fcntl(fd, F_GETFL);
  if (flags < 0) {
    return false;
  }

  flags &= ~O_NONBLOCK;
  if (fcntl(fd, F_SETFL, flags) < 0) {
    return false;
  }

  return true;
}

bool IOUtils::CloseSocket(int fd) {
  return close(fd) == 0;
}

bool IOUtils::CreatePipe(int* fds) {
  if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0) {
    perror("can't create notify pipe");
    return false;
  }

  SetNonBlock(fds[0]);
  SetNonBlock(fds[1]);

  return true;
}

int IOUtils::Accept(int sock, std::string* remote_addr, uint16_t* remote_port) {
  sockaddr_in addr{};
  socklen_t len = sizeof(addr);
  int fd = accept(sock, reinterpret_cast<sockaddr*>(&addr), &len);
  if (fd != -1) {
    SetNonBlock(fd);
    int no_delay = 1;
    int err = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &no_delay, sizeof(no_delay));
    if (err == -1) {
      ErrorMsg("setsockopt is failed. error = %s", strerror(errno));
    }
  }

  if (remote_addr) {
    char buf[16]{};
    if (inet_ntop(AF_INET, &addr.sin_addr, buf, sizeof(buf))) {
      *remote_addr = buf;
    } else {
      ErrorMsg("inet_ntop failed. error = %s", strerror(errno));
    }
  }

  if (remote_port) {
    *remote_port = ntohs(addr.sin_port);
  }

  return fd;
}

bool IOUtils::TCPListen(const std::string& addr, uint16_t port, int* fd, bool blocking) {
  int ln_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (ln_fd < 0) {
    ErrorMsg("create socket failed. error = %s", strerror(errno));
    return false;
  }

  if (!blocking) {
    if (!SetNonBlock(ln_fd)) {
      ErrorMsg("set socket %d nonblocking failed. error = %s", ln_fd, strerror(errno));
      CloseSocket(ln_fd);
      return false;
    }
  }

  int flag = 1;
  if (setsockopt(ln_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
    ErrorMsg("set socket %d reuse address failed. error = %s", ln_fd, strerror(errno));
    CloseSocket(ln_fd);
    return false;
  }

  if (setsockopt(ln_fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag)) < 0) {
    ErrorMsg("set socket %d nodelay failed. error = %s", ln_fd, strerror(errno));
    CloseSocket(ln_fd);
    return false;
  }

  sockaddr_in addr_in{};
  addr_in.sin_family = AF_INET;
  addr_in.sin_port = htons(port);
  addr_in.sin_addr.s_addr = INADDR_ANY;
  if (!addr.empty()) {
    if (inet_pton(AF_INET, addr.c_str(), &addr_in.sin_addr.s_addr) != 1) {
      ErrorMsg("convert address %s to binary failed. error = %s", addr.c_str(), strerror(errno));
      CloseSocket(ln_fd);
      return false;
    }
  }

  if (bind(ln_fd, reinterpret_cast<sockaddr*>(&addr_in), sizeof(addr_in)) < 0) {
    ErrorMsg("bind to %s:%u failed. error = %s", addr.c_str(), port, strerror(errno));
    CloseSocket(ln_fd);
    return false;
  }

  if (listen(ln_fd, 4096) < 0) {
    ErrorMsg("listen %s:%u failed. error = %s", addr.c_str(), port, strerror(errno));
    CloseSocket(ln_fd);
    return false;
  }

  *fd = ln_fd;
  InfoMsg("TCP listen %s:%u success", addr.c_str(), port);

  return true;
}

bool IOUtils::TCPConnect(const std::string& addr, uint16_t port, int* fd,
                         std::string* local_addr, uint16_t* local_port) {
  int cli_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (cli_fd < 0) {
    ErrorMsg("create socket failed. error = %s", strerror(errno));
    return false;
  }

  sockaddr_in addr_in{};
  addr_in.sin_family = AF_INET;
  addr_in.sin_port = htons(port);
  if (inet_pton(AF_INET, addr.c_str(), &addr_in.sin_addr.s_addr) != 1) {
    ErrorMsg("convert address %s to binary failed. error = %s", addr.c_str(), strerror(errno));
    CloseSocket(cli_fd);
    return false;
  }

  SetNonBlock(cli_fd);
  if (connect(cli_fd, reinterpret_cast<sockaddr*>(&addr_in), sizeof(addr_in)) < 0) {
    if (errno == EINPROGRESS) {
      timeval tv{3, 0};
      fd_set write_fds;
      FD_ZERO(&write_fds);
      FD_SET(cli_fd, &write_fds);
      if (select(cli_fd + 1, nullptr, &write_fds, nullptr, &tv) > 0) {
        int err;
        socklen_t len = sizeof(err);
        getsockopt(cli_fd, SOL_SOCKET, SO_ERROR, &err, &len);
        if (err != 0) {
          ErrorMsg("socket(%d) %s:%u has pending error = %d", cli_fd, addr.c_str(), port, err);
          CloseSocket(cli_fd);
          return false;
        }
      } else {
        ErrorMsg("wait socket(%d) %s:%u ready to write failed. error = %s",
                 cli_fd, addr.c_str(), port, strerror(errno));
        CloseSocket(cli_fd);
        return false;
      }
    } else {
      ErrorMsg("connect to %s:%u failed. error = %s", addr.c_str(), port, strerror(errno));
      CloseSocket(cli_fd);
      return false;
    }
  }
  InfoMsg("TCP connect to %s:%u success", addr.c_str(), port);

  sockaddr_in self_addr{};
  socklen_t len = sizeof(self_addr);
  if (getsockname(cli_fd, reinterpret_cast<sockaddr*>(&self_addr), &len) < 0) {
    ErrorMsg("getsocketname(%d) %s:%u failed. error = %s", cli_fd, addr.c_str(), port, strerror(errno));
    CloseSocket(cli_fd);
    return false;
  }

  if (local_addr) {
    char buf[16]{};
    if (inet_ntop(AF_INET, &self_addr.sin_addr, buf, sizeof(buf))) {
      *local_addr = buf;
    } else {
      ErrorMsg("inet_ntop failed. error = %s", strerror(errno));
    }
  }

  if (local_port) {
    *local_port = ntohs(self_addr.sin_port);
  }

  return true;
}

bool IOUtils::UDPListen(const std::string& addr, uint16_t port, int* fd, bool blocking) {
  int ln_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (ln_fd < 0) {
    ErrorMsg("create socket failed. error = %s", strerror(errno));
    return false;
  }

  if (!blocking) {
    if (!SetNonBlock(ln_fd)) {
      ErrorMsg("set socket %d nonblocking failed. error = %s", ln_fd, strerror(errno));
      CloseSocket(ln_fd);
      return false;
    }
  }

  int flag = 1;
  if (setsockopt(ln_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
    ErrorMsg("set socket %d reuse address failed. error = %s", ln_fd, strerror(errno));
    CloseSocket(ln_fd);
    return false;
  }

  sockaddr_in addr_in{};
  addr_in.sin_family = AF_INET;
  addr_in.sin_port = htons(port);
  addr_in.sin_addr.s_addr = INADDR_ANY;
  if (!addr.empty()) {
    if (inet_pton(AF_INET, addr.c_str(), &addr_in.sin_addr.s_addr) != 1) {
      ErrorMsg("convert address %s to binary failed. error = %s", addr.c_str(), strerror(errno));
      CloseSocket(ln_fd);
      return false;
    }
  }

  if (bind(ln_fd, reinterpret_cast<sockaddr*>(&addr_in), sizeof(addr_in)) < 0) {
    ErrorMsg("bind to %s:%u failed. error = %s", addr.c_str(), port, strerror(errno));
    CloseSocket(ln_fd);
    return false;
  }

  *fd = ln_fd;
  InfoMsg("UDP listen %s:%u success", addr.c_str(), port);

  return true;
}

bool IOUtils::UDPConnect(const std::string& addr, uint16_t port, int* fd,
                         std::string* local_addr, uint16_t* local_port) {
  int cli_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (cli_fd < 0) {
    ErrorMsg("create socket failed. error = %s", strerror(errno));
    return false;
  }

  sockaddr_in addr_in{};
  addr_in.sin_family = AF_INET;
  addr_in.sin_port = htons(port);
  if (inet_pton(AF_INET, addr.c_str(), &addr_in.sin_addr.s_addr) != 1) {
    ErrorMsg("convert address %s to binary failed. error = %s", addr.c_str(), strerror(errno));
    CloseSocket(cli_fd);
    return false;
  }

  if (connect(cli_fd, reinterpret_cast<sockaddr*>(&addr_in), sizeof(addr_in)) < 0) {
    ErrorMsg("connect to %s:%u failed. error = %s", addr.c_str(), port, strerror(errno));
    CloseSocket(cli_fd);
    return false;
  }
  InfoMsg("UDP connect to %s:%u success", addr.c_str(), port);

  SetNonBlock(cli_fd);

  sockaddr_in self_addr{};
  socklen_t len = sizeof(self_addr);
  if (getsockname(cli_fd, reinterpret_cast<sockaddr*>(&self_addr), &len) < 0) {
    ErrorMsg("getsocketname(%d) %s:%u failed. error = %s", cli_fd, addr.c_str(), port, strerror(errno));
    CloseSocket(cli_fd);
    return false;
  }

  if (local_addr) {
    char buf[16]{};
    if (inet_ntop(AF_INET, &self_addr.sin_addr, buf, sizeof(buf))) {
      *local_addr = buf;
    } else {
      ErrorMsg("inet_ntop failed. error = %s", strerror(errno));
    }
  }

  if (local_port) {
    *local_port = ntohs(self_addr.sin_port);
  }

  if (fd) {
    *fd = cli_fd;
  }

  return true;
}

bool IOUtils::UDXListen(uint16_t port, int thread_num, IOEvent* io_event) {
  FatalMsg("UDXListen() unimplemented");
  return false;
}

bool IOUtils::UDXConnect(const std::string& addr, uint16_t port, uint16_t local_port,
                         IOEvent* io_event, IOService* io_service, bool sync) {

  FatalMsg("UDXConnect() unimplemented");
  return false;
}

bool IOUtils::UDXP2PCreate(uint16_t port, IOEvent* io_event) {
  FatalMsg("UDXP2PCreate() unimplemented");
  return false;
}

}  // namespace cutio::net