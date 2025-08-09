// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <event2/event_struct.h>

#include <atomic>
#include <mutex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "common/net/src/base_io_thread.h"
#include "common/net/src/net_type.h"

namespace cutio::net {

class IOEvent;
class Session;
class UDPWriteIOThread;

class UDPReadIOThread : public BaseIOThread {
 public:
  UDPReadIOThread();
  ~UDPReadIOThread() override;

  void AddSocket(int fd);
  bool DelSocket(int fd);

  static void OnPipeProcessCallback(int fd, short which, void* arg);
  static void OnNetReadProcessCallback(int fd, short which, void* arg);

 private:
  void AddRemoteAddrSession(std::string addr, Session* session);
  Session* GetRemoteAddrSession(const std::string& addr) const;
  void DelRemoteAddrSession(const std::string& addr);

  void OnRelease() override;

  void OnNotify(int fd, short which);
  void OnReceive(int fd, short which);

  void ConnectOK(ConnQueueItem* item);

 private:
  std::unordered_set<int> sockets_;
  std::unordered_map<std::string, Session*> sessions_;
  std::mutex mu_;
  event event_;
  IOEvent* io_event_;
  UDPWriteIOThread* write_io_thread_;

  static std::atomic<uint32_t> index_;
};

}  // namespace cutio::net

