// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include "gtest/gtest.h"

#include <filesystem>

namespace fs = std::filesystem;

namespace cutio::test {

class GlobalEnv : public testing::Environment {
 public:
  static GlobalEnv* Instance();

  GlobalEnv(const GlobalEnv&) = delete;
  ~GlobalEnv() override;

  GlobalEnv& operator=(const GlobalEnv&) = delete;

  void SetUp() override;
  void TearDown() override;

  void SetCleanClutter(bool clean);

  fs::path BasePath() const;
  fs::path TestDataPath() const;

 private:
  GlobalEnv();

 private:
  bool clean_clutter_;
  fs::path base_path_;
  fs::path test_data_path_;
};

}  // namespace cutio::test
