
#include <stdexcept>
#include <vector>

#include <sys/epoll.h>

#include <unistd.h>

#include "epoll.hpp"

namespace simple_http::net {

Epoll::Epoll() : epoll_fd_(::epoll_create1(EPOLL_CLOEXEC)), events_(kInitEvents) {}
Epoll::~Epoll() { ::close(epoll_fd_); }

void Epoll::Add(int fd, uint32_t events) const {
  struct epoll_event ev {};
  ev.events  = events;
  ev.data.fd = fd;
  if (::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
    throw std::runtime_error("epoll_ctl() EPOLL_CTL_ADD");
  }
}

void Epoll::Mod(int fd, uint32_t events) const {
  struct epoll_event ev {};
  ev.events  = events;
  ev.data.fd = fd;
  if (::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == -1) {
    throw std::runtime_error("epoll_ctl() EPOLL_CTL_MOD");
  }
}

void Epoll::Del(int fd) const {
  if (::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) == -1) {
    throw std::runtime_error("epoll_ctl() EPOLL_CTL_DEL");
  }
}

void Epoll::Wait(int timeout, std::vector<struct epoll_event> &events) {
  auto num_events = ::epoll_wait(epoll_fd_, events_.data(), static_cast<int>(events_.size()), timeout);

  events.clear();

  if (num_events > 0) {
    events.assign(events_.begin(), events_.begin() + num_events);

    if (num_events == events_.size()) {
      events_.resize(events_.size() * 2);
    }
  }
}
}  // namespace simple_http::net