/**
 * @file tcp_server.hpp
 * @author Qirui Wang (qirui.wang@moegi.waseda.jp)
 * @brief Header file for TcpServer class
 * @version 0.1
 * @date 2023-01-04
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include <cstdint>

#include "utils/non_copyable.hpp"

namespace simple_http::net {
constexpr inline auto kDefaultPort = 8080;
constexpr inline auto kMaxConn     = 16;
constexpr inline auto kMaxEvents   = 32;
constexpr inline auto kBufSize     = 16;
constexpr inline auto kMaxLine     = 256;

struct TcpServer : public simple_http::util::NonCopyable {
 public:
  TcpServer(uint16_t port);
  ~TcpServer();

  void Start() const;

 private:
  uint16_t port_;
};
}  // namespace simple_http::net