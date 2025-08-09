// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/net/demo/udp_server_data_trans.h"

#include "common/log/inc/log_controller.h"

namespace cutio {

std::unordered_set<UDPServerDataTrans*> UDPServerDataTrans::events_;

UDPServerDataTrans::UDPServerDataTrans(net::IOHandler* handler, std::string address)
    : handler_(handler),
      address_{std::move(address)} {
  InfoMsg("UDPServerDataTrans()");
  AddEvent(this);
}

UDPServerDataTrans::~UDPServerDataTrans() {
  InfoMsg("~UDPServerDataTrans()");
  DelEvent(this);
}

void UDPServerDataTrans::AddEvent(UDPServerDataTrans* event) {
  events_.insert(event);
}

void UDPServerDataTrans::DelEvent(UDPServerDataTrans* event) {
  events_.erase(event);
  InfoMsg("events size is %zu", events_.size());
}

void UDPServerDataTrans::TransferAll(const net::IOBuffer& buf, UDPServerDataTrans* event) {
  for (auto* dt : events_) {
    if (dt == event) {
      continue;
    }
    InfoMsg("transfer to %s", dt->address_.c_str());
    dt->handler_->Write(buf, true);
  }
}

int UDPServerDataTrans::OnIOEvent(net::IOHandler* io_handler, net::IOEventType event_type, const net::IOBuffer& buf) {
  if (event_type == net::kIOEventTypeOpenOK) {
    InfoMsg("kIOEventTypeOpenOK")
  } else if (event_type == net::kIOEventTypeReadOK) {
    InfoMsg("KIOEventTypeReadOK: len: %u, %s", buf.Size(), buf.ToString().c_str());
    TransferAll(buf, this);
  } else if (event_type == net::kIOEventTypeClosed) {
    InfoMsg("kIOEventTypeClosed");
    delete this;
  }
  return 0;
}

} // cutio
