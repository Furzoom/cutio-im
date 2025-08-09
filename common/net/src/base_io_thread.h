// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <event2/event.h>

#include "common/net/src/io_thread.h"

namespace cutio::net {

class BaseIOThread : public IOThread {
 public:
  BaseIOThread();
  ~BaseIOThread() override;

  bool SendNotify();

 protected:
  typedef void(*Callback(int, short, void*));

  void ExitEventLoop();

  void InitPipeEvent(event_callback_fn callback, void* arg);

  void OnThreadRun() override;
  std::string GetThreadName() override;

 protected:
  event_base* base_;
  event* notify_event_;
  std::string thread_name_;
};

}  // namespace cutio::net
