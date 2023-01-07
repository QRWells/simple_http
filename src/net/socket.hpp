#pragma once

#include <stdexcept>

#include <netinet/in.h>
#include <sys/socket.h>

#include "utils/non_copyable.hpp"

namespace simple_http::net {
struct Socket : public simple_http::util::NonCopyable {
 public:
  Socket(int fd) : fd_(fd) {}
  ~Socket();

  static Socket CreateNonBlockingSocket() {
    int fd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (fd < 0) {
      throw std::runtime_error("create socket error");
    }
    return {fd};
  }

  [[nodiscard]] struct sockaddr_in GetLocalAddr() const;
  [[nodiscard]] struct sockaddr_in GetPeerAddr() const;

  [[nodiscard]] int Accept(struct sockaddr_in& peer_addr) const noexcept;
  void              Bind(struct sockaddr_in const& addr) const;
  void              Listen() const;
  void              Shutdown() const;

  void SetReuseAddr(bool on) const noexcept;
  void SetReusePort(bool on) const noexcept;
  void SetKeepAlive(bool on) const noexcept;
  void SetTcpNoDelay(bool on) const noexcept;

  [[nodiscard]] int GetFd() const noexcept { return fd_; }

 private:
  int fd_;
};
}  // namespace simple_http::net