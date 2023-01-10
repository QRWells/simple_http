#pragma once

#include <cstdint>
#include <functional>
#include <memory>

#include <sys/epoll.h>

#include "net/event_loop.hpp"
#include "utils/non_copyable.hpp"

namespace simple_http::net {
enum class ChannelState {
  kNew,
  kAdded,
  kDeleted,
};
struct Channel : public ::simple_http::util::NonCopyable, public std::enable_shared_from_this<Channel> {
 public:
  Channel(EventLoop* event_loop, int fd) : fd_(fd), event_loop_(event_loop) {}
  Channel(std::shared_ptr<EventLoop> event_loop, int fd) : fd_(fd), event_loop_(event_loop) {}  // NOLINT

  using EventHandler = std::function<void()>;

  void SetReadEventHandler(EventHandler handler) { readEventHandler_ = std::move(handler); }
  void SetWriteEventHandler(EventHandler handler) { writeEventHandler_ = std::move(handler); }
  void SetErrorEventHandler(EventHandler handler) { errorEventHandler_ = std::move(handler); }
  void SetCloseEventHandler(EventHandler handler) { closeEventHandler_ = std::move(handler); }
  void SetEventEventHandler(EventHandler handler) { eventEventHandler_ = std::move(handler); }

  [[nodiscard]] int GetFd() const { return fd_; }

  [[nodiscard]] ChannelState GetState() const { return state_; }
  void                       SetState(ChannelState state) { state_ = state; }

  [[nodiscard]] uint32_t GetEvents() const { return enabled_events_; }
  void                   SetEvents(uint32_t events) { enabled_events_ = events; }

  void EnableAll() {
    enabled_events_ = kReadEvent | kWriteEvent;
    Update();
  }
  void DisableAll() {
    enabled_events_ = kNoneEvent;
    Update();
  }
  void EnableReading() {
    enabled_events_ |= kReadEvent;
    Update();
  }
  void DisableReading() {
    enabled_events_ &= ~kReadEvent;
    Update();
  }
  void EnableWriting() {
    enabled_events_ |= kWriteEvent;
    Update();
  }
  void DisableWriting() {
    enabled_events_ &= ~kWriteEvent;
    Update();
  }

  [[nodiscard]] bool IsWritingEnabled() const { return (enabled_events_ & kWriteEvent) != 0U; }
  [[nodiscard]] bool IsReadingEnabled() const { return (enabled_events_ & kReadEvent) != 0U; }

  void UpdateEvents(int events) {
    enabled_events_ = events;
    Update();
  }

  void Update() { event_loop_->UpdateChannel(shared_from_this()); }
  void Remove() { event_loop_->RemoveChannel(shared_from_this()); }

  [[nodiscard]] uint32_t GetOccurredEvents() const { return occurred_events_; }
  void                   SetOccurredEvents(uint32_t occurred_events) { occurred_events_ = occurred_events; }

  [[nodiscard]] bool IsNoneEvent() const { return enabled_events_ == kNoneEvent; }
  [[nodiscard]] bool IsReading() const { return (enabled_events_ & EPOLLIN) != 0U; }
  [[nodiscard]] bool IsWriting() const { return (enabled_events_ & EPOLLOUT) != 0U; }

 private:
  friend class EventLoop;

  int const fd_;

  constexpr static uint32_t kNoneEvent  = 0;
  constexpr static uint32_t kReadEvent  = EPOLLIN | EPOLLPRI;
  constexpr static uint32_t kWriteEvent = EPOLLOUT;

  uint32_t enabled_events_{kNoneEvent};
  uint32_t occurred_events_{kNoneEvent};

  ChannelState state_{ChannelState::kNew};

  std::shared_ptr<EventLoop> event_loop_;

  EventHandler readEventHandler_;
  EventHandler writeEventHandler_;
  EventHandler errorEventHandler_;
  EventHandler closeEventHandler_;
  EventHandler eventEventHandler_;

  void HandleEvent();
};

}  // namespace simple_http::net