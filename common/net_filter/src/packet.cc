// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/net_filter/src/packet.h"

namespace cutio::filter {
namespace {

union BaseEndian {
  uint16_t n;
  char c[2];
};

static union BaseEndian b{0x0102};

static bool BigEndian() {
  return b.c[0] == 1;
}

template<typename T, std::size_t size = sizeof(T), std::enable_if_t<size == 2, int> = 0>
T hton(T v) {
  if (BigEndian()) {
    return v;
  }
  return (v & 0xff00) >> 8 | (v & 0x00ff) << 8;
}

template<typename T, std::size_t size = sizeof(T), std::enable_if_t<size == 4, int> = 0>
T hton(T v) {
  if (BigEndian()) {
    return v;
  }
  return (v & 0xff000000) >> 24 |
         (v & 0x00ff0000) >> 8 |
         (v & 0x0000ff00) << 8 |
         (v & 0x000000ff) << 24;
}

template<typename T, std::size_t size = sizeof(T), std::enable_if_t<size == 8, int> = 0>
T hton(T v) {
  if (BigEndian()) {
    return v;
  }
  return (v & 0xff00000000000000ull) >> 56 |
         (v & 0x00ff000000000000ull) >> 40 |
         (v & 0x0000ff0000000000ull) >> 24 |
         (v & 0x000000ff00000000ull) >> 8 |
         (v & 0x00000000ff000000ull) << 8 |
         (v & 0x0000000000ff0000ull) << 24 |
         (v & 0x000000000000ff00ull) << 40 |
         (v & 0x00000000000000ffull) << 56;
}

}  // anonymous namespace

Packet::Packet(const net::IOBuffer& buffer)
  : buffer_{buffer},
    good_{false},
    offset_{0},
    buffer_size_{buffer_.Size()} {
  if (buffer_size_ == 0) {
    buffer_size_ = buffer_.Capacity();
  }
}

Packet::~Packet() {}

Packet& Packet::operator>>(uint8_t& value) {
  return *this;
}
Packet& Packet::operator<<(const uint8_t& value) {
  return *this;
}
Packet& Packet::operator>>(uint16_t& value) {
  return *this;
}
Packet& Packet::operator<<(const uint16_t& value) {
  return *this;
}
Packet& Packet::operator>>(uint32_t& value) {
  return *this;
}
Packet& Packet::operator<<(const uint32_t& value) {
  return *this;
}
Packet& Packet::operator>>(uint64_t& value) {
  return *this;
}
Packet& Packet::operator<<(const uint64_t& value) {
  return *this;
}
Packet& Packet::operator>>(int8_t& value) {
  return *this;
}
Packet& Packet::operator<<(const int8_t& value) {
  return *this;
}
Packet& Packet::operator>>(int16_t& value) {
  return *this;
}
Packet& Packet::operator<<(const int16_t& value) {
  return *this;
}
Packet& Packet::operator>>(int32_t& value) {
  return *this;
}
Packet& Packet::operator<<(const int32_t& value) {
  return *this;
}
Packet& Packet::operator>>(int64_t& value) {
  return *this;
}
Packet& Packet::operator<<(const int64_t& value) {
  return *this;
}
bool Packet::readBuffer(void* data, int32_t size) {
  return false;
}
bool Packet::writeBuffer(const void* data, int32_t size) {
  return false;
}
bool Packet::addOff(int32_t size) {
  return false;
}
bool Packet::Good() {
  return false;
}
bool Packet::Reset() {
  return false;
}
int32_t Packet::Size() {
  return 0;
}
void* Packet::GetBuf() {
  return nullptr;
}
uint8_t Packet::GetVersion() const {
  return 0;
}
void Packet::SetVersion(uint8_t version) {

}
void Packet::SetBadPacket() {

}
}  // namespace cutio::filter
