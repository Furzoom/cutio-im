// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/net/inc/mutex.h"

#include <cassert>

#include "common/thread_pool/inc/bedrock_debug.h"

namespace cutio::net {

// class Mutex.

Mutex::Mutex() : pid_{} {
  bedrock::debug::AbortingCall(pthread_mutex_init(&mu_, nullptr));
}

Mutex::~Mutex() {
  bedrock::debug::AbortingCall(pthread_mutex_destroy(&mu_));
}

void Mutex::Lock() {
  pthread_t pid = pthread_self();
  if (pid_ == pid) {
    assert(false);
  }
  pid_ = pid;
  bedrock::debug::AbortingCall(pthread_mutex_lock(&mu_));
}

void Mutex::Unlock() {
  pid_ = 0;
  bedrock::debug::AbortingCall(pthread_mutex_unlock(&mu_));
}

// class Lock.

Lock::Lock(Mutex &m) : mu_(m) {
  mu_.Lock();
}

Lock::~Lock() {
  mu_.Unlock();
}

// class RWMutex.

RWMutex::RWMutex() {
  bedrock::debug::AbortingCall(pthread_rwlock_init(&mu_, nullptr));
}

RWMutex::~RWMutex() {
  bedrock::debug::AbortingCall(pthread_rwlock_destroy(&mu_));
}

void RWMutex::RLock() {
  bedrock::debug::AbortingCall(pthread_rwlock_rdlock(&mu_));
}

void RWMutex::RUnlock() {
  bedrock::debug::AbortingCall(pthread_rwlock_unlock(&mu_));
}

void RWMutex::WLock() {
  bedrock::debug::AbortingCall(pthread_rwlock_wrlock(&mu_));
}

void RWMutex::WUnlock() {
  bedrock::debug::AbortingCall(pthread_rwlock_unlock(&mu_));
}

// class ReadLock.

ReadLock::ReadLock(RWMutex& m) : mu_(m) {
  mu_.RLock();
}

ReadLock::~ReadLock() {
  mu_.RUnlock();
}

// class WriteLock.

WriteLock::WriteLock(RWMutex& m) : mu_(m) {
  mu_.WLock();
}

WriteLock::~WriteLock() {
  mu_.WUnlock();
}

}  // namespace cutio::net
