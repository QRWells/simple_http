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

#include <atomic>
#include <cstdint>

#include "net/epoll.hpp"
#include "net/socket.hpp"
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

  void Start();
  void Stop();

 private:
  std::atomic_bool   running_{false};
  struct sockaddr_in addr_ {};
  Socket             listen_socket_;
  Epoll              epoll_;
};
}  // namespace simple_http::net