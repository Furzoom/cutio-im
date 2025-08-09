// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/net_filter/src/package_filter.h"

namespace cutio::filter {

const uint32_t kDefaultPackageSize = 2 << 13;
const uint32_t kPackageSizeMask = ~(kDefaultPackageSize - 1);

PackageFilter::PackageFilter()
    : read_buf_{0},
      waiting_len_{0} {}

bool PackageFilter::InitFilter(const net::IOBuffer& buf) {
  read_buf_ = net::IOBuffer{kDefaultPackageSize};
  return true;
}

bool PackageFilter::FilterRead(const net::IOBuffer& buf) {
  if (read_buf_.FreeSize() < buf.Size()) {
    read_buf_.Resize((read_buf_.Capacity() + buf.Size() + kDefaultPackageSize - 1) & kPackageSizeMask);
  }

  read_buf_.Produce(buf.GetData(), buf.Size());
  if (waiting_len_ > buf.Size()) {
    waiting_len_ -= buf.Size();
    return false;
  }

}

net::IOBuffer PackageFilter::GetPackage() {
  if (packages_.empty()) {
    return net::IOBuffer{};
  }

  auto pkg = packages_.front();
  packages_.pop_front();
  return pkg;
}
net::IOBuffer PackageFilter::FilterWrite(net::IOBuffer package) {
  return net::IOBuffer();
}
uint32_t PackageFilter::GetHeaderLen() const {
  return 0;
}

}  // namespace cutio::filter
