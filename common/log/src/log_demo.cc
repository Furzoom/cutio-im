// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/log/inc/log_controller.h"

#include <filesystem>
#include <iostream>

namespace l = cutio::log;
namespace fs = std::filesystem;

void cleanup() {
  delete l::LogController::GetInstance();
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << " <log_config_file>" << std::endl;
    return -1;
  }
  auto cwd = fs::current_path();
  fs::path config = argv[1];
  if (config.is_relative()) {
    config = cwd;
    config.append(argv[1]);
  }

  l::LogController::Initialize(config.string(), "log-demo", "test",
                               cwd.append("test_tmp").append("log").string());
  int n = 0;
  while (n < 300) {
    DebugMsg("debug message. times %d", n);
    n++;
    InfoMsg("info message. times %d", n);
    n++;
    WarnMsg("warning message. times %d", n);
    n++;
    ErrorMsg("error message. times %d", n)
    n++;
    DebugSend("send a message. times %d", n);
    n++;
    DebugRecv("recv a message. times %d", n);
    n++;
  }

  FatalMsg("fatal message. times %d", n)

  atexit(cleanup);
  return 0;
}
