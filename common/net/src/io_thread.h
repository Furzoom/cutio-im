// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <mutex>
#include <queue>

#include "inc/inc.h"

namespace cutio::net {

class IOThread : public noncopyable {
public:
  IOThread();
  ~IOThread() override;

  uint32_t Push(void *arg);
  std::unique_lock<std::mutex> Push(void *arg, int *size);
  void *Pop();
  void ClearQueue();

  static void *ThreadFun(void *arg);

  void StartThread();
  void StopThread();

 protected:
  virtual void OnRelease() = 0;
  virtual void OnThreadRun() = 0;
  virtual std::string GetThreadName() { return std::string{}; };

protected:
  int notify_receive_fd_;
  int notify_send_fd_;

private:
  std::mutex mu_;
  std::queue<void*> queue_;
};

}  // namespace cutio::net
