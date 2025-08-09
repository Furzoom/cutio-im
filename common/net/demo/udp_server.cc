// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include <signal.h>

#include <cstdio>
#include <filesystem>
#include <iostream>

#include "common/log/inc/log_controller.h"
#include "common/net/inc/network_io.h"
#include "common/net/demo/udp_server_io_event.h"

namespace fs = std::filesystem;

using namespace cutio::log;
using namespace cutio::net;
using namespace cutio;

bool gExit = false;

static void cleanup() {
  delete LogController::GetInstance();
}

static void CatchSignal(int sig) {
  gExit = true;
}

static void SetUpSignal() {
  struct sigaction sa;
  sa.sa_handler = CatchSignal;
  sa.sa_mask = 0;
  sigemptyset(&sa.sa_flags);
  sigaction(SIGINT, &sa, nullptr);
}

int main(int argc, char* argv[]) {
  atexit(cleanup);
  SetUpSignal();

  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << "<log_config_file>" << std::endl;
    return -1;
  }
  auto cwd = fs::current_path();
  fs::path config = argv[1];
  if (config.is_relative()) {
    config = cwd;
    config.append(argv[1]);
  }

  LogController::Initialize(config.string(), "net-demo", "udp-server", cwd.append("test_tmp").string());

  bool ret = CreateServerIOService(kIOServiceTypeUDPServer, "127.0.0.1", 10001,
                                   new UDPServerIOEvent, nullptr, 1, kIOThreadFree, false);

  if (!ret) {
    printf("start failed\n");
  } else {
    printf("server started. listen at 127.0.0.1:10001\n");
  }

  while (!gExit) {
    usleep(100 * 1000);
  }

  return 0;
}