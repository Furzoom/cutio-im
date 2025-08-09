// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include "inc/inc.h"
#include "common/net/inc/io_buffer.h"

namespace cutio::net {

class NetFilter {
 public:
  virtual ~NetFilter() = default;
  virtual bool InitFilter(const IOBuffer& buf) = 0;
  virtual bool FilterRead(const IOBuffer& buf) = 0;
  virtual IOBuffer GetPackage() = 0;
  virtual IOBuffer FilterWrite(IOBuffer package) = 0;
  virtual uint32_t GetHeaderLen() const = 0;
};

}  // namespace cutio::net
