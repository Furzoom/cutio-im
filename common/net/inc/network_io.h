// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <string>

#include "common/net/inc/net_filter.h"
#include "common/net/inc/io_buffer.h"

namespace cutio::net {

// Thread mode.
enum IOThreadType {
  kIOThreadNull = 0,
  kIOThreadFree = 1,
  kIOThreadAPP = 2,
};

// Socket type.
enum SocketType {
  kSocketTypeNull = 0,
  kSocketTypeAccept = 1,
  kSocketTypeConnect = 2,
};

// Socket status.
enum SocketStatus {
  kSocketStatusNull = 0,
  kSocketStatusConnect = 1,
  kSocketStatusClose = 2,
};

// Service types.
enum IOServiceType {
  kIOServiceTypeUnknown = 0,
  kIOServiceTypeTCPServer = 1,
  kIOServiceTypeTCPClient = 2,
  kIOServiceTypeUDPServer = 3,
  kIOServiceTypeUDPClient = 4,
  kIOServiceTypeUDXServer = 5,
  kIOServiceTypeUDXClient = 6,
  kIOServiceTypeUDXP2PClient = 7,
};

// Network type.
enum NetworkType {
  kNetworkTypeNull = 0,
  kNetworkTypeTCP = 1,
  kNetworkTypeUDP = 2,
  kNetworkTypeUDX = 3,
};

// IO event type.
enum IOEventType {
  // Invalid IOEventType
  kIOEventTypeUnknown = 0,

  // Band IP/port success
  kIOEventTypeInit = 1,

  // Accepted a new connection
  kIOEventTypeOpenOK = 2,

  // Connected to server
  kIOEventTypeClientOpenOK = 3,

  // Read data
  kIOEventTypeReadOK = 4,

  // Write data
  kIOEventTypeWriteOK = 5,

  // Write failed
  kIOEventTypeWriteFailed = 6,

  // Open server failed
  kIOEventTypeOpenFailed = 7,

  // Connection closed
  kIOEventTypeClosed = 8,
};

class IOHandler;

class IOEvent {
 public:
  virtual ~IOEvent() = default;
  virtual int OnIOEvent(IOHandler* io_handler, IOEventType event_type, const IOBuffer& buf) = 0;
  virtual int OnP2PIOEvent(IOHandler* io_handler, IOEventType event_type, const IOBuffer& buf) { return 0; }
};

enum QueryInfoType {
  kQueryInfoTypeLocalIP = 1,
  kQueryInfoTypeLocalPort = 2,
  kQueryInfoTypeRemoteIP = 3,
  kQueryInfoTYpeRemotePort = 4,
};

class IOHandler {
 public:
  virtual ~IOHandler() = default;

  virtual void Write(const IOBuffer& buf, bool reliable) = 0;
  virtual std::string QueryHandleInfo(QueryInfoType type) = 0;
  virtual NetworkType QueryNetworkType() = 0;
  virtual void CloseHandle() = 0;
  virtual void Register(IOEvent* io_event, NetFilter* filter) = 0;
  virtual NetFilter* GetNetFilter() = 0;
  virtual SocketType GetSocketType() = 0;
  virtual bool CanSend() { return true; }
  virtual void SetSendBuf(uint32_t len) = 0;
};

bool CreateServerIOService(IOServiceType service_type, const std::string& local_ip, uint16_t local_port,
                           IOEvent* io_event, NetFilter* filter, int thread_num, IOThreadType thread_type, bool sync);

bool CreateClientIOService(IOServiceType service_type, const std::string& local_ip, uint16_t local_port,
                           const std::string& remote_ip, uint16_t remote_port, IOEvent* io_event,
                           NetFilter* filter, int thread_num, IOThreadType thread_type, bool sync);

}  // namespace cutio::net
