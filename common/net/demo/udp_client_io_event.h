// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include "common/net/inc/network_io.h"

namespace cutio {

class UDPClientIOEvent : public net::IOEvent {
 public:
  UDPClientIOEvent();
  ~UDPClientIOEvent() override;

  void WriteMsg();

 protected:
  int OnIOEvent(net::IOHandler* io_handler, net::IOEventType event_type, const net::IOBuffer& buf) override;

 private:
  net::IOHandler* handler_;
  uint32_t index_;
};

}  // namespace cutio
