#include <array>
#include <exception>
#include <iostream>
#include <memory>
#include <vector>

#include <csignal>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>

#include "net/event_loop.hpp"
#include "net/inet_addr.hpp"
#include "net/socket.hpp"
#include "net/tcp_connection.hpp"
#include "utils/msg_buffer.hpp"

#include "tcp_server.hpp"

namespace simple_http::net {

TcpServer::TcpServer(EventLoop *event_loop, InetAddr const &addr)
    : addr_(addr),
      event_loop_(event_loop),
      acceptor_(std::make_unique<Acceptor>(event_loop, addr_, true, true)),
      receive_message_handler_([](std::shared_ptr<TcpConnection> const &conn, util::MsgBuffer &msg) {
        conn->Send(msg);
        msg.RetrieveAll();
      }) {
  ::signal(SIGPIPE, SIG_IGN);  // ignore SIGPIPE

  acceptor_->OnNewConnection([this](int fd, InetAddr const &addr) { this->HandleNewConnection(fd, addr); });
}

TcpServer::~TcpServer() = default;

void TcpServer::Start() {
  event_loop_->RunInLoop([this]() {
    running_ = true;
    acceptor_->Listen();
  });
}

void TcpServer::Stop() {
  running_.store(false, std::memory_order_release);
  if (event_loop_->IsInLoopThread()) {
    acceptor_.reset();
    for (auto const &connection : connections_) {
      connection->ForceClose();
    }
  } else {
    std::promise<void> pro;
    auto               f = pro.get_future();
    event_loop_->QueueInLoop([this, &pro]() {
      acceptor_.reset();
      for (auto const &connection : connections_) {
        connection->ForceClose();
      }
      pro.set_value();
    });
    f.get();
  }
  event_loop_group_.reset();
}

void TcpServer::HandleNewConnection(int fd, InetAddr const &addr) {
  EventLoop *io_loop = nullptr;
  if (event_loop_group_) {
    io_loop = event_loop_group_->GetNextEventLoop();
  }
  if (io_loop == nullptr) {
    io_loop = event_loop_;
  }

  auto new_conn = std::make_shared<TcpConnection>(io_loop, fd, InetAddr{Socket::GetLocalAddr(fd)}, addr);
  new_conn->SetReceiveMessageHandler(receive_message_handler_);
  new_conn->SetCloseHandler([this](std::shared_ptr<TcpConnection> const &conn) { HandleConnectionClosed(conn); });
  new_conn->SetWriteCompleteHandler([this](std::shared_ptr<TcpConnection> const &conn) {
    if (write_complete_handler_) {
      write_complete_handler_(conn);
    }
  });
  new_conn->SetConnectionHandler([this](std::shared_ptr<TcpConnection> const &conn) {
    if (connection_handler_) {
      connection_handler_(conn);
    }
  });

  connections_.emplace(new_conn);
  new_conn->InformConnected();
}

void TcpServer::HandleConnectionClosed(std::shared_ptr<TcpConnection> const &conn) {
  if (event_loop_->IsInLoopThread()) {
    auto  id   = connections_.erase(conn);
    auto *loop = conn->GetEventLoop();
    loop->QueueInLoop([conn, id] { conn->ConnectionDestroyed(); });
  } else {
    event_loop_->QueueInLoop([this, conn] {
      auto  id   = connections_.erase(conn);
      auto *loop = conn->GetEventLoop();
      loop->QueueInLoop([conn, id] { conn->ConnectionDestroyed(); });
    });
  }
}

}  // namespace simple_http::net