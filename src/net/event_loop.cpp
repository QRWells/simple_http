#include <atomic>
#include <ctime>
#include <memory>

#include <cstdlib>
#include <utility>

#include <sys/eventfd.h>
#include <unistd.h>

#include "net/channel.hpp"
#include "net/epoll.hpp"

#include "event_loop.hpp"

namespace simple_http::net {
thread_local EventLoop* t_loop_in_this_thread = nullptr;

int CreateEventfd() {
  int eventfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (eventfd < 0) {
    std::abort();
  }
  return eventfd;
}

EventLoop::EventLoop()
    : running_(false),
      quit_(false),
      thread_id_(std::this_thread::get_id()),
      epoller_(std::make_unique<Epoll>(this)),
      wakeup_fd_(CreateEventfd()),
      wakeup_channel_(std::make_unique<Channel>(this, wakeup_fd_)) {
  if (t_loop_in_this_thread != nullptr) {
    // TODO: Handle error
  } else {
    t_loop_in_this_thread = this;
  }

  wakeup_channel_->SetReadEventHandler([this]() {
    auto one = 1;
    auto n   = ::read(wakeup_fd_, &one, sizeof(one));
  });
  wakeup_channel_->EnableReading();
}

EventLoop::~EventLoop() {
  wakeup_channel_->DisableAll();
  wakeup_channel_->Remove();
  ::close(wakeup_fd_);
  t_loop_in_this_thread = nullptr;
}

EventLoop* EventLoop::GetEventLoopOfCurrentThread() { return t_loop_in_this_thread; }

bool EventLoop::IsInLoopThread() const { return thread_id_ == std::this_thread::get_id(); }

void EventLoop::Start() {
  running_ = true;
  quit_    = false;

  while (!quit_.load(std::memory_order_acquire)) {
    active_channels_.clear();
    epoller_->Select(10000, active_channels_);
    for (auto const& channel : active_channels_) {
      current_active_channel_ = channel;
      current_active_channel_->HandleEvent();
    }
    current_active_channel_ = nullptr;
    InvokeRunInLoopFuncs();
  }

  running_ = false;
}

void EventLoop::Stop() {
  quit_.store(true, std::memory_order_release);
  if (!IsInLoopThread()) {
    WakeUp();
  }
}

void EventLoop::WakeUp() const {
  auto one = 1;
  auto n   = ::write(wakeup_fd_, &one, sizeof(one));
}

void EventLoop::RunInLoop(Func func) {
  if (IsInLoopThread()) {
    func();
  } else {
    QueueInLoop(std::forward<Func>(func));
  }
}

void EventLoop::QueueInLoop(Func func) {
  pending_func_queue_.Enqueue(std::move(func));
  if (!IsInLoopThread() or !running_.load(std::memory_order_acquire)) {
    WakeUp();
  }
}

void EventLoop::UpdateChannel(Channel* channel) { epoller_->UpdateChannel(channel); }
void EventLoop::RemoveChannel(Channel* channel) { epoller_->RemoveChannel(channel); }

void EventLoop::InvokeRunInLoopFuncs() {
  while (!pending_func_queue_.Empty()) {
    Func func;
    while (pending_func_queue_.TryDequeue(func)) {
      func();
    }
  }
}

}  // namespace simple_http::net