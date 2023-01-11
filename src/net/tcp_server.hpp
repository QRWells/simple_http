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
#include <condition_variable>
#include <future>
#include <memory>
#include <mutex>
#include <set>
#include <thread>

#include <cstdint>

#include "net/acceptor.hpp"
#include "net/channel.hpp"
#include "net/epoll.hpp"
#include "net/event_loop.hpp"
#include "net/event_loop_group.hpp"
#include "net/inet_addr.hpp"
#include "net/socket.hpp"
#include "net/tcp_connection.hpp"
#include "utils/non_copyable.hpp"

namespace simple_http::net {
constexpr inline auto kBufSize = 16;

struct TcpServer : public simple_http::util::NonCopyable {
 public:
  TcpServer(EventLoop* event_loop, InetAddr const& addr, size_t thread_num = std::thread::hardware_concurrency());
  ~TcpServer();

  void Start();
  void Stop();

  void SetEventLoopGroupNum(size_t num) {
    event_loop_group_ = std::make_unique<EventLoopGroup>(num);
    event_loop_group_->Start();
  }

  void OnReceiveMessage(ReceiveMessageHandler handler) { receive_message_handler_ = std::move(handler); }
  void OnWriteComplete(WriteCompleteHandler handler) { write_complete_handler_ = std::move(handler); }
  void OnConnection(ConnectionHandler handler) { connection_handler_ = std::move(handler); }

 private:
  std::string      name_{};
  std::atomic_bool running_{false};
  InetAddr         addr_{};

  ReceiveMessageHandler receive_message_handler_{};
  WriteCompleteHandler  write_complete_handler_{};
  ConnectionHandler     connection_handler_{};

  EventLoop*                      event_loop_{};
  std::unique_ptr<Acceptor>       acceptor_;
  std::unique_ptr<EventLoopGroup> event_loop_group_{nullptr};

  std::set<std::shared_ptr<TcpConnection>> connections_{};

  void HandleNewConnection(int fd, InetAddr const& addr);
  void HandleConnectionClosed(std::shared_ptr<TcpConnection> const& connection);
};
}  // namespace simple_http::net