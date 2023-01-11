#pragma once

#include <map>
#include <memory>
#include <vector>

#include <sys/epoll.h>

#include "net/channel.hpp"
#include "net/event_loop.hpp"
#include "utils/non_copyable.hpp"

namespace simple_http::net {
struct Epoll : public simple_http::util::NonCopyable {
 public:
  enum class EpollCtrlOperation {
    kAdd = EPOLL_CTL_ADD,
    kDel = EPOLL_CTL_DEL,
    kMod = EPOLL_CTL_MOD,
  };

  Epoll(EventLoop *loop);
  ~Epoll();

  void Add(int fd, uint32_t events) const;
  void Mod(int fd, uint32_t events) const;
  void Remove(int fd) const;
  void Wait(int timeout, std::vector<struct epoll_event> &events);

  void Select(int timeout_ms, ChannelList &active_channels);
  void UpdateChannel(Channel *channel);
  void RemoveChannel(Channel *channel);

  [[nodiscard]] int GetFd() const noexcept { return epoll_fd_; }

 private:
  static constexpr auto kInitEvents = 32;

  int epoll_fd_;

  std::vector<struct epoll_event> events_;
  std::map<int, Channel *>        channels_;
  EventLoop                      *loop_;

  void Update(EpollCtrlOperation operation, Channel *channel) const;
};
}  // namespace simple_http::net