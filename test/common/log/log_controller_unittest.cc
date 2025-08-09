// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/log/inc/log_controller.h"

#include <string>

#include "gtest/gtest.h"
#include "test/test_utils/global_env.h"

namespace cutio::log {

class LogControllerTest : public testing::Test {
 public:
  LogControllerTest() = default;

 protected:
  void SetUp() override {
    std::error_code err;
    fs::copy_file(test::GlobalEnv::Instance()->TestDataPath().append(log_file),
                  test::GlobalEnv::Instance()->BasePath().append(log_file),
                  fs::copy_options::overwrite_existing,
                  err);
    EXPECT_FALSE(err);
  }

 protected:
  std::string log_file = "log_config.yaml";
};

void exit_handler() {
  delete LogController::GetInstance();
}

TEST_F(LogControllerTest, Construct) {
  auto* log = LogController::Initialize("log_config.yaml", "server", "",
                                        test::GlobalEnv::Instance()->BasePath());
  ASSERT_NE(nullptr, log);
  EXPECT_FALSE(log->config_filepath_.empty());
  atexit(exit_handler);
}

TEST_F(LogControllerTest, FormatTime) {
  auto* log = new LogController("log_config.yaml", "session", "",
                                test::GlobalEnv::Instance()->BasePath());
  auto datetime = log->FormatTime(true);
  EXPECT_FALSE(datetime.empty());
  delete log;
}

TEST_F(LogControllerTest, GetConfig) {
  auto* log = new LogController("log_config.yaml", "session", "",
                                test::GlobalEnv::Instance()->BasePath());
  EXPECT_TRUE(log->GetConfig());
  delete log;
}

}  // namespace cutio::log