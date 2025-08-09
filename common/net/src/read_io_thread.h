// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <atomic>
#include <unordered_set>

#include "common/net/inc/network_io.h"
#include "common/net/src/base_io_thread.h"
#include "common/net/src/net_type.h"

namespace cutio::net {

class Session;

class ReadIOThread : public BaseIOThread {
 public:
  ReadIOThread();
  ~ReadIOThread() override;

  void AddSocket(int fd);
  bool DelSocket(int fd);

  static void OnPipeProcessCallback(int fd, short which, void* arg);
  static void OnNetReadProcessCallback(int fd, short which, void* arg);

 protected:
  void OnRelease() override;

  void OnNotify(int fd, short which);

  Session* NewSession(int sfd, BaseIOThread* write_io_thread, IOThreadType io_thread_type,
                      NetworkType network_type, const Address& address);

 private:
  std::unordered_set<int> sockets_;

  static std::atomic<uint32_t> index_;
};

}  // namespace cutio::net
