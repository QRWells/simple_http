#include "sys/poll.h"

#include "channel.hpp"
#include "event_loop.hpp"

namespace simple_http::net {

void Channel::Update() { event_loop_->UpdateChannel(this); }
void Channel::Remove() { event_loop_->RemoveChannel(this); }

void Channel::HandleEvent() {
  if (enabled_events_ == kNoneEvent) {
    return;
  }

  if (eventEventHandler_) {
    eventEventHandler_();
    return;
  }

  if (((occurred_events_ & EPOLLHUP) != 0U) and ((occurred_events_ & EPOLLIN) == 0U)) {
    if (closeEventHandler_) {
      closeEventHandler_();
    }
  }

  if ((occurred_events_ & (POLLNVAL | EPOLLERR)) != 0U) {
    if (errorEventHandler_) {
      errorEventHandler_();
    }
  }

  if ((occurred_events_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) != 0U) {
    if (readEventHandler_) {
      readEventHandler_();
    }
  }

  if ((occurred_events_ & EPOLLOUT) != 0U) {
    if (writeEventHandler_) {
      writeEventHandler_();
    }
  }
}
}  // namespace simple_http::net