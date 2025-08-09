// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/net/demo/udp_server_io_event.h"

#include "common/log/inc/log_controller.h"
#include "common/net/demo/udp_server_data_trans.h"

namespace cutio {

UDPServerIOEvent::UDPServerIOEvent() {
  InfoMsg("UDPServerIOEvent()");
}

UDPServerIOEvent::~UDPServerIOEvent() {
  InfoMsg("~UDPServerIOEvent()");
}

int UDPServerIOEvent::OnIOEvent(net::IOHandler* io_handler, net::IOEventType event_type, const net::IOBuffer& buf) {
  InfoMsg("OnIOEvent");
  if (event_type == net::kIOEventTypeInit) {
    InfoMsg("kIOEventTypeInit")
  } else if(event_type == net::kIOEventTypeOpenOK) {
    auto remote_addr = io_handler->QueryHandleInfo(net::kQueryInfoTypeRemoteIP) + ":" +
        io_handler->QueryHandleInfo(net::kQueryInfoTYpeRemotePort);
    InfoMsg("kIOEventTypeOpenOK: %s", remote_addr.c_str());
    auto* data_trans = new UDPServerDataTrans(io_handler, remote_addr);
    io_handler->Register(data_trans, nullptr);
  } else if (event_type == net::kIOEventTypeReadOK) {
    InfoMsg("kIOEventTypeReadOK")
    InfoMsg("read %u bytes", buf.Size());
  } else if (event_type == net::kIOEventTypeClosed) {
    InfoMsg("kIOEventTypeClosed");
  }
  return 0;
}

}  // namespace cutio
