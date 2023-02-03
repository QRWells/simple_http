
#include <stdexcept>
#include <utility>
#include <vector>

#include <sys/epoll.h>

#include <unistd.h>

#include "epoll.hpp"

namespace simple_http::net {

Epoll::Epoll(EventLoop *loop) : epoll_fd_(::epoll_create1(EPOLL_CLOEXEC)), events_(kInitEvents), loop_(loop) {}
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

void Epoll::Remove(int fd) const {
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

void Epoll::Select(int timeout_ms, ChannelList &active_channels) {
  auto num_events = ::epoll_wait(epoll_fd_, &*events_.begin(), static_cast<int>(events_.size()), timeout_ms);
  if (num_events > 0) {
    for (int i = 0; i < num_events; ++i) {
      auto *channel = static_cast<Channel *>(events_[i].data.ptr);
      channel->SetOccurredEvents(static_cast<int>(events_[i].events));
      active_channels.emplace_back(channel);
    }

    if (static_cast<size_t>(num_events) == events_.size()) {
      events_.resize(events_.size() * 2);
    }
  }
}

void Epoll::UpdateChannel(Channel *channel) {
  auto const state = channel->GetState();
  auto       fd    = channel->GetFd();
  switch (state) {
    case ChannelState::kAdded: {
      if (channel->IsNoneEvent()) {
        Update(EpollCtrlOperation::kDel, channel);
        channel->SetState(ChannelState::kDeleted);
      } else {
        Update(EpollCtrlOperation::kMod, channel);
      }
      break;
    }

    case ChannelState::kNew: {
      channels_[fd] = channel;
      [[fallthrough]];
    }
    case ChannelState::kDeleted: {
      channel->SetState(ChannelState::kAdded);
      Update(EpollCtrlOperation::kAdd, channel);
    }
  }
}

void Epoll::RemoveChannel(Channel *channel) {
  auto fd    = channel->GetFd();
  auto n     = channels_.erase(fd);
  auto state = channel->GetState();
  if (state == ChannelState::kAdded) {
    Update(EpollCtrlOperation::kDel, channel);
  }
  channel->SetState(ChannelState::kNew);
}

void Epoll::Update(EpollCtrlOperation operation, Channel *channel) const {
  struct epoll_event event {};
  event.events   = channel->GetEvents();
  event.data.ptr = channel;
  int fd         = channel->GetFd();
  if (operation != EpollCtrlOperation::kDel) {
    event.events |= EPOLLET;
  }
  if (::epoll_ctl(epoll_fd_, (int)operation, fd, &event) < 0) {
    throw std::runtime_error("epoll_ctl error");
  }
}

}  // namespace simple_http::net