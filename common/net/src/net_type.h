// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <string>
#include <utility>

#include "common/net/inc/io_buffer.h"
#include "common/net/inc/network_io.h"

namespace cutio::net {

struct Address {
  std::string local_ip;
  uint16_t local_port;
  std::string remote_ip;
  uint16_t remote_port;

  Address() : local_port(0), remote_port(0) {}
  Address(std::string remote_ip, uint16_t remote_port, std::string local_ip, uint16_t local_port)
      : local_ip(std::move(local_ip)), local_port(local_port),
        remote_ip(std::move(remote_ip)), remote_port(remote_port) {}
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
  IOThreadType io_thread_type;
  NetworkType network_type;
  bool del_self;
  Address address;

  ConnQueueItem()
      : io_event(nullptr),
        write_io_thread(nullptr),
        session(nullptr),
        io_service(nullptr),
        filter(nullptr),
        sfd(0),
        socket_type(kSocketTypeNull),
        socket_status(kSocketStatusNull),
        io_thread_type(kIOThreadNull),
        network_type(kNetworkTypeNull),
        del_self(false) {}
};

struct WriteQueueItem {
  IOBuffer buf;
  bool success;

  WriteQueueItem() : success(false) {}
};

struct NotifyWriteItem {
  IOBuffer buf;
  bool success;
  SocketStatus socket_status;
  void* session;
  bool del_self;

  NotifyWriteItem()
      : success(false), socket_status(kSocketStatusNull), session(nullptr), del_self(false) {}
};

}  // namespace cutio::net
