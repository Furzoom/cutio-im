// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <unordered_set>

#include "common/net/inc/network_io.h"

namespace cutio {

class UDPServerDataTrans : public net::IOEvent {
 public:
  UDPServerDataTrans(net::IOHandler* handler, std::string address);
  ~UDPServerDataTrans() override;

  static void AddEvent(UDPServerDataTrans* event);
  static void DelEvent(UDPServerDataTrans* event);
  static void TransferAll(const net::IOBuffer& buf, UDPServerDataTrans* event);

 protected:
  int OnIOEvent(net::IOHandler * io_handler, net::IOEventType event_type, const net::IOBuffer& buf) override;

 private:
  net::IOHandler* handler_;
  std::string address_;
  static std::unordered_set<UDPServerDataTrans*> events_;
};

}  // namespace cutio
