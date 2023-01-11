#pragma once

#include <atomic>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

#include "utils/concurrent_queue.hpp"
#include "utils/non_copyable.hpp"

// TODO: support schedule task

namespace simple_http::net {
struct Epoll;
struct Channel;

using ChannelList = std::vector<Channel*>;
using Func        = std::function<void()>;

/**
 * @brief Single thread event loop
 *
 */
struct EventLoop : public simple_http::util::NonCopyable {
 public:
  EventLoop();
  ~EventLoop();

  [[nodiscard]] static EventLoop* GetEventLoopOfCurrentThread();

  [[nodiscard]] bool IsInLoopThread() const;

  void Start();
  void Stop();
  void WakeUp() const;

  void RunInLoop(Func func);
  void QueueInLoop(Func func);

  void UpdateChannel(Channel* channel);
  void RemoveChannel(Channel* channel);

 private:
  std::atomic_bool running_{false};
  std::atomic_bool quit_{false};
  std::thread::id  thread_id_;

  std::unique_ptr<Epoll> epoller_;

  ChannelList active_channels_;
  Channel*    current_active_channel_{nullptr};

  int                      wakeup_fd_;
  std::unique_ptr<Channel> wakeup_channel_;

  util::ConcurrentQueue<Func> pending_func_queue_;

  void InvokeRunInLoopFuncs();
};
}  // namespace simple_http::net