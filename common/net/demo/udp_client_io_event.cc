// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/net/demo/udp_client_io_event.h"

#include "common/log/inc/log_controller.h"

namespace cutio {

UDPClientIOEvent::UDPClientIOEvent()
    : handler_{nullptr},
      index_{0} {}

UDPClientIOEvent::~UDPClientIOEvent() {
  InfoMsg("~UDPClientIOEvent()");
}

void UDPClientIOEvent::WriteMsg() {
  net::IOBuffer buf{64};
  auto size = snprintf(static_cast<char*>(buf.GetData()), buf.Capacity(), "hi %u\r\n", index_);
  index_++;
  buf.Produce(nullptr, static_cast<uint32_t>(size));
  handler_->Write(buf, true);
}

int UDPClientIOEvent::OnIOEvent(net::IOHandler* io_handler, net::IOEventType event_type, const net::IOBuffer& buf) {
  InfoMsg("event type: %d", event_type);
  if (event_type == net::kIOEventTypeClientOpenOK) {
    InfoMsg("kIOEventTypeClientOpenOK");
    handler_ = io_handler;
    io_handler->Register(this, nullptr);
  } else if (event_type == net::kIOEventTypeOpenOK) {
    InfoMsg("kIOEventTypeOpenOK");
    ASSERTS_RETURN_INT(false, "kIOEventTypeOpenOK")
  } else if (event_type == net::kIOEventTypeWriteOK) {
    InfoMsg("kIOEventTypeWriteOK");
  } else if (event_type == net::kIOEventTypeReadOK) {
    InfoMsg("kIOEventTypeReadOK, len = %u, %s", buf.Size(), buf.ToString().c_str());
  } else if (event_type == net::kIOEventTypeClosed) {
    InfoMsg("kIOEventTypeClosed");
    delete this;
  }
  return 0;
}

}  // namespace cutio
