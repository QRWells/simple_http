/**
 * @file non_copyable.hpp
 * @author Qirui Wang (qirui.wang@moegi.waseda.jp)
 * @brief Helper class to make a class non-copyable
 * @version 0.1
 * @date 2023-01-04
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

namespace simple_http::util {
struct NonCopyable {
 protected:
  NonCopyable()                                         = default;
  ~NonCopyable()                                        = default;
  NonCopyable(NonCopyable const &)                      = delete;
  NonCopyable &operator=(NonCopyable const &)           = delete;
  NonCopyable(NonCopyable &&) noexcept(true)            = default;
  NonCopyable &operator=(NonCopyable &&) noexcept(true) = default;
};
}  // namespace simple_http::util