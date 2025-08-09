// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/net/src/read_io_thread.h"

#include <event2/event.h>

#include "common/log/inc/log_controller.h"
#include "common/net/src/session.h"
#include "common/net/src/write_io_thread.h"

namespace cutio::net {

std::atomic<uint32_t> ReadIOThread::index_ = 0;

ReadIOThread::ReadIOThread() {
  InitPipeEvent(OnPipeProcessCallback, this);
  thread_name_ = "tcp_read:" + std::to_string(index_++);
}

ReadIOThread::~ReadIOThread() {}

void ReadIOThread::AddSocket(int fd) {
  sockets_.insert(fd);
}

bool ReadIOThread::DelSocket(int fd) {
  auto it = sockets_.find(fd);
  if (it != sockets_.end()) {
    sockets_.erase(it);
    return true;
  }
  return false;
}

void ReadIOThread::OnPipeProcessCallback(int fd, short which, void* arg) {
  auto* self = static_cast<ReadIOThread*>(arg);
  self->OnNotify(fd, which);
}

void ReadIOThread::OnNetReadProcessCallback(int fd, short which, void* arg) {
  auto* session = static_cast<Session*>(arg);
  session->OnTCPReceive(fd, which);
}

void ReadIOThread::OnRelease() {
  auto* item = new ConnQueueItem;
  item->del_self = true;
  if (Push(item) <= 1) {
    if (!SendNotify()) {
      ASSERTS_RETURN(false, "send notify failed. error = %s", strerror(errno));
      ErrorMsg("send notify failed. error = %s", strerror(errno));
    }
  }
}

void ReadIOThread::OnNotify(int fd, short which) {
  char buf[1];
  if (recv(fd, buf, 1, 0) != 1) {
    ASSERTS_RETURN(false, "recv from PIPE failed. error = %s", strerror(errno));
    ErrorMsg("recv from PIPE failed. error = %s", strerror(errno));
    return;
  }

  ConnQueueItem* item = nullptr;
  while (true) {
    item = static_cast<ConnQueueItem*>(Pop());
    if (item == nullptr) {
      break;
    }

    if (item->del_self) {
      ExitEventLoop();
      delete item;
      break;
    }

    if (item->socket_status == kSocketStatusConnect) {
      auto* session = NewSession(item->sfd, static_cast<BaseIOThread*>(item->write_io_thread),
                                 item->io_thread_type, item->network_type, item->address);
      session->Register(static_cast<IOEvent*>(item->io_event), static_cast<NetFilter*>(item->filter));
      AddSocket(item->sfd);
      if (item->socket_type == kSocketTypeAccept) {
        session->OnConnect(item->sfd);
      } else if (item->socket_type == kSocketTypeConnect) {
        session->OnConnectSuccess(item->sfd, static_cast<IOService*>(item->io_service));
      }
    } else if (item->socket_status == kSocketStatusClose) {
      auto ret = DelSocket(item->sfd);
      if (ret) {
        auto* session = static_cast<Session*>(item->session);
        event_del(&session->read_event_);
        session->OnRecvClose();
      }
    }

    delete item;
  }
}

Session* ReadIOThread::NewSession(int sfd, BaseIOThread* write_io_thread, IOThreadType io_thread_type,
                                  NetworkType network_type, const Address& address) {
  auto* session = new Session(write_io_thread, this, io_thread_type, network_type, address);
  if (network_type == kNetworkTypeTCP) {
    event_assign(&session->read_event_, base_, sfd, EV_READ | EV_PERSIST, OnNetReadProcessCallback, session);
    if (event_add(&session->read_event_, nullptr) == -1) {
      ASSERTS_RETURN_NULL(false, "add TCP fd(%d) to event failed", sfd);
    }
    dynamic_cast<WriteIOThread*>(write_io_thread)->SetWriteEvent(sfd, session);
    session->SetSendBuf(session->kDefaultBufferSize);
  }
  return session;
}

}  // namespace cutio::net
