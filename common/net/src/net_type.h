// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <string>

#include "common/net/inc/network_io.h"

namespace cutio::net {

struct IPAddr {
  std::string local_ip;
  std::string local_port;
  std::string remote_ip;
  std::string remote_port;

  IPAddr() = default;
};

struct ConnQueueItem {
  void* io_event;
  void* write_io_thread;
  void* session;
  void* io_service;
  void* filter;
  int sfd;
  SocketType socket_type;
  SocketStatus socket_status;
  IOThreadModeType io_thread_mode;
  NetworkType network_type;
  bool del_self;
  IPAddr ip_addr;
};

}  // namespace cutio::net
