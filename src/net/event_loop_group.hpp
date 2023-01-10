#pragma once

#include <atomic>
#include <cstddef>
#include <memory>
#include <vector>

#include "net/event_loop.hpp"
#include "net/event_loop_thread.hpp"
#include "utils/non_copyable.hpp"

namespace simple_http::net {
struct EventLoopGroup : public simple_http::util::NonCopyable {
 public:
  EventLoopGroup(size_t num_event_loops = std::thread::hardware_concurrency()) : event_loops_(num_event_loops) {
    for (int i = 0; i < num_event_loops; ++i) {
      event_loops_.emplace_back(std::make_shared<EventLoopThread>());
    }
  }

  ~EventLoopGroup() = default;

  [[nodiscard]] size_t GetSize() const { return event_loops_.size(); }

  void Start() {
    for (auto& event_loop : event_loops_) {
      event_loop->Run();
    }
  }

  void Stop() {
    for (auto& event_loop : event_loops_) {
      event_loop->Wait();
    }
  }

  EventLoop* GetNextEventLoop() {
    if (!event_loops_.empty()) {
      auto* loop = event_loops_[next_event_loop_.fetch_add(1)]->GetLoop();
      next_event_loop_.store(next_event_loop_.load() % event_loops_.size());
      return loop;
    }
    return nullptr;
  }

  EventLoop* GetEventLoop(size_t index) {
    if (index < event_loops_.size()) {
      return event_loops_[index]->GetLoop();
    }
    return nullptr;
  }

 private:
  std::atomic_size_t                            next_event_loop_{0};
  std::vector<std::shared_ptr<EventLoopThread>> event_loops_;
};
}  // namespace simple_http::net