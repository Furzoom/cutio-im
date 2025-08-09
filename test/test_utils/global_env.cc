// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "global_env.h"

namespace cutio::test {

GlobalEnv *GlobalEnv::Instance() {
  static auto* inst_ = new GlobalEnv;
  return inst_;
}

GlobalEnv::GlobalEnv() : clean_clutter_(true) {}

GlobalEnv::~GlobalEnv() {}

void GlobalEnv::SetUp() {
  std::error_code err;
  base_path_ = fs::current_path(err);
  if (err) {
    fprintf(stderr, "create temp directory failed: %s\n", err.message().c_str());
  }
  base_path_.append("test_tmp");
  if (!fs::exists(base_path_)) {
    fs::create_directories(base_path_, err);
  }

  err.clear();
  test_data_path_ = fs::current_path(err);
  if (err) {
    fprintf(stderr, "get current work directory failed: %s\n", err.message().c_str());
  }
  test_data_path_.append("testdata");
}

void GlobalEnv::TearDown() {
  if (!clean_clutter_) {
    return;
  }

  std::error_code err;
  fs::remove_all(base_path_, err);
  if (err) {
    fprintf(stderr, "remove %s failed: %s\n", base_path_.c_str(), err.message().c_str());
  }
}

void GlobalEnv::SetCleanClutter(bool clean) {
  clean_clutter_ = clean;
}

fs::path GlobalEnv::BasePath() const {
  return base_path_;
}

fs::path GlobalEnv::TestDataPath() const {
  return test_data_path_;
}

}  // namespace cutio::test
