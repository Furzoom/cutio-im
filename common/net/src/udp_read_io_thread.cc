// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/net/src/udp_read_io_thread.h"

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>

#include <sstream>

#include "common/log/inc/log_controller.h"
#include "common/net/src/net_type.h"
#include "common/net/src/session.h"
#include "common/net/src/udp_write_io_thread.h"

namespace cutio::net {

std::atomic<uint32_t> UDPReadIOThread::index_ = 0;

UDPReadIOThread::UDPReadIOThread()
  : event_{},
    io_event_(nullptr),
    write_io_thread_(nullptr) {
  InitPipeEvent(OnPipeProcessCallback, this);
  thread_name_ = "udp_read:" + std::to_string(index_++);
}

UDPReadIOThread::~UDPReadIOThread() {}

void UDPReadIOThread::AddSocket(int fd) {
  sockets_.insert(fd);
}

bool UDPReadIOThread::DelSocket(int fd) {
  if (sockets_.find(fd) != sockets_.end()) {
    sockets_.erase(fd);
    return true;
  }

  return false;
}

void UDPReadIOThread::OnPipeProcessCallback(int fd, short which, void* arg) {
  auto* self = static_cast<UDPReadIOThread*>(arg);
  self->OnNotify(fd, which);
}

void UDPReadIOThread::OnNetReadProcessCallback(int fd, short which, void* arg) {
  auto* self = static_cast<UDPReadIOThread*>(arg);
  self->OnReceive(fd, which);
}

void UDPReadIOThread::AddRemoteAddrSession(std::string addr, Session* session) {
  InfoMsg("add UDP session %s, %p", addr.c_str(), session);
  sessions_.insert(std::make_pair(addr, session));
}

Session* UDPReadIOThread::GetRemoteAddrSession(const std::string& addr) const {
  auto it = sessions_.find(addr);
  if (it != sessions_.end()) {
    return it->second;
  }
  return nullptr;
}

void UDPReadIOThread::DelRemoteAddrSession(const std::string& addr) {
  auto it = sessions_.find(addr);
  if (it != sessions_.end()) {
    InfoMsg("remove UDP session %s, %p", addr.c_str(), it->second);
    sessions_.erase(it);
  }
}

void UDPReadIOThread::OnNotify(int fd, short which) {
  char buf[1];
  if (recv(fd, buf, 1, 0) != 1) {
    ASSERTS_RETURN(false, "recv from PIPE failed. error = %s", strerror(errno));
    ErrorMsg("recv from PIPE failed. error = %s", strerror(errno));
    return;
  }

  auto* item = static_cast<ConnQueueItem*>(Pop());
  if (item == nullptr) {
    return;
  }
  if (item->del_self) {
    ExitEventLoop();
    delete item;
    return;
  }

  switch (item->socket_status) {
    case kSocketStatusConnect:
      switch (item->socket_type) {
        case kSocketTypeAccept:
          event_assign(&event_, base_, item->sfd, EV_READ | EV_PERSIST, OnNetReadProcessCallback, this);
          if (event_add(&event_, nullptr) != 0) {
            ASSERTS_RETURN(false, "event_add(%d) failed.", item->sfd);
            return;
          }
          io_event_ = static_cast<IOEvent*>(item->io_event);
          write_io_thread_ = static_cast<UDPWriteIOThread*>(item->write_io_thread);
          break;
        case kSocketTypeConnect:
          DebugMsg("connect");
          ConnectOK(item);
          break;
        default:
          WarnMsg("socket type is null");
          break;
      }
      break;
    case kSocketStatusClose: {
      auto* session = static_cast<Session*>(item->session);
      std::stringstream ss;
      ss << session->GetAddress().remote_ip << ":" << session->GetAddress().remote_port;
      DelRemoteAddrSession(ss.str());
      session->OnRecvClose();
      DebugMsg("socket fd(%d)(%s) closed", item->sfd, ss.str().c_str());
      break;
    }
    default:
    DebugMsg("socket status is null");
      break;
  }

  delete item;
}

void UDPReadIOThread::OnReceive(int fd, short which) {
  sockaddr_in addr{};
  socklen_t addr_len = sizeof(addr);
  IOBuffer buf{1500};
  auto len = recvfrom(fd, buf.GetData(), buf.FreeSize(), 0, reinterpret_cast<sockaddr*>(&addr), &addr_len);
  if (len == -1) {
    ASSERTS_RETURN(false, "recv from fd(%d) failed. error = %s", fd, strerror(errno));
    ErrorMsg("recv from fd(%d) failed. error = %s", fd, strerror(errno));
    return;
  } else if (len == 0) {
    WarnMsg("recv 0 byte from fd(%d)", fd);
    return;
  }
  buf.Produce(nullptr, static_cast<uint32_t>(len));

  std::stringstream ss;
  char ip_str[16]{};
  if (inet_ntop(AF_INET, &addr.sin_addr, ip_str, sizeof(ip_str))) {
    ss << ip_str << ":" << htons(addr.sin_port);
  } else {
    ErrorMsg("inet_ntop failed. error = %s", strerror(errno));
    return;
  }

  auto* session = GetRemoteAddrSession(ss.str());
  if (session == nullptr) {
    Address address;
    address.remote_ip = ip_str;
    address.remote_port = htons(addr.sin_port);
    session = new Session(write_io_thread_, this, kIOThreadFree, kNetworkTypeUDP, address);
    write_io_thread_->SetWriteEvent(fd, session);
    AddRemoteAddrSession(ss.str(), session);
    session->Register(io_event_, nullptr);
    session->OnConnect(fd);
  }
  session->OnRecvData(buf);
}

void UDPReadIOThread::ConnectOK(ConnQueueItem* item) {
  io_event_ = static_cast<IOEvent*>(item->io_event);
  write_io_thread_ = static_cast<UDPWriteIOThread*>(item->write_io_thread);
  std::stringstream ss;
  ss << item->address.remote_ip << ":" << item->address.remote_port;
  auto* session = GetRemoteAddrSession(ss.str());
  if (session == nullptr) {
    Address address = item->address;
    session = new Session(write_io_thread_, this, kIOThreadFree, kNetworkTypeUDP, address);
    write_io_thread_->SetWriteEvent(item->sfd, session);
    AddRemoteAddrSession(ss.str(), session);
    session->Register(io_event_, nullptr);
    session->OnConnectSuccess(item->sfd, nullptr);
  }
}

void UDPReadIOThread::OnRelease() {
  auto* item = new ConnQueueItem;
  item->del_self = true;
  if (Push(item) <= 1) {
    if (!SendNotify()) {
      ASSERTS_RETURN(false, "send failed. error = %s", strerror(errno));
    }
  }
}

}  // namespace cutio::net

