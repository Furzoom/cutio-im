// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/net/src/io_service.h"

#include "common/log/inc/log_controller.h"

namespace cutio::net {

IOService::IOService()
    : event_base_(nullptr),
      success_(true),
      count_(0) {
  pthread_mutex_init(&mutex_, nullptr);
  pthread_cond_init(&cond_, nullptr);
}

IOService::~IOService() {
  pthread_mutex_destroy(&mutex_);
  pthread_cond_destroy(&cond_);

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
  pthread_t tid;
  if (pthread_create(&tid, nullptr, ThreadFunc, this) != 0) {
    ASSERTS_RETURN(false, "create thread failed, error: %s", strerror(errno));
  }

  pthread_detach(tid);
}

void* IOService::ThreadFunc(void* arg) {
  return nullptr;
}
void IOService::OnTCPAcceptCallback(int fd, short which, void* arg) {

}
void IOService::OnUDPAcceptCallback(int fd, short which, void* arg) {

}
void IOService::DispatchNewCon(int nfd,
                               SocketType socket_type,
                               NetworkType network_type,
                               const std::string& ip,
                               const std::string& port) {

}

}  // namespace cutio::net