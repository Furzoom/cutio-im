// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <memory>

namespace cutio {
namespace net {

class NetFilter;

}  // namespace net

namespace filter {

enum NetFilterType {
  kNetFilterNull = 0,
  kNetFilterPkgFilter = 1,
};

std::shared_ptr<net::NetFilter> CreateNetFilter(NetFilterType net_filter_type);

}  // namespace filter
}  // namespace cutio
