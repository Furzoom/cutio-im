// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <cinttypes>

#include "common/net/inc/io_buffer.h"

namespace cutio::filter {

class Packet {
   public:
    explicit Packet(const net::IOBuffer& buffer);
    ~Packet();

   public:
    Packet& operator>>(uint8_t& value);
    Packet& operator<<(const uint8_t& value);

    Packet& operator>>(uint16_t& value);
    Packet& operator<<(const uint16_t& value);

    Packet& operator>>(uint32_t& value);
    Packet& operator<<(const uint32_t& value);

    Packet& operator>>(uint64_t& value);
    Packet& operator<<(const uint64_t& value);

    Packet& operator>>(int8_t& value);
    Packet& operator<<(const int8_t& value);

    Packet& operator>>(int16_t& value);
    Packet& operator<<(const int16_t& value);

    Packet& operator>>(int32_t& value);
    Packet& operator<<(const int32_t& value);

    Packet& operator>>(int64_t& value);
    Packet& operator<<(const int64_t& value);

    bool readBuffer(void* data, int32_t size);
    bool writeBuffer(const void* data, int32_t size);

    bool addOff(int32_t size);

    // check
    bool Good();
    bool Reset();

    // size
    int32_t Size();
    void* GetBuf();

    // following functions used for decoding / encoding different version protocol
    uint8_t GetVersion() const;
    void SetVersion(uint8_t version);
    void SetBadPacket();

   private:
    net::IOBuffer buffer_;
    bool good_;

    uint32_t offset_;
    uint32_t buffer_size_;
};

}  // namespace cutio::filter
