#include <stdexcept>

#include <cstdlib>
#include <cstring>

#include <netinet/tcp.h>
#include <unistd.h>

#include "net/InetAddr.hpp"
#include "socket.hpp"

namespace simple_http::net {

Socket::~Socket() {
  if (fd_ >= 0) {
    ::close(fd_);
  }
}

InetAddr Socket::GetLocalAddr() const {
  struct sockaddr_in localaddr;
  ::memset(&localaddr, 0, sizeof(localaddr));

  auto addrlen = static_cast<socklen_t>(sizeof localaddr);

  if (::getsockname(fd_, static_cast<struct sockaddr *>((void *)(&localaddr)), &addrlen) < 0) {
    throw std::runtime_error("getsockname error");
  }
  return InetAddr{localaddr};
}

InetAddr Socket::GetPeerAddr() const {
  struct sockaddr_in peeraddr;
  ::memset(&peeraddr, 0, sizeof(peeraddr));

  auto addrlen = static_cast<socklen_t>(sizeof peeraddr);

  if (::getpeername(fd_, static_cast<struct sockaddr *>((void *)(&peeraddr)), &addrlen) < 0) {
    throw std::runtime_error("getpeername error");
  }

  return InetAddr{peeraddr};
}

int Socket::Accept(InetAddr &peer_addr) const noexcept {
  struct sockaddr_in addr;
  socklen_t          size = sizeof(addr);
  memset(&addr, 0, sizeof(addr));

  int connfd = ::accept4(fd_, (struct sockaddr *)&addr, &size, SOCK_NONBLOCK | SOCK_CLOEXEC);

  if (connfd >= 0) {
    peer_addr.SetSockAddr(addr);
  }

  return connfd;
}

void Socket::Bind(InetAddr const &addr) const {
  int ret;
  ret = ::bind(fd_, addr.GetSockAddr(), sizeof(sockaddr_in));

  if (ret != 0) {
    throw std::runtime_error("bind error");
  }
}

void Socket::Listen() const {
  int ret = ::listen(fd_, SOMAXCONN);
  if (ret < 0) {
    throw std::runtime_error("listen error");
  }
}

void Socket::Shutdown() const {
  if (::shutdown(fd_, SHUT_WR) < 0) {
    throw std::runtime_error("shutdown error");
  }
}

void Socket::SetKeepAlive(bool on) const noexcept {
  int optval = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::SetReuseAddr(bool on) const noexcept {
  int optval = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::SetReusePort(bool on) const noexcept {
  int optval = on ? 1 : 0;
  ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &optval, static_cast<socklen_t>(sizeof optval));
}

void Socket::SetTcpNoDelay(bool on) const noexcept {
  int optval = on ? 1 : 0;
  ::setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, &optval, static_cast<socklen_t>(sizeof optval));
}

}  // namespace simple_http::net