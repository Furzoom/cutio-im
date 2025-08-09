// Copyright (c) 2025 Furzoom.com, All rights reserved.
// Author: Niz, mn@furzoom.com

#include "common/log/inc/log_controller.h"

#include <sys/time.h>

#include <cinttypes>
#include <cstring>
#include <ctime>
#include <utility>

#include "common/log/src/config.h"
#include "yaml-cpp/yaml.h"

namespace cutio::log {

// YYYY-MM-DD HH:mm:ss.xxx
const char* kTimeFmt = "%04" PRIu16 "-%02" PRIu16 "-%02" PRIu16
                       " %02" PRIu16 ":%02" PRIu16 ":%02" PRIu16 ".%03" PRIu16;
// YYYYMMDDHHmmss.xxx
const char* kTimeFmtPathName = "%04" PRIu16 "%02" PRIu16 "%02" PRIu16
                               "%02" PRIu16 "%02" PRIu16 "%02" PRIu16 ".%03" PRIu16;

// Max log files.
const int kMaxLogFileCount = 11;

std::string LevelToString(Level level) {
  switch (level) {
    case Level::kClose:return "CLOSE";
    case Level::kFatal:return "FATAL";
    case Level::kError:return "ERROR";
    case Level::kWarn:return "WARN";
    case Level::kInfo:return "INFO";
    case Level::kDebug:return "DEBUG";
    default:assert(0);
  }
  return "UNKNOWN";
}

LogEntity::LogEntity() : level_(Level::kClose) {}

LogEntity::LogEntity(std::string time, std::string type, std::string msg, Level level)
    : time_(std::move(time)),
      type_(std::move(type)),
      msg_(std::move(msg)),
      level_(level) {}

LogController* LogController::instance_ = nullptr;

LogController* LogController::Initialize(const std::string& config_file,
                                         const std::string& profile_name,
                                         const std::string& extra_param,
                                         const std::string& log_filepath) {
  if (instance_ == nullptr) {
    instance_ = new LogController(config_file, profile_name, extra_param, log_filepath);
  }

  return instance_;
}

LogController* LogController::GetInstance() {
  assert(instance_ != nullptr);
  return instance_;
}

LogController::LogController(const std::string& config_file, const std::string& profile_name,
                             const std::string& extra_param, const std::string& log_filepath)
    : config_(new Config),
      log_index_(1),
      log_file_(nullptr) {
  assert(!config_file.empty());
  assert(!profile_name.empty());
  begin_time_ = FormatTime(true);

  config_->name = profile_name;

  char work_path[512];
  if (getcwd(work_path, sizeof(work_path)) == nullptr) {
    if (errno == ERANGE) {
      assert(false);
    }
  }

  if (log_filepath.empty()) {
    base_path_.assign(work_path);
  } else {
    base_path_.assign(log_filepath);
  }

  config_filepath_.assign(config_file);
  if (config_filepath_.is_relative()) {
    config_filepath_.assign(base_path_).append(config_file);
  }

  if (!fs::exists(config_filepath_)) {
    fprintf(stderr, "log config file '%s' does not exist\n", config_filepath_.c_str());
    return;
  }

  if (!GetConfig()) {
    assert(0);
    return;
  }

  if (!extra_param.empty()) {
    config_->basename += "_" + extra_param;
  }

  current_log_filename_.assign(config_->path);
  if (current_log_filename_.is_relative()) {
    current_log_filename_.assign(base_path_);
    current_log_filename_.append(config_->path);
  }
  current_log_filename_.append(config_->basename + "_runtime.log");
  log_path_ = current_log_filename_.parent_path();

  InitLogFile();
}

LogController::~LogController() {
  assert(log_file_);
  PrintDelimiter(false);
  fclose(log_file_);
  delete config_;
}

void LogController::Log(Level level, const char* type, const char* msg) {
  LogEntity entity(FormatTime(false), type, msg, level);
  PushLogEntity(entity);
}

Level LogController::GetLevel() const {
  return config_->level;
}

bool LogController::GetConfig() {
  LogConfigParser lcp(config_filepath_);
  auto name = config_->name;
  *config_ = lcp.GetConfig(config_->name);
  config_->max_size = config_->max_size * 1024;
  if (config_->basename.empty()) {
    fprintf(stderr, "log config section '%s' does not exist\n", name.c_str());
    return false;
  }
  return true;
}

bool LogController::InitLogFile() {
  assert(!current_log_filename_.empty());

  std::error_code err;
  fs::create_directories(log_path_, err);
  if (err) {
    fprintf(stderr, "create '%s' failed: %s\n", log_path_.c_str(), err.message().c_str());
    return false;
  }
  fs::permissions(log_path_, fs::perms::sticky_bit, fs::perm_options::add, err);
  if (err) {
    fprintf(stderr, "change permission of '%s' failed: %s\n", log_path_.c_str(), err.message().c_str());
    return false;
  }

  if (log_file_) {
    fclose(log_file_);
    log_file_ = nullptr;
  }

  log_file_ = fopen(current_log_filename_.c_str(), "a+e");
  if (!log_file_) {
    fprintf(stderr, "open file '%s' failed: %s\n", current_log_filename_.c_str(), strerror(errno));
    assert(0);
    return false;
  }

  if (setvbuf(log_file_, nullptr, _IOLBF, 0) != 0) {
    fprintf(stderr, "set line buffered for '%s' failed\n", current_log_filename_.c_str());
  }

  PrintDelimiter(true);

  return true;
}

std::string LogController::FormatTime(bool used_for_path) {
  timeval tv{};
  gettimeofday(&tv, nullptr);
  time_t sec = tv.tv_sec;
  tm t{};
  localtime_r(&sec, &t);

  char buf[48]{};
  char const* fmt = kTimeFmt;
  if (used_for_path) {
    fmt = kTimeFmtPathName;
  }
  snprintf(buf, sizeof(buf), fmt, t.tm_year + 1900, t.tm_mon, t.tm_mday + 1,
           t.tm_hour, t.tm_min, t.tm_sec, tv.tv_usec / 1000);
  return buf;
}

void LogController::PrintDelimiter(bool start) {
  assert(log_file_);
  const char* title = start ? "START TIME" : "END TIME";
  fprintf(log_file_, "====================%s %s====================\n", title, FormatTime(false).c_str());
}

void LogController::PushLogEntity(const LogEntity& entity) {
  std::lock_guard lock(mu_);
  assert(log_file_);
  fprintf(log_file_, "%s|%5s|%s|%s\n", entity.GetTime().c_str(), LevelToString(entity.GetLevel()).c_str(),
          entity.GetType().c_str(), entity.GetMessage().c_str());

  // Rotate log file.
  auto log_size = ftell(log_file_);
  if (log_size < config_->max_size) {
    return;
  }

  if (!RotateLogFile()) {
    fprintf(stderr, "rotate log file failed\n");
    return;
  }

  // Backup log files.
  if (log_index_ % kMaxLogFileCount != 0) {
    return;
  }

  if (!BackupLogFiles()) {
    fprintf(stderr, "backup log file failed\n");
    return;
  }
}

bool LogController::RotateLogFile() {
  PrintDelimiter(false);
  fclose(log_file_);

  fs::path dst_file_name = log_path_;
  dst_file_name.append(config_->basename + "_" + std::to_string(log_index_) + ".log");

  std::error_code err;
  fs::rename(current_log_filename_, dst_file_name, err);
  if (err == std::errc::file_exists) {
    fs::remove(dst_file_name, err);
    fs::copy_file(current_log_filename_, dst_file_name, err);
  }

  log_index_++;
  if (!InitLogFile()) {
    assert(0);
    return false;
  }

  return true;
}

bool LogController::BackupLogFiles() {
  end_time_ = FormatTime(true);
  fs::path back_path = log_path_;
  back_path.append("bak").append(config_->basename).append(begin_time_ + "--" + end_time_);
  std::error_code err;
  if (!fs::exists(back_path, err)) {
    if (!fs::create_directories(back_path, err)) {
      fprintf(stderr, "create directory '%s' failed: %s\n", back_path.c_str(), err.message().c_str());
      return false;
    }
  }

  for (const auto& dir_entry : fs::directory_iterator(log_path_)) {
    std::string stem = dir_entry.path().stem().string();
    std::string filename = dir_entry.path().filename().string();
    if (dir_entry.is_regular_file() &&
        stem.rfind(config_->basename, 0) == 0 &&
        filename != current_log_filename_.filename().string()) {
      auto new_file = back_path;
      new_file.append(dir_entry.path().filename().string());
      if (!fs::copy_file(dir_entry, new_file, err)) {
        fprintf(stderr, "backup '%s' to '%s' failed: %s\n",
                dir_entry.path().c_str(), new_file.c_str(), err.message().c_str());
      }
      if (!fs::remove(dir_entry, err)) {
        fprintf(stderr, "remove file '%s' failed: %s\n", dir_entry.path().c_str(), err.message().c_str());
      }
    }
  }

  end_time_ = begin_time_;
  return true;
}

}  // namespace cutio::log