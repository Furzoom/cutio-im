// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <memory>
#include <string>
#include <utility>

#include "inc/inc.h"

namespace cutio::net {

class BufImpl;

class IOBuffer {
 public:
  IOBuffer();
  IOBuffer(const IOBuffer& other);
  IOBuffer(IOBuffer&& other) noexcept ;
  explicit IOBuffer(uint32_t capacity);
  IOBuffer(const char* buf, uint32_t length);
  ~IOBuffer();

  IOBuffer& operator=(IOBuffer other);

  bool Empty() const;
  uint32_t Size() const;
  uint32_t Capacity() const;
  uint32_t FreeSize() const;
  void* GetData() const;
  void* GetRawData();
  void Reset();
  void Resize(uint32_t capacity);

  std::string ToString() const;

  bool Consume(void* buf, uint32_t size);
  bool Produce(const void* buf, uint32_t size);

  friend void swap(IOBuffer& lhs, IOBuffer& rhs);

 private:


 private:
  uint32_t begin_;
  uint32_t end_;
  uint32_t capacity_;
  std::shared_ptr<BufImpl> buf_;
};

}  // namespace cutio::net
