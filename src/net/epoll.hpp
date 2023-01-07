#pragma once

#include <vector>

#include <sys/epoll.h>

#include "utils/non_copyable.hpp"

namespace simple_http::net {
struct Epoll : public simple_http::util::NonCopyable {
 public:
  Epoll();
  ~Epoll();

  void Add(int fd, uint32_t events) const;
  void Mod(int fd, uint32_t events) const;
  void Del(int fd) const;
  void Wait(int timeout, std::vector<struct epoll_event> &events);

  [[nodiscard]] int GetFd() const noexcept { return epoll_fd_; }

 private:
  static constexpr auto kInitEvents = 32;

  int epoll_fd_;

  std::vector<struct epoll_event> events_;
};
}  // namespace simple_http::net