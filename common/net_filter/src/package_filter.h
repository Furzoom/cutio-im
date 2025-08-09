// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <deque>

#include "common/net/inc/io_buffer.h"
#include "common/net/inc/net_filter.h"

namespace cutio::filter {

class PackageFilter : public net::NetFilter {
 public:
  PackageFilter();
  ~PackageFilter() override = default;

  bool InitFilter(const net::IOBuffer& buf) override;
  bool FilterRead(const net::IOBuffer& buf) override;
  net::IOBuffer GetPackage() override;
  net::IOBuffer FilterWrite(net::IOBuffer package) override;
  uint32_t GetHeaderLen() const override;

 private:
  net::IOBuffer read_buf_;
  std::deque<net::IOBuffer> packages_;
  uint32_t waiting_len_;
};

}  // namespace cutio::filter
