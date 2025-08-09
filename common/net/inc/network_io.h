// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

namespace cutio::net {

// Thread mode.
enum IOThreadModeType {
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

// Network type.
enum NetworkType {
  kNetworkTypeNull = 0,
  kNetworkTypeTCP = 1,
  kNetworkTypeUDP = 2,
  kNetworkTypeUDX = 3,
};

class IOEvent {
 public:
  virtual ~IOEvent() = default;
  virtual int OnIOEvent() = 0;
  virtual int OnP2PIOEvent() { return 0; };
};

}  // namespace cutio::net
