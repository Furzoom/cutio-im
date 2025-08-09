// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/net/src/io_thread.h"

#include <pthread.h>

#include "common/net/src/io_utils.h"
#include "common/net/src/net_type.h"

namespace cutio::net {

IOThread::IOThread() {
  int fds[2];
  IOUtils::CreatePipe(fds);
  notify_receive_fd_ = fds[0];
  notify_send_fd_ = fds[1];
}

IOThread::~IOThread() {
  IOUtils::CloseSocket(notify_receive_fd_);
  IOUtils::CloseSocket(notify_send_fd_);
}

int IOThread::Push(void* arg) {
  Lock lock(mu_);
  queue_.push(arg);
  return static_cast<int>(queue_.size());
}

Lock* IOThread::Push(void* arg, int* size) {
  Lock* lock = new Lock(mu_);
  queue_.push(arg);
  *size = static_cast<int>(queue_.size());
  return lock;
}

void* IOThread::Pop() {
  Lock lock(mu_);
  if (queue_.empty()) {
    return nullptr;
  }
  void *p = queue_.front();
  queue_.pop();
  return p;
}

void IOThread::ClearQueue() {
  Lock lock(mu_);
  while (!queue_.empty()) {
    void* p = queue_.front();
    queue_.pop();
    auto* c = static_cast<ConnQueueItem*>(p);
    delete c;
  }
}

void* IOThread::ThreadFun(void* arg) {
  auto* p = static_cast<IOThread*>(arg);
  p->OnThreadRun();
  return nullptr;
}

void IOThread::StartThread() {
  pthread_t tid;
  if (pthread_create(&tid, nullptr, ThreadFun, this) != 0) {
    assert(0);
  }

  pthread_detach(tid);
}

void IOThread::StopThread() {
  ClearQueue();
  OnRelease();
}

}  // namespace cutio::net