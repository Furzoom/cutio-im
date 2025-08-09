// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/net/inc/io_buffer.h"

namespace cutio::net {

class BufImpl {
 public:
  BufImpl() : buf_{nullptr} {}
  BufImpl(const BufImpl&) = delete;
  BufImpl& operator=(BufImpl) = delete;

  ~BufImpl() { delete[] buf_; }

  bool InitBuf(uint32_t capacity) {
    if (buf_) {
      return false;
    }
    buf_ = new char[capacity];
    return true;
  }

  char* GetData() const { return buf_; }

 private:
  char* buf_;
};

IOBuffer::IOBuffer()
    : begin_{0}, end_{0}, capacity_{0} {
  buf_ = std::make_shared<BufImpl>();
  buf_->InitBuf(capacity_);
}

IOBuffer::IOBuffer(const IOBuffer& other)
    : begin_{other.begin_},
      end_{other.end_},
      capacity_{other.capacity_},
      buf_{other.buf_} {}

IOBuffer::IOBuffer(IOBuffer&& other) noexcept
    : begin_{0},
      end_{0},
      capacity_{0} {
  swap(*this, other);
}

IOBuffer::IOBuffer(uint32_t capacity)
    : begin_{0}, end_{0}, capacity_{0} {
  buf_ = std::make_shared<BufImpl>();
  if (!buf_->InitBuf(capacity)) {
    return;
  }
  capacity_ = capacity;
}

IOBuffer::IOBuffer(const char* buf, uint32_t length)
  : begin_{0}, end_{0}, capacity_{0} {
  buf_ = std::make_shared<BufImpl>();
  if (!buf_->InitBuf(length)) {
    return;
  }

  memcpy(buf_->GetData(), buf, length);

  end_ = length;
  capacity_ = length;
}

IOBuffer::~IOBuffer() {}

IOBuffer& IOBuffer::operator=(IOBuffer other) {
  swap(*this, other);
  return *this;
}

bool IOBuffer::Empty() const {
  return begin_ == end_;
}

uint32_t IOBuffer::Size() const {
  return end_ - begin_;
}

uint32_t IOBuffer::Capacity() const {
  return capacity_;
}

uint32_t IOBuffer::FreeSize() const {
  return capacity_ - end_;
}

void* IOBuffer::GetData() const {
  return buf_->GetData() + begin_;
}

void* IOBuffer::GetRawData() {
  return buf_->GetData();
}

void IOBuffer::Reset() {
  begin_ = 0;
  end_ = 0;
}

void IOBuffer::Resize(uint32_t capacity) {
  if (capacity == capacity_) {
    return;
  }
  if (capacity < capacity_) {
    if (begin_ != 0) {
      memmove(buf_->GetData(), buf_->GetData() + begin_, Size());
      end_ -= begin_;
      begin_ = 0;
    }
    capacity_ = capacity;
    end_ = end_ < capacity_ ? end_ : capacity_;
    return;
  }

  auto new_buf = std::make_shared<BufImpl>();
  new_buf->InitBuf(capacity);
  memcpy(new_buf->GetData(), GetData(), Size());

  std::swap(buf_, new_buf);
  capacity_ = capacity;
  end_ -= begin_;
  begin_ = 0;
}

std::string IOBuffer::ToString() const {
  return std::string{buf_->GetData() + begin_, Size()};
}

bool IOBuffer::Consume(void* buf, uint32_t size) {
  if (size > Size()) {
    return false;
  }

  memmove(buf, GetData(), size);
  begin_ += size;
  return true;
}

bool IOBuffer::Produce(const void* buf, uint32_t size) {
  if (size > FreeSize()) {
    return false;
  }

  if (buf != nullptr) {
    memcpy(GetData(), buf, size);
  }

  end_ += size;
  return true;
}

void swap(IOBuffer& lhs, IOBuffer& rhs) {
  std::swap(lhs.buf_, rhs.buf_);
  std::swap(lhs.begin_, rhs.begin_);
  std::swap(lhs.end_, rhs.end_);
  std::swap(lhs.capacity_, rhs.capacity_);
}

}  // namespace cutio::net
