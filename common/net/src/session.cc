// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/net/src/session.h"

#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <utility>

#include "common/log/inc/log_controller.h"
#include "common/net/src/io_service.h"
#include "common/net/src/io_utils.h"
#include "common/net/src/read_io_thread.h"

namespace cutio::net {

Session::Session(BaseIOThread* write_io_thread, BaseIOThread* read_io_thread,
                 IOThreadType thread_type, NetworkType network_type, const Address& address)
    : read_event_{},
      write_event_{},
      read_thread_{read_io_thread},
      write_thread_{write_io_thread},
      io_event_{nullptr},
      filter_{nullptr},
      io_service_{nullptr},
      io_thread_type_{thread_type},
      socket_type_{kSocketTypeNull},
      network_type_{network_type},
      close_{false},
      send_close_handle_{false},
      send_error_{false},
      fd_(-1),
      address_{address} {}

Session::~Session() {
  delete io_service_;
  delete filter_;
  ClearWriteQueue();
  if (network_type_ == kNetworkTypeUDP) {
    IOUtils::CloseSocket(fd_);
  }
}

void Session::Register(IOEvent* io_event, NetFilter* filter) {
  io_event_ = io_event;
  filter_ = filter;
}

void Session::Write(const IOBuffer& buf, bool reliable) {
  if (close_ || send_close_handle_ || send_error_) {
    return;
  }

  IOBuffer new_buf{buf};
  if (filter_) {
    new_buf = filter_->FilterWrite(buf);
  }

  AddWriteQueue(new_buf);
}

Address Session::GetAddress() const {
  return address_;
}

void Session::OnRecvClose() {
  InfoMsg("OnRecvClose this = %p, %s:%u --> %s:%u", this,
          address_.local_ip.c_str(), address_.local_port, address_.remote_ip.c_str(), address_.remote_port);
  NotifyCloseToWriteThread();
}

void Session::OnConnect(int fd) {
  fd_ = fd;
  socket_type_ = kSocketTypeAccept;
  io_event_->OnIOEvent(this, kIOEventTypeOpenOK, IOBuffer{});
}

void Session::OnRecvData(const IOBuffer& buf) {
  if (send_close_handle_ || send_error_) {
    return;
  }
  if (filter_) {
    if (filter_->FilterRead(buf)) {
      while (true) {
        auto pkg = filter_->GetPackage();
        if (pkg.Empty()) {
          break;
        }
        OnIOEvent(kIOEventTypeReadOK, pkg);
      }
    }
  } else {
    OnIOEvent(kIOEventTypeReadOK, buf);
  }
}

std::string Session::QueryHandleInfo(QueryInfoType type) {
  switch (type) {
    case kQueryInfoTypeLocalIP:
      return address_.local_ip;
    case kQueryInfoTypeLocalPort:
      return std::to_string(address_.local_port);
    case kQueryInfoTypeRemoteIP:
      return address_.remote_ip;
    case kQueryInfoTYpeRemotePort:
      return std::to_string(address_.remote_port);
  }
}

NetworkType Session::QueryNetworkType() {
  return network_type_;
}

void Session::CloseHandle() {
  if (send_close_handle_) {
    return;
  }
  send_close_handle_ = true;
  InfoMsg("CloseHandle this = %p, %s:%u --> %s:%u", this,
          address_.local_ip.c_str(), address_.local_port, address_.remote_ip.c_str(), address_.remote_port);
  NotifyCloseToReadThread();
}

NetFilter* Session::GetNetFilter() {
  return filter_;
}

SocketType Session::GetSocketType() {
  return socket_type_;
}

void Session::SetSendBuf(uint32_t len) {
  IOBuffer buf(len);
  swap(buf_, buf);
}

uint32_t Session::PushWriteQueue(WriteQueueItem* item) {
  std::lock_guard lock(mu_);
  write_queue_.push(item);
  return static_cast<uint32_t>(write_queue_.size());
}

WriteQueueItem* Session::WriteQueueFront() {
  std::lock_guard lock(mu_);
  if (write_queue_.empty()) {
    return nullptr;
  }

  while (!write_queue_.empty()) {
    auto* item = write_queue_.front();
    if (item->success) {
      write_queue_.pop();
      delete item;
      continue;
    }
    return item;
  }
  return nullptr;
}

void Session::ClearWriteQueue() {
  std::lock_guard lock(mu_);
  while (!write_queue_.empty()) {
    auto* item = write_queue_.front();
    write_queue_.pop();
    delete item;
  }
}

uint32_t Session::WriteQueueSize() {
  return static_cast<uint32_t>(write_queue_.size());
}

void Session::OnConnectSuccess(int fd, IOService* io_service) {
  io_service_ = io_service;
  fd_ = fd;
  socket_type_ = kSocketTypeConnect;
  OnIOEvent(kIOEventTypeClientOpenOK, IOBuffer{});
}

void Session::OnWriteThreadClose() {
  InfoMsg("OnWriteThreadClose this = %p, %s:%u --> %s:%u", this,
          address_.local_ip.c_str(), address_.local_port, address_.remote_ip.c_str(), address_.remote_port);
  OnIOEvent(kIOEventTypeClosed, IOBuffer{});
  delete this;
}

void Session::OnWriteFailure() {
  OnIOEvent(kIOEventTypeWriteFailed, IOBuffer{});
}

void Session::OnWriteOK(IOBuffer buf) {
  // TODO(mn): use filter
  OnIOEvent(kIOEventTypeWriteOK, std::move(buf));
}

void Session::OnIOEvent(IOEventType io_event_type, const IOBuffer& buf) {
  if (io_thread_type_ == kIOThreadFree) {
    io_event_->OnIOEvent(this, io_event_type, buf);
  } else if (io_thread_type_ == kIOThreadAPP) {
    // TODO(mn): unimplemented
    ASSERTS_RETURN(false, "unimplemented")
  } else {
    ASSERTS_RETURN(false, "unreachable code");
  }
}

void Session::OnUDPSend(int fd, short which) {
  auto* item = WriteQueueFront();
  if (item == nullptr) {
    return;
  }

  ssize_t len = -1;
  if (socket_type_ == kSocketTypeAccept) {
    sockaddr_in dst_addr{};
    dst_addr.sin_family = AF_INET;
    dst_addr.sin_port = htons(address_.remote_port);
    if (inet_pton(AF_INET, address_.remote_ip.c_str(), &dst_addr.sin_addr.s_addr) != 1) {
      ErrorMsg("convert %s to IP failed. error = %s", address_.remote_ip.c_str(), strerror(errno));
      return;
    }
    len = sendto(fd, item->buf.GetData(), static_cast<size_t>(item->buf.Size()),
                 MSG_NOSIGNAL, reinterpret_cast<sockaddr*>(&dst_addr), sizeof(dst_addr));
  } else if (socket_type_ == kSocketTypeConnect) {
    len = send(fd, item->buf.GetData(), static_cast<size_t>(item->buf.Size()), MSG_NOSIGNAL);
  }

  if (len == -1) {
    if (errno == EINTR || errno == EAGAIN) {
      WarnMsg("send to %s:%u failed. error = %s",
              address_.remote_ip.c_str(), address_.remote_port, strerror(errno));
      event_add(&write_event_, nullptr);
      return;
    } else {
      ErrorMsg("send to %s:%u failed. error = %s",
               address_.remote_ip.c_str(), address_.remote_port, strerror(errno));
      return;
    }
  } else {
    OnWriteOK(item->buf);
    item->success = true;
  }
  event_add(&write_event_, nullptr);
}

void Session::OnTCPReceive(int fd, short which) {
  buf_.Reset();
  auto len = recv(fd, buf_.GetData(), buf_.Capacity(), 0);
  if (len <= 0) {
    bool error = false;
    if (len < 0) {
      DebugMsg("receive data error = %s", strerror(errno));
      if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
        ErrorMsg("receive data error = %s", strerror(errno));
        return;
      } else {
        error = true;
      }
    }

    if (len == 0 || error) {
      auto ret = dynamic_cast<ReadIOThread*>(read_thread_)->DelSocket(fd);
      if (ret) {
        event_del(&read_event_);
        OnRecvClose();
      }
    }
  } else {
    buf_.Produce(nullptr, static_cast<uint32_t>(len));
    OnRecvData(buf_);
  }
}

void Session::OnTCPSend(int fd, short which) {
  auto* item = WriteQueueFront();
  if (item == nullptr) {
    return;
  }

  if (send_error_) {
    return;
  }

  ssize_t len = send(fd, item->buf.GetData(), static_cast<size_t>(item->buf.Size()), MSG_NOSIGNAL);

  if (len == -1) {
    if (errno == EINTR || errno == EAGAIN) {
      WarnMsg("send to %s:%u failed. error = %s",
              address_.remote_ip.c_str(), address_.remote_port, strerror(errno));
      event_add(&write_event_, nullptr);
      return;
    } else {
      ErrorMsg("send to %s:%u failed. error = %s",
               address_.remote_ip.c_str(), address_.remote_port, strerror(errno));
      send_error_ = true;
      ClearWriteQueue();
    }
  } else if(item->buf.Size() > len) {
    item->buf.Consume(nullptr, static_cast<uint32_t>(len));
    event_add(&write_event_, nullptr);
  } else {
    item->buf.Consume(nullptr, static_cast<uint32_t>(len));
    OnWriteOK(item->buf);
    item->success = true;
  }
  event_add(&write_event_, nullptr);
}

void Session::OnReadProcessCallback(int fd, short which, void* arg) {

}
void Session::OnReadNotify(int fd, short which) {

}

void Session::AddWriteQueue(IOBuffer buf) {
  if (close_) {
    return;
  }

  auto* item = new WriteQueueItem;
  item->buf = std::move(buf);
  auto size = PushWriteQueue(item);
  if (size > 2) {
    return;
  }

  // Notify
  auto* notify = new NotifyWriteItem;
  notify->session = this;
  notify->del_self = false;
  size = write_thread_->Push(static_cast<void*>(notify));
  if (size <= 1) {
    if (!write_thread_->SendNotify()) {
      ASSERTS_RETURN(false, "send notify failed. error = %s", strerror(errno));
      ErrorMsg("send notify failed. error = %s", strerror(errno));
    }
  }
}

void Session::NotifyCloseToWriteThread() {
  if (close_) {
    return;
  }
  close_ = true;
  auto* item = new NotifyWriteItem;
  item->session = this;
  item->socket_status = kSocketStatusClose;
  item->del_self = false;
  auto size = write_thread_->Push(item);
  if (size <= 1) {
    if (write_thread_->SendNotify()) {
      ASSERTS_RETURN(false, "send notify failed. error = %s", strerror(errno));
      ErrorMsg("send notify failed. error = %s", strerror(errno));
    }
  }
}

void Session::NotifyCloseToReadThread() {
  auto* item = new ConnQueueItem;
  item->sfd = fd_;
  item->session = this;
  item->socket_status = kSocketStatusClose;
  item->del_self = false;
  auto size = read_thread_->Push(item);
  DebugMsg("NotifyCloseToReadThread size = %u", size);
  if (size <= 1) {
    if (!read_thread_->SendNotify()) {
      ASSERTS_RETURN(false, "send notify failed. error = %s", strerror(errno));
      ErrorMsg("send notify failed. error = %s", strerror(errno));
    }
  }
}

}  // namespace cutio::net
