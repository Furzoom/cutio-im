// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/net_filter/inc/net_filter_factory.h"

#include "common/log/inc/log_controller.h"
#include "common/net_filter/src/package_filter.h"

namespace cutio::filter {

std::shared_ptr<net::NetFilter> CreateNetFilter(NetFilterType net_filter_type) {
  switch (net_filter_type) {
    case kNetFilterNull:
      return nullptr;
    case kNetFilterPkgFilter:
      return std::make_shared<PackageFilter>();
    default:
      ASSERTS_RETURN_NULL(false, "unimplemented net filter");
      return nullptr;
  }
}

}  // namespace cutio::filter
