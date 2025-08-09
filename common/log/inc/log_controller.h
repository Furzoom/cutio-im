// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#pragma once

#include <cstdio>
#include <filesystem>
#include <mutex>
#include <queue>

#include "gtest/gtest_prod.h"
#include "inc/inc.h"

namespace fs = std::filesystem;

namespace cutio::log {

enum class Level {
  kClose = 0,
  kFatal = 1,
  kError = 2,
  kWarn = 3,
  kInfo = 4,
  kDebug = 5,
};

const int kLineLength = 256;
const int kBufferLength = 2048;

const char* const kLogTypeMsgRecv = "MSG-RECV";
const char* const kLogTypeMsgSend = "MSG-SEND";
const char* const kLogTypeNotify = "NOTIFY";

#if OS_ANDROID
# define _LOG_MSG(level, type, msg, ...)
#elif OS_IOS
# define _LOG_MSG(level, type, msg, ...)
#elif OS_LINUX || OS_MAC
# define _LOG_MSG(level, type, msg, ...)                                                      \
    if (level <= cutio::log::LogController::GetInstance()->GetLevel()) {                      \
      char log_line[cutio::log::kLineLength] = {0};                                           \
      snprintf(log_line, sizeof(log_line), "%s:%d|%s()|",                                     \
      strrchr(__FILE__, '/') + 1, __LINE__, __FUNCTION__);                                    \
      char log_buf[cutio::log::kBufferLength] = {0};                                          \
      snprintf(log_buf, sizeof(log_buf), msg, __VA_ARGS__);                                   \
      std::string msg_str(log_line);                                                          \
      msg_str += log_buf;                                                                     \
      cutio::log::LogController::GetInstance()->Log(level, type, msg_str.c_str());            \
    }

#else
#error "unsupported OS"
#endif

# define ASSERTS(c, msg, ...) {                                                               \
    if (!(c)) {                                                                               \
      _LOG_MSG(cutio::log::Level::kError, cutio::log::kLogTypeNotify, msg, __VA_ARGS__);      \
      assert(0);                                                                              \
    }                                                                                         \
  }

# define ASSERTS_RETURN(c, msg, ...) { \
    if (!(c)) {                                                                               \
      _LOG_MSG(cutio::log::Level::kError, cutio::log::kLogTypeNotify, msg, __VA_ARGS__);      \
      assert(0);                                                                              \
      return;                                                                                 \
    }                                                                                         \
  }


# define ASSERTS_RETURN_INT(c, msg, ...) {                                                   \
    if (!(c)) {                                                                              \
      _LOG_MSG(cutio::log::Level::kError, cutio::log::kLogTypeNotify, msg, __VA_ARGS__);     \
      assert(0);                                                                             \
      return -1;                                                                             \
    }                                                                                        \
  }

# define ASSERTS_RETURN_FALSE(c, msg, ...) {                                                 \
    if (!(c)) {                                                                              \
      _LOG_MSG(cutio::log::Level::kError, cutio::log::kLogTypeNotify, msg, __VA_ARGS__);     \
      assert(0);                                                                             \
      return false;                                                                          \
    }                                                                                        \
  }

# define ASSERTS_RETURN_NULL(c, msg, ...) {                                                  \
    if (!(c)) {                                                                              \
      _LOG_MSG(cutio::log::Level::kError, cutio::log::kLogTypeNotify, msg, __VA_ARGS__);     \
      assert(0);                                                                             \
      return nullptr;                                                                        \
    }                                                                                        \
  }

# define DebugSend(msg, ...)                                                                 \
    _LOG_MSG(cutio::log::Level::kDebug, cutio::log::kLogTypeMsgSend, msg, __VA_ARGS__);

# define DebugRecv(msg, ...)                                                                 \
    _LOG_MSG(cutio::log::Level::kDebug, cutio::log::kLogTypeMsgRecv, msg, __VA_ARGS__);

# define DebugMsg(msg, ...)                                                                  \
    _LOG_MSG(cutio::log::Level::kDebug, cutio::log::kLogTypeNotify, msg, __VA_ARGS__);

# define InfoMsg(msg, ...)                                                                   \
    _LOG_MSG(cutio::log::Level::kInfo, cutio::log::kLogTypeNotify, msg, __VA_ARGS__);

# define WarnMsg(msg, ...)                                                                   \
    _LOG_MSG(cutio::log::Level::kWarn, cutio::log::kLogTypeNotify, msg, __VA_ARGS__);

# define ErrorMsg(msg, ...)                                                                  \
    _LOG_MSG(cutio::log::Level::kError, cutio::log::kLogTypeNotify, msg, __VA_ARGS__);

# define FatalMsg(msg, ...)                                                                  \
    _LOG_MSG(cutio::log::Level::kFatal, cutio::log::kLogTypeNotify, msg, __VA_ARGS__);       \
    abort();

class Config;

class LogEntity {
 public:
  LogEntity();
  LogEntity(std::string time, std::string type, std::string msg, Level level);

  std::string GetTime() const { return time_; }
  std::string GetType() const { return type_; }
  std::string GetMessage() const { return msg_; }
  Level GetLevel() const { return level_; }

 private:
  std::string time_;
  std::string type_;
  std::string msg_;
  Level level_;
};

class LogController {
 public:
  static LogController* GetInstance();
  static LogController* Initialize(const std::string& config_file, const std::string& profile_name,
                                   const std::string& extra_param, const std::string& log_filepath);

  ~LogController();
  LogController(const LogController&) = delete;
  LogController(LogController&&) = delete;
  LogController& operator=(LogController) = delete;

  void Log(Level level, const char* type, const char* msg);
  Level GetLevel() const;

 private:
  LogController(const std::string& config_file, const std::string& profile_name,
                const std::string& extra_param, const std::string& log_filepath);

  bool GetConfig();
  bool InitLogFile();
  std::string FormatTime(bool used_for_path);
  void PrintDelimiter(bool start);
  void PushLogEntity(const LogEntity& entity);
  bool RotateLogFile();
  bool BackupLogFiles();

 private:
  FRIEND_TEST(LogControllerTest, FormatTime);
  FRIEND_TEST(LogControllerTest, Construct);
  FRIEND_TEST(LogControllerTest, GetConfig);

 private:
  static LogController* instance_;

  Config* config_;
  std::string begin_time_;
  std::string end_time_;
  fs::path base_path_;
  fs::path config_filepath_;
  fs::path log_path_;
  fs::path current_log_filename_;

  int log_index_;
  FILE* log_file_;
  std::mutex mu_;
};

}  // namespace cutio::log
