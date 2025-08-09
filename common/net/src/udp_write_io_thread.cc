// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/net/src/udp_write_io_thread.h"

#include <event2/event.h>

#include "common/log/inc/log_controller.h"
#include "common/net/src/net_type.h"

namespace cutio::net {

std::atomic<uint32_t> UDPWriteIOThread::index_ = 0;

UDPWriteIOThread::UDPWriteIOThread() {
  InitPipeEvent(OnPipeProcessCallback, this);
  thread_name_ = "udp_write:" + std::to_string(index_++);
}

UDPWriteIOThread::~UDPWriteIOThread() {}

void UDPWriteIOThread::SetWriteEvent(int fd, Session* session) {
  event_assign(&session->write_event_, base_, fd, EV_WRITE, OnNetWriteProcessCallback, session);
}

void UDPWriteIOThread::OnPipeProcessCallback(int fd, short which, void* arg) {
  auto* self = static_cast<UDPWriteIOThread*>(arg);
  self->OnNotify(fd, which);
}

void UDPWriteIOThread::OnNetWriteProcessCallback(int fd, short which, void* arg) {
  auto* self = static_cast<Session*>(arg);
  self->OnUDPSend(fd, which);
}

void UDPWriteIOThread::OnRelease() {
  auto* item = new NotifyWriteItem;
  item->del_self = true;
  if (Push(item) <= 1) {
    if (!SendNotify()) {
      ASSERTS_RETURN(false, "send failed. error = %s", strerror(errno));
    }
  }
}

void UDPWriteIOThread::OnNotify(int fd, short which) {
  char buf[1];
  if (recv(fd, buf, 1, 0) != 1) {
    ASSERTS_RETURN(false, "recv from PIPE failed. error = %s", strerror(errno));
    ErrorMsg("recv from PIPE failed. error = %s", strerror(errno));
    return;
  }

  NotifyWriteItem* item;
  while ((item = static_cast<NotifyWriteItem*>(Pop())) != nullptr) {
    if (item->del_self) {
      ExitEventLoop();
      delete item;
      break;
    }
    auto* session = static_cast<Session*>(item->session);
    if (session->close_) {
      if (item->socket_status == kSocketStatusClose) {
        event_del(&session->write_event_);
        session->OnWriteThreadClose();
      }
    } else {
      event_add(&session->write_event_, nullptr);
    }
    delete item;
  }
}

}  // namespace cutio::net
