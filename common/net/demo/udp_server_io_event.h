// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include "common/net/inc/network_io.h"

namespace cutio {

class UDPServerIOEvent : public net::IOEvent {
 public:
  UDPServerIOEvent();
  ~UDPServerIOEvent() override;

 protected:
  int OnIOEvent(net::IOHandler * io_handler, net::IOEventType event_type, const net::IOBuffer& buf) override;
};

}  // namespace cutio
