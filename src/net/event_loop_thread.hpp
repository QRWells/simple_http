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
#include <string_view>
#include <thread>

#include "net/event_loop.hpp"
#include "utils/non_copyable.hpp"

namespace simple_http::net {

struct EventLoopThread : public simple_http::util::NonCopyable {
 public:
  explicit EventLoopThread(std::string_view thread_name = "EventLoopThread");
  ~EventLoopThread();

  void Wait();
  void Run();

  [[nodiscard]] EventLoop *GetLoop() const { return loop_.load(std::memory_order_acquire).get(); }

 private:
  std::atomic<std::shared_ptr<EventLoop>> loop_{nullptr};
  std::mutex                              loopMutex_;

  std::string                              loopThreadName_;
  std::promise<std::shared_ptr<EventLoop>> promiseForLoopPointer_;
  std::promise<int>                        promiseForRun_;
  std::promise<int>                        promiseForLoop_;
  std::once_flag                           once_;
  std::thread                              thread_;

  void LoopFuncs();
};
}  // namespace simple_http::net