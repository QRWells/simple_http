#include <string_view>
#include <thread>

#include <sys/prctl.h>

#include "event_loop_thread.hpp"

namespace simple_http::net {

EventLoopThread::EventLoopThread(std::string_view thread_name)
    : loopThreadName_(thread_name), thread_([this] { LoopFuncs(); }) {
  auto f = promiseForLoopPointer_.get_future();
  loop_  = f.get();
}

EventLoopThread::~EventLoopThread() {
  Run();
  std::shared_ptr<EventLoop> loop;
  {
    std::unique_lock<std::mutex> lk(loopMutex_);
    loop = loop_;
  }
  if (loop) {
    loop->Stop();
  }
  if (thread_.joinable()) {
    thread_.join();
  }
}

void EventLoopThread::Wait() { thread_.join(); }

void EventLoopThread::Run() {
  std::call_once(once_, [this]() {
    auto f = promiseForLoop_.get_future();
    promiseForRun_.set_value(1);
    // Make sure the event loop loops before returning.
    f.get();
  });
}

void EventLoopThread::LoopFuncs() {
  ::prctl(PR_SET_NAME, loopThreadName_.c_str());

  thread_local static std::shared_ptr<EventLoop> loop = std::make_shared<EventLoop>();

  loop->RunInLoop([this]() { promiseForLoop_.set_value(1); });

  promiseForLoopPointer_.set_value(loop);

  promiseForRun_.get_future().get();

  loop->Start();

  // Loop is stopped, set loop_ to nullptr.
  {
    std::unique_lock<std::mutex> lk(loopMutex_);
    loop_.store(nullptr, std::memory_order_release);
  }
}

}  // namespace simple_http::net