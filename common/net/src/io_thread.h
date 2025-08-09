// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <queue>

#include "common/net/inc/mutex.h"
#include "inc/inc.h"

namespace cutio::net {

class IOThread : public noncopyable {
public:
  IOThread();
  virtual ~IOThread();

  int Push(void *arg);
  Lock *Push(void *arg, int *size);
  void *Pop();
  void ClearQueue();

  static void *ThreadFun(void *arg);

  void StartThread();
  void StopThread();

 protected:
  virtual void OnRelease() = 0;
  virtual void OnThreadRun() = 0;

protected:
  int notify_receive_fd_;
  int notify_send_fd_;

private:
  Mutex mu_;
  std::queue<void*> queue_;
};

}  // namespace cutio::net
