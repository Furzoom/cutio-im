// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/net/src/base_io_thread.h"

#include <event2/event.h>

namespace cutio::net {

BaseIOThread::BaseIOThread() : base_(nullptr), notify_event_(nullptr) {}

BaseIOThread::~BaseIOThread() {}

void BaseIOThread::ExitEventLoop() {
  event_del(notify_event_);
  event_base_loopexit(base_, nullptr);
}

void BaseIOThread::InitPipeEvent(event_callback_fn callback, void* arg) {
  base_ = event_base_new();
  assert(base_ != nullptr);
  notify_event_ = event_new(base_, notify_receive_fd_, EV_READ | EV_PERSIST, callback, arg);
  assert(notify_event_ != nullptr);
  assert(event_add(notify_event_, nullptr) == 0);
}

void BaseIOThread::OnThreadRun() {
  event_base_dispatch(base_);
  event_free(notify_event_);
  event_base_free(base_);
}

std::string BaseIOThread::GetThreadName() {
  return thread_name_;
}

bool BaseIOThread::SendNotify() {
  return (write(notify_send_fd_, "", 1) == 1);
}

}  // namespace cutio::net
