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
#include <memory>

#include "net/acceptor.hpp"
#include "net/channel.hpp"
#include "net/epoll.hpp"
#include "net/inet_addr.hpp"
#include "net/socket.hpp"
#include "utils/non_copyable.hpp"

namespace simple_http::net {
constexpr inline auto kBufSize = 16;

struct TcpServer : public simple_http::util::NonCopyable {
 public:
  TcpServer(uint16_t port);
  ~TcpServer();

  void Start();
  void Stop();

 private:
  std::string      name_{};
  std::atomic_bool running_{false};
  InetAddr         addr_{};
  Socket           listen_socket_;
  Epoll            epoll_;

  std::unique_ptr<Acceptor> acceptor_{nullptr};
};
}  // namespace simple_http::net