// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/net/src/io_service.h"

#include <thread>

#include "common/log/inc/log_controller.h"
#include "common/net/src/io_utils.h"
#include "common/net/src/read_io_thread.h"
#include "common/net/src/udp_read_io_thread.h"
#include "common/net/src/udp_write_io_thread.h"
#include "common/net/src/write_io_thread.h"

namespace cutio::net {

IOService::IOService()
    : event_base_(nullptr),
      success_(true),
      count_(0) {
}

IOService::~IOService() {
  if (event_base_) {
    event_del(&accept_event_);
  }

  for (auto* reader : read_io_threads_) {
    reader->StopThread();
  }
  read_io_threads_.clear();

  for (auto* writer : write_io_threads_) {
    writer->StopThread();
  }
  write_io_threads_.clear();
}

void IOService::StartThread() {
  std::thread t(ThreadFunc, this);
  t.detach();
}

void* IOService::ThreadFunc(void* arg) {
  auto* self = static_cast<IOService*>(arg);
  return self->Run();
}

void* IOService::Run() {
  std::unique_lock lock(mutex_);
  bool ret = false;
  int fd = -1;
  switch (start_up_arg_.service_type) {
    case kIOServiceTypeUnknown:
      break;
    case kIOServiceTypeTCPServer:
      ret = IOUtils::TCPListen(start_up_arg_.address.local_ip, start_up_arg_.address.local_port, &fd);
      break;
    case kIOServiceTypeTCPClient:
      ret = IOUtils::TCPConnect(start_up_arg_.address.remote_ip, start_up_arg_.address.remote_port, &fd,
                                &start_up_arg_.address.local_ip, &start_up_arg_.address.local_port);
      break;
    case kIOServiceTypeUDPServer:
      ret = IOUtils::UDPListen(start_up_arg_.address.local_ip, start_up_arg_.address.local_port, &fd);
      break;
    case kIOServiceTypeUDPClient:
      ret = IOUtils::UDPConnect(start_up_arg_.address.remote_ip, start_up_arg_.address.remote_port, &fd,
                                &start_up_arg_.address.local_ip, &start_up_arg_.address.local_port);
      break;
    case kIOServiceTypeUDXServer:
      ret = IOUtils::UDXListen(start_up_arg_.address.local_port, start_up_arg_.thread_num, start_up_arg_.io_event);

      success_ = ret;
      lock.unlock();
      cond_.notify_one();

      return nullptr;
    case kIOServiceTypeUDXClient:
      ret = IOUtils::UDXConnect(start_up_arg_.address.remote_ip, start_up_arg_.address.remote_port,
                                start_up_arg_.address.local_port, start_up_arg_.io_event, this, start_up_arg_.sync);
      success_ = ret;
      lock.unlock();
      cond_.notify_one();

      return nullptr;
    case kIOServiceTypeUDXP2PClient:
      ret = IOUtils::UDXP2PCreate(start_up_arg_.address.local_port, start_up_arg_.io_event);

      lock.unlock();
      cond_.notify_one();

      return nullptr;
    default:
      WarnMsg("Unknown service type %d", start_up_arg_.service_type);
      break;
  }

  success_ = ret;
  bool sync = start_up_arg_.sync;
  lock.unlock();
  cond_.notify_one();

  if (!success_) {
    if (!sync) {
      start_up_arg_.io_event->OnIOEvent(nullptr, kIOEventTypeOpenFailed, IOBuffer{});
      delete this;
    }
    return nullptr;
  }

  for (int i = 0; i < start_up_arg_.thread_num; i++) {
    BaseIOThread* r = nullptr;
    BaseIOThread* w = nullptr;
    if (start_up_arg_.service_type == kIOServiceTypeUDPServer ||
        start_up_arg_.service_type == kIOServiceTypeUDPClient) {
      r = new UDPReadIOThread;
      w = new UDPWriteIOThread;
    } else {
      r = new ReadIOThread;
      w = new WriteIOThread;
    }

    r->StartThread();
    read_io_threads_.push_back(r);
    w->StartThread();
    write_io_threads_.push_back(w);
  }

  NetworkType network_type{kNetworkTypeNull};
  switch (start_up_arg_.service_type) {
    case kIOServiceTypeTCPServer:
    case kIOServiceTypeUDPServer:
      start_up_arg_.io_event->OnIOEvent(nullptr, kIOEventTypeInit, IOBuffer{});
      break;
    case kIOServiceTypeTCPClient:
      network_type = kNetworkTypeNull;
      [[fallthrough]];
    case kIOServiceTypeUDPClient:
      network_type = kNetworkTypeUDP;
      DispatchNewCon(fd, kSocketTypeConnect, network_type,
                     start_up_arg_.address.local_ip, start_up_arg_.address.local_port);
      break;
    default:
      break;
  }

  if (start_up_arg_.service_type == kIOServiceTypeTCPServer) {
    event_base_ = event_base_new();
    if (event_base_ == nullptr) {
      FatalMsg("create base event failed")
    }
    int r = event_assign(&accept_event_, event_base_, fd, EV_READ | EV_PERSIST,
                         OnTCPAcceptCallback, this);
    if (r != 0) {
      ASSERTS_RETURN_NULL(false, "event assign with invalid arguments");
    }
    if (event_add(&accept_event_, nullptr) != 0) {
      ASSERTS_RETURN_NULL(false, "add event failed");
    }
    event_base_dispatch(event_base_);
    event_base_free(event_base_);
  } else if (start_up_arg_.service_type == kIOServiceTypeUDPServer) {
    DispatchNewCon(fd, kSocketTypeAccept, kNetworkTypeUDP, "", 0);
  }

  return nullptr;
}

void IOService::OnTCPAcceptCallback(int fd, short which, void* arg) {
  std::string addr;
  uint16_t port;
  int cfd = IOUtils::Accept(fd, &addr, &port);
  if (cfd < 0) {
    ErrorMsg("accept new connection from fd(%d) failed", fd);
    return;
  }
  auto self = static_cast<IOService*>(arg);
  self->DispatchNewCon(cfd, kSocketTypeAccept, kNetworkTypeTCP, addr, port);
}

void IOService::OnUDPAcceptCallback(int fd, short which, void* arg) {
  InfoMsg("accept a UDP");
}

void IOService::DispatchNewCon(int nfd, SocketType socket_type, NetworkType network_type,
                               const std::string& ip, uint16_t port) {
  auto* item = new ConnQueueItem;
  item->io_event = start_up_arg_.io_event;
  item->filter = start_up_arg_.filter;
  item->write_io_thread = write_io_threads_[count_];
  item->sfd = nfd;
  item->socket_type = socket_type;
  item->socket_status = kSocketStatusConnect;
  item->io_thread_type = start_up_arg_.thread_type;
  item->del_self = false;
  item->network_type = network_type;

  if (socket_type == kSocketTypeConnect) {
    start_up_arg_.address.local_ip = ip;
    start_up_arg_.address.local_port = port;
    item->address = start_up_arg_.address;
    item->io_service = this;

    InfoMsg("create new connection %s:%u --> %s:%u",
            start_up_arg_.address.local_ip.c_str(), start_up_arg_.address.local_port,
            start_up_arg_.address.remote_ip.c_str(), start_up_arg_.address.remote_port);
  } else if (socket_type == kSocketTypeAccept) {
    start_up_arg_.address.remote_ip = ip;
    start_up_arg_.address.remote_port = port;
    item->address = start_up_arg_.address;

    InfoMsg("accept new connection %s:%u <-- %s:%u",
            start_up_arg_.address.local_ip.c_str(), start_up_arg_.address.local_port,
            start_up_arg_.address.remote_ip.c_str(), start_up_arg_.address.remote_port);
  }

  auto* read_thread = read_io_threads_[count_];
  auto size = read_thread->Push(item);
  if (size <= 1) {
    if (!read_thread->SendNotify()) {
      ErrorMsg("notify send failed. error = %s", strerror(errno));
    }
  }

  count_++;
  if (static_cast<int>(count_) == start_up_arg_.thread_num) {
    count_ = 0;
  }
}

bool IOService::StartUp(IOServiceType service_type, const Address& address, IOEvent* io_event,
                        NetFilter* fileter, int thread_num, IOThreadType io_thread_type, bool sync) {
  start_up_arg_.address = address;
  start_up_arg_.thread_type = io_thread_type;
  start_up_arg_.thread_num = thread_num;
  start_up_arg_.sync = sync;
  start_up_arg_.io_event = io_event;
  start_up_arg_.service_type = service_type;
  start_up_arg_.filter = fileter;

  std::unique_lock lock(mutex_);
  StartThread();
  if (sync) {
    cond_.wait(lock);
  }

  return success_;
}

}  // namespace cutio::net