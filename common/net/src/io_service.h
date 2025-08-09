// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <pthread.h>

#include <event2/event.h>
#include <event2/event_struct.h>

#include <string>
#include <vector>

#include "common/net/src/io_thread.h"
#include "common/net/inc/network_io.h"

namespace cutio::net {

class IOService {
public:
  IOService();
  virtual ~IOService();

  void StartThread();

  static void *ThreadFunc(void *arg);

  virtual bool StartUp();

  static void OnTCPAcceptCallback(int fd, short which, void* arg);

  static void OnUDPAcceptCallback(int fd, short which, void* arg);

  void DispatchNewCon(int nfd, SocketType socket_type, NetworkType network_type,
                      const std::string& ip, const std::string& port);

 private:
  struct StartUpArg {

  };

 private:
  struct event_base* event_base_;
  struct event accept_event_;

  std::vector<IOThread*> read_io_threads_;
  std::vector<IOThread*> write_io_threads_;

  pthread_mutex_t mutex_;
  pthread_cond_t cond_;

  StartUpArg start_up_arg_;
  bool volatile success_;
  int count_;
};

}  // namespace cutio::net
