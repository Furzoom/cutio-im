// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <pthread.h>

namespace cutio::net {

class Mutex {
 public:
  Mutex();
  ~Mutex();

  void Lock();
  void Unlock();

 private:
  pthread_mutex_t& GetMutex() { return mu_; }

  pthread_mutex_t mu_;
  pthread_t pid_;
};

class Lock {
 public:
  explicit Lock(Mutex& m);
  ~Lock();

 private:
  Mutex& mu_;
};

class RWMutex {
 public:
  RWMutex();
  ~RWMutex();

  void RLock();
  void RUnlock();
  void WLock();
  void WUnlock();

 private:
  pthread_rwlock_t mu_;
};

class ReadLock {
 public:
  explicit ReadLock(RWMutex& m);
  ~ReadLock();

 private:
  RWMutex& mu_;
};

class WriteLock {
 public:
  explicit WriteLock(RWMutex& m);
  ~WriteLock();

 private:
  RWMutex& mu_;
};

}  // namespace cutio::net

