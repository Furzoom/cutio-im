// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/net/src/io_thread.h"

#include <pthread.h>

#include "common/net/src/io_utils.h"
#include "common/net/src/net_type.h"

namespace cutio::net {

static void SetThreadName(pthread_t pid, const char* name) {
#if defined(__GLIBC__) && ((__GLIBC__ > 2) || ((__GLIBC__ == 2) && (__GLIBC_MINOR__ >= 12)))
  pthread_setname_np(pid, name);
#elif OS_APPLE
  pthread_setname_np(name);
#endif
}

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

uint32_t IOThread::Push(void* arg) {
  std::lock_guard lock(mu_);
  queue_.push(arg);
  return static_cast<uint32_t>(queue_.size());
}

std::unique_lock<std::mutex> IOThread::Push(void* arg, int* size) {
  std::unique_lock lock(mu_);
  queue_.push(arg);
  *size = static_cast<int>(queue_.size());
  return lock;
}

void* IOThread::Pop() {
  std::lock_guard lock(mu_);
  if (queue_.empty()) {
    return nullptr;
  }
  void *p = queue_.front();
  queue_.pop();
  return p;
}

void IOThread::ClearQueue() {
  std::lock_guard lock(mu_);
  while (!queue_.empty()) {
    void* p = queue_.front();
    queue_.pop();
    auto* c = static_cast<ConnQueueItem*>(p);
    delete c;
  }
}

void* IOThread::ThreadFun(void* arg) {
  auto* p = static_cast<IOThread*>(arg);
  SetThreadName(nullptr, p->GetThreadName().c_str());
  p->OnThreadRun();
  return nullptr;
}

void IOThread::StartThread() {
  pthread_t tid;
  if (pthread_create(&tid, nullptr, ThreadFun, this) != 0) {
    assert(0);
  }

  SetThreadName(tid, GetThreadName().c_str());

  pthread_detach(tid);
}

void IOThread::StopThread() {
  ClearQueue();
  OnRelease();
}

}  // namespace cutio::net