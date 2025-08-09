// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <string>

namespace cutio::net {

class IOEvent;
class IOService;

class IOUtils {
 public:
  IOUtils() = delete;

  static bool SetNonBlock(int fd);
  static bool SetBlock(int fd);

  static bool CloseSocket(int fd);

  static bool CreatePipe(int* fds);

  // TCP accept.
  static int Accept(int sock, std::string* remote_addr, uint16_t* remote_port);

  // TCP listen.
  static bool TCPListen(const std::string& addr, uint16_t port, int* fd, bool blocking = true);

  // TCP connect.
  static bool TCPConnect(const std::string& addr, uint16_t port, int* fd, std::string* local_addr, uint16_t* local_port);

  // UDP listen.
  static bool UDPListen(const std::string& addr, uint16_t port, int* fd, bool blocking = true);

  // UDP connect.
  static bool UDPConnect(const std::string& addr, uint16_t port, int* fd, std::string* local_addr, uint16_t* local_port);

  // UDX listen.
  static bool UDXListen(uint16_t port, int thread_num, IOEvent* io_event);

  // UDX connect.
  static bool UDXConnect(const std::string& addr, uint16_t port, uint16_t local_port, IOEvent* io_event,
                         IOService* io_service, bool sync);

  static bool UDXP2PCreate(uint16_t port, IOEvent* io_event);

};

}  // namespace cutio::net
