// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include <atomic>

#include "common/net/src/session.h"
#include "common/net/src/base_io_thread.h"

namespace cutio::net {

class UDPWriteIOThread : public BaseIOThread {
 public:
  UDPWriteIOThread();
  ~UDPWriteIOThread() override;

  void SetWriteEvent(int fd, Session* session);

  static void OnPipeProcessCallback(int fd, short which, void* arg);
  static void OnNetWriteProcessCallback(int fd, short which, void* arg);

 private:
  void OnRelease() override;

  void OnNotify(int fd, short which);

 private:
  static std::atomic<uint32_t> index_;
};

}  // namespace cutio::net
