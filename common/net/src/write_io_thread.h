// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <atomic>

#include "common/net/src/base_io_thread.h"

namespace cutio::net {

class Session;

class WriteIOThread : public BaseIOThread {
 public:
  WriteIOThread();
  ~WriteIOThread() override;

  void SetWriteEvent(int sfd, Session* session);

  static void OnPipeProcessCallback(int fd, short which, void* arg);
  static void OnNetWriteProcessCallback(int fd, short which, void* arg);

 protected:
  void OnRelease() override;

  void OnNotify(int fd, short which);

 private:
  static std::atomic<uint32_t> index_;
};

}  // namespace cutio::net
