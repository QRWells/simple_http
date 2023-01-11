#include <iostream>

#include <unistd.h>

#include "net/socket.hpp"

#include "acceptor.hpp"
namespace simple_http::net {

Acceptor::Acceptor(EventLoop *loop, InetAddr const &addr, bool reuse_addr, bool reuse_port)
    : socket_(Socket::CreateNonBlockingSocket()), addr_(addr), loop_(loop), channel_(loop, socket_.GetFd()) {
  socket_.SetReuseAddr(reuse_addr);
  socket_.SetReusePort(reuse_port);
  socket_.Bind(addr);

  channel_.SetReadEventHandler([this] {
    InetAddr peer;
    int      newsock = socket_.Accept(peer);
    if (newsock >= 0) {
      if (new_connection_handler_) {
        new_connection_handler_(newsock, peer);
        std::cout << "New connection from " << peer.ToIpPort() << std::endl;
      } else {
        ::close(newsock);
      }
    }
    // TODO: handle error
  });

  if (addr_.GetPort() == 0) {
    addr_ = InetAddr{Socket::GetLocalAddr(socket_.GetFd())};
  }
}

Acceptor::~Acceptor() {
  channel_.DisableAll();
  channel_.Remove();
}

InetAddr const &Acceptor::GetAddr() const { return addr_; }

void Acceptor::OnNewConnection(NewConnectionHandler handler) { new_connection_handler_ = std::move(handler); }

void Acceptor::Listen() {
  std::cout << "Listning on " << socket_.GetLocalAddr().ToIpPort() << std::endl;
  socket_.Listen();
  channel_.EnableReading();
}

}  // namespace simple_http::net