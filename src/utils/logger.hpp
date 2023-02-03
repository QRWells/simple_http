/**
 * @file logger.hpp
 * @author Qirui Wang (qirui.wang@moegi.waseda.jp)
 * @brief
 * @version 0.1
 * @date 2023-01-04
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include <iomanip>
#include <iostream>
#include <ostream>
#include <source_location>
#include <string_view>

#include "non_copyable.hpp"

namespace simple_http::util {

enum class LogLevel {
  kDebug,
  kInfo,
  kWarning,
  kError,
  kFatal,
};

struct Logger : public NonCopyable {
 public:
  Logger() : location_(std::source_location::current()) {}
  ~Logger();

  static auto const& GetLogLevel() { return log_level; }
  static auto        SetLogLevel(LogLevel const& level) { log_level = level; }

  static auto& GetTarget() { return target; }
  static auto  SetTarget(std::ostream& os) { target.rdbuf(os.rdbuf()); }

  void Debug(std::string_view message) { Log(LogLevel::kDebug, message); }

  void Info(std::string_view message) { Log(LogLevel::kInfo, message); }

  void Warning(std::string_view message) { Log(LogLevel::kWarning, message); }

  void Error(std::string_view message) { Log(LogLevel::kError, message); }

  void Fatal(std::string_view message) { Log(LogLevel::kFatal, message); }

 private:
  inline static LogLevel     log_level{LogLevel::kDebug};
  inline static std::ostream target{std::cout.rdbuf()};

  std::source_location location_;

  void Log(LogLevel level, std::string_view message) {
    if (level >= log_level) {
      target << "[" << location_.file_name() << ":" << location_.line() << "] " << message << std::endl;
    }
  }
};
}  // namespace simple_http::util