#pragma once

#include <memory>

#include "net/channel.hpp"
#include "net/event_loop.hpp"
#include "net/inet_addr.hpp"
#include "net/socket.hpp"
#include "utils/non_copyable.hpp"

namespace simple_http::net {

using NewConnectionHandler = std::function<void(int fd, InetAddr const &)>;

struct Acceptor : public simple_http::util::NonCopyable {
 public:
  Acceptor(EventLoop *loop, InetAddr const &addr, bool reuse_addr = true, bool reuse_port = true);
  ~Acceptor();

  [[nodiscard]] InetAddr const &GetAddr() const;

  void OnNewConnection(NewConnectionHandler handler);

  void Listen();

 private:
  Socket               socket_;
  InetAddr             addr_;
  EventLoop           *loop_;
  Channel              channel_;
  NewConnectionHandler new_connection_handler_;
};
}  // namespace simple_http::net