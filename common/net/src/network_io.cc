// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/net/inc/network_io.h"

#include "common/net/src/io_service.h"

namespace cutio::net {

bool CreateServerIOService(IOServiceType service_type, const std::string& local_ip, uint16_t local_port,
                           IOEvent* io_event, NetFilter* filter, int thread_num, IOThreadType thread_type, bool sync) {
  auto* service = new IOService;
  Address address("", 0, local_ip, local_port);
  bool ret = service->StartUp(service_type, address, io_event, filter, thread_num, thread_type, sync);
  if (!ret && sync) {
    delete service;
  }
  return ret;
}

bool CreateClientIOService(IOServiceType service_type, const std::string& local_ip, uint16_t local_port,
                           const std::string& remote_ip, uint16_t remote_port, IOEvent* io_event,
                           NetFilter* filter, int thread_num, IOThreadType thread_type, bool sync) {
  auto* service = new IOService;
  Address address(remote_ip, remote_port, local_ip, local_port);
  bool ret = service->StartUp(service_type, address, io_event, filter, thread_num, thread_type, sync);
  if (!ret && sync) {
    delete service;
  }
  return ret;
}

}  // namespace cutio::net
