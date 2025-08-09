// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/log/src/config.h"

#include "yaml-cpp/yaml.h"

namespace YAML {

namespace lc = cutio::log;
typedef lc::Config LCConfig;
typedef lc::LogConfig LCLogConfig;

template<>
struct convert<LCConfig> {
  static Node encode(const LCConfig& rhs) {
    Node node;
    node["name"] = rhs.name;
    node["basename"] = rhs.basename;
    node["size"] = rhs.max_size;
    node["level"] = static_cast<int>(rhs.level);
    return node;
  }

  static bool decode(const Node& node, LCConfig& rhs) {
    if (!node.IsMap()) {
      return false;
    }

    if (node["name"] && node["name"].IsScalar()) {
      rhs.name = node["name"].as<std::string>();
    }
    if (node["basename"] && node["basename"].IsScalar()) {
      rhs.basename = node["basename"].as<std::string>();
    }
    if (node["size"] && node["size"].IsScalar()) {
      rhs.max_size = node["size"].as<uint32_t>();
    }
    if (node["level"] && node["level"].IsScalar()) {
      rhs.level = static_cast<cutio::log::Level>(node["level"].as<int>());
    }

    return true;
  }
};

template<>
struct convert<LCLogConfig> {
  static Node encode(const LCLogConfig& rhs) {
    Node seq;
    for (const auto& c : rhs.configs) {
      Node n = convert<LCConfig>::encode(c);
      seq.push_back(n);
    }
    Node node;
    node[rhs.title] = seq;
    return node;
  }

  static bool decode(const Node& node, LCLogConfig& rhs) {
    if (!node.IsMap()) {
      return false;
    }
    if (!node["log"] || !node["log"].IsSequence()) {
      return false;
    }
    auto& seq = node["log"];
    rhs.title = "log";
    for (const auto& s : seq) {
      LCConfig c;
      if (s["path"] && s["path"].IsScalar()) {
        rhs.path = s["path"].as<std::string>();
        continue;
      } else if (convert<LCConfig>::decode(s, c)) {
        rhs.configs.push_back(c);
        continue;
      }
    }
    for (size_t i = 0; i < rhs.configs.size(); i++) {
      rhs.configs[i].path = rhs.path;
    }
    return true;
  }
};

}  // namespace YAML

namespace cutio::log {

LogConfigParser::LogConfigParser(const fs::path& filename) {
  auto node = YAML::LoadFile(filename.string());
  if (!node) {
    fprintf(stderr, "load file '%s' failed\n", filename.c_str());
    return;
  }

  log_config_ = node.as<LogConfig>();
}

Config LogConfigParser::GetConfig(const std::string& name) {
  for (const auto& c : log_config_.configs) {
    if (c.name == name) {
      return c;
    }
  }
  return Config{};
}

}  // namespace cutio::log
