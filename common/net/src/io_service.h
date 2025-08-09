// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <event2/event.h>
#include <event2/event_struct.h>

#include <condition_variable>
#include <mutex>
#include <string>
#include <vector>

#include "common/net/inc/network_io.h"
#include "common/net/src/base_io_thread.h"
#include "common/net/src/io_thread.h"
#include "common/net/src/net_type.h"
#include "inc/inc.h"

namespace cutio::net {

class IOService : public noncopyable {
public:
  IOService();
  virtual ~IOService();

  void StartThread();

  static void *ThreadFunc(void *arg);

  virtual bool StartUp(IOServiceType service_type, const Address& address, IOEvent* io_event,
                       NetFilter* fileter, int thread_num, IOThreadType io_thread_type, bool sync);

  static void OnTCPAcceptCallback(int fd, short which, void* arg);

  static void OnUDPAcceptCallback(int fd, short which, void* arg);

  void DispatchNewCon(int nfd, SocketType socket_type, NetworkType network_type,
                      const std::string& ip, uint16_t port);

 private:
  void* Run();

  struct StartUpArg {
    IOServiceType service_type;
    IOThreadType thread_type;
    Address address;
    int thread_num;
    IOEvent* io_event;
    NetFilter* filter;
    bool sync;
  };

 private:
  struct event_base* event_base_;
  struct event accept_event_;

  std::vector<BaseIOThread*> read_io_threads_;
  std::vector<BaseIOThread*> write_io_threads_;

  std::mutex mutex_;
  std::condition_variable cond_;

  StartUpArg start_up_arg_;
  bool volatile success_;
  uint32_t count_;
};

}  // namespace cutio::net
