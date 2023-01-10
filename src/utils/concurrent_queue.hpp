/**
 * @file concurrent_queue.hpp
 * @author Qirui Wang (qirui.wang@moegi.waseda.jp)
 * @brief
 * @version 0.1
 * @date 2023-01-04
 *
 * @copyright Copyright (c) 2023
 *
 */

#pragma once

#include <atomic>
#include <memory>

#include "non_copyable.hpp"

namespace simple_http::util {

template <typename T>
struct ConcurrentQueue : public NonCopyable {
 public:
  ConcurrentQueue() : head_(std::make_shared<Node>()), tail_(head_.load(std::memory_order_relaxed)) {}

  ~ConcurrentQueue() {
    T output;
    while (this->TryDequeue(output)) {
    }
  }

  void Enqueue(T const& data) {
    auto node       = std::make_shared<Node>(data);
    auto prev_head  = head_.exchange(node);
    prev_head->next = node;
  }

  void Enqueue(T&& data) {
    auto node       = std::make_shared<Node>(std::move(data));
    auto prev_head  = head_.exchange(node);
    prev_head->next = node;
  }

  bool TryDequeue(T& output) {
    auto tail = tail_.load(std::memory_order_relaxed);
    auto next = tail->next.load(std::memory_order_acquire);

    if (next == nullptr) {
      return false;
    }
    output = std::move(*next->data);
    tail_.store(next, std::memory_order_release);
    return true;
  }

  [[nodiscard]] bool Empty() const {
    auto tail = tail_.load(std::memory_order_relaxed);
    auto next = tail->next.load(std::memory_order_acquire);
    return next == nullptr;
  }

 private:
  struct Node {
    Node() = default;
    Node(T const& data) : data(std::make_shared<T>(data)), next(nullptr) {}
    Node(T&& data) : data(std::make_shared<T>(std::move(data))), next(nullptr) {}

    std::shared_ptr<T>                 data;
    std::atomic<std::shared_ptr<Node>> next;
  };

  std::atomic<std::shared_ptr<Node>> head_;
  std::atomic<std::shared_ptr<Node>> tail_;
};
}  // namespace simple_http::util