// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "common/log/inc/log_controller.h"
#include "inc/inc.h"

namespace fs = std::filesystem;

namespace cutio::log {

class Config {
 public:
  std::string name;
  std::string basename;
  std::string path;
  Level level;
  uint32_t max_size;
};

struct LogConfig {
  std::vector<Config> configs;
  std::string title;
  std::string path;
};

class LogConfigParser {
 public:
  explicit LogConfigParser(const fs::path& filename);
  Config GetConfig(const std::string& name);

 private:
  LogConfig log_config_;
};

}  // namespace cutio::log

