// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <event2/event.h>
#include <event2/event_struct.h>

#include <mutex>
#include <queue>
#include <string>

#include "common/net/inc/net_filter.h"
#include "common/net/inc/network_io.h"
#include "common/net/src/base_io_thread.h"
#include "common/net/src/net_type.h"

namespace cutio::net {

class IOService;

class Session : public IOHandler {
 public:
  Session(BaseIOThread* write_io_thread, BaseIOThread* read_io_thread,
          IOThreadType thread_type, NetworkType network_type, const Address& address);
  ~Session();

  Address GetAddress() const;
  uint32_t PushWriteQueue(WriteQueueItem* item);
  WriteQueueItem* WriteQueueFront();
  void ClearWriteQueue();
  uint32_t WriteQueueSize();

  // IOHandler
  void Register(IOEvent* io_event, NetFilter* filter) override;
  void Write(const IOBuffer& buf, bool reliable) override;
  std::string QueryHandleInfo(QueryInfoType type) override;
  NetworkType QueryNetworkType() override;
  void CloseHandle() override;
  NetFilter* GetNetFilter() override;
  SocketType GetSocketType() override;
  void SetSendBuf(uint32_t len) override;

 public:
  void OnConnectSuccess(int fd, IOService* io_service);
  void OnConnect(int fd);
  void OnRecvData(const IOBuffer& buf);
  void OnRecvClose();
  void OnWriteThreadClose();
  void OnWriteFailure();
  void OnWriteOK(IOBuffer buf);
  void OnIOEvent(IOEventType io_event_type, const IOBuffer& buf);

  void OnUDPSend(int fd, short which);
  void OnTCPReceive(int fd, short which);
  void OnTCPSend(int fd, short which);

 public:
  static void OnReadProcessCallback(int fd, short which, void* arg);

 private:
  void OnReadNotify(int fd, short which);
  void AddWriteQueue(IOBuffer buf);
  void NotifyCloseToWriteThread();
  void NotifyCloseToReadThread();

 private:
  friend class UDPWriteIOThread;
  friend class ReadIOThread;
  friend class WriteIOThread;

 private:
  static const uint32_t kDefaultBufferSize = 65536;

 private:
  event read_event_;
  event write_event_;
  BaseIOThread* read_thread_;
  BaseIOThread* write_thread_;
  IOEvent* io_event_;
  NetFilter* filter_;
  IOService* io_service_;
  IOThreadType io_thread_type_;
  SocketType socket_type_;
  NetworkType network_type_;
  IOBuffer buf_;

  std::queue<WriteQueueItem*> write_queue_;

  volatile bool close_;
  volatile bool send_close_handle_;
  volatile bool send_error_;
  int fd_;
  Address address_;
  mutable std::mutex mu_;
};

}  // namespace cutio::net
