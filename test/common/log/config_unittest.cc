// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/log/src/config.h"

#include "gtest/gtest.h"
#include "test/test_utils/global_env.h"

namespace cutio::log {

TEST(LogConfigParserTest, Parse) {
  auto input = test::GlobalEnv::Instance()->TestDataPath();
  input.append("log_config.yaml");

  LogConfigParser lcp(input);
  auto lc = lcp.GetConfig("server");
  EXPECT_EQ("server", lc.basename);
  EXPECT_EQ("server", lc.name);
  EXPECT_EQ(Level::kDebug, lc.level);
  EXPECT_EQ(1024, lc.max_size);
  EXPECT_EQ("log", lc.path);
}

}  // namespace cutio::log
