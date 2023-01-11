#pragma once

#include <any>
#include <list>
#include <memory>
#include <mutex>
#include <string_view>
#include <utility>

#include <sys/types.h>

#include "net/channel.hpp"
#include "net/event_loop.hpp"
#include "net/inet_addr.hpp"
#include "net/socket.hpp"
#include "utils/msg_buffer.hpp"
#include "utils/non_copyable.hpp"

namespace simple_http::net {
struct TcpConnection;

using ReceiveMessageHandler = std::function<void(std::shared_ptr<TcpConnection> const &, util::MsgBuffer &)>;
using ConnectionHandler     = std::function<void(std::shared_ptr<TcpConnection> const &)>;
using CloseHandler          = std::function<void(std::shared_ptr<TcpConnection> const &)>;
using WriteCompleteHandler  = std::function<void(std::shared_ptr<TcpConnection> const &)>;

struct TcpConnection : public simple_http::util::NonCopyable, std::enable_shared_from_this<TcpConnection> {
 public:
  enum class ConnectionState {
    kDisconnected,
    kConnecting,
    kConnected,
    kDisconnecting,
  };

  TcpConnection(EventLoop *event_loop, int fd, InetAddr const &local_addr, InetAddr const &peer_addr);
  ~TcpConnection() = default;

  InetAddr const           &GetLocalAddr() const { return local_addr_; }
  InetAddr const           &GetPeerAddr() const { return peer_addr_; }
  EventLoop                *GetEventLoop() const { return event_loop_; }
  std::shared_ptr<std::any> GetContext() const { return context_; }

  void Send(std::string_view msg);
  void Send(util::MsgBuffer &msg);

  void SetContext(std::shared_ptr<std::any> const &context) { context_ = context; }
  void SetContext(std::shared_ptr<std::any> &&context) { context_ = std::move(context); }

  bool IsConnected() const { return state_ == ConnectionState::kConnected; }
  bool IsDisconnected() const { return state_ == ConnectionState::kDisconnected; }
  bool HasContext() const { return context_ != nullptr; }

  void Shutdown();
  void ForceClose();

  void InformConnected();

 private:
  friend class TcpServer;

  EventLoop                *event_loop_{nullptr};
  std::unique_ptr<Channel>  channel_{nullptr};
  std::unique_ptr<Socket>   socket_{nullptr};
  std::shared_ptr<std::any> context_{nullptr};

  util::MsgBuffer                             read_buffer_{};
  std::list<std::shared_ptr<util::MsgBuffer>> write_buffer_{};

  InetAddr local_addr_{};
  InetAddr peer_addr_{};

  ConnectionState state_{ConnectionState::kDisconnected};

  std::mutex send_mutex_{};
  uint32_t   send_count_{};

  ReceiveMessageHandler receive_message_handler_{};
  ConnectionHandler     connection_handler_{};
  CloseHandler          close_handler_{};
  WriteCompleteHandler  write_complete_handler_{};

  void SetReceiveMessageHandler(ReceiveMessageHandler handler) { receive_message_handler_ = std::move(handler); }
  void SetConnectionHandler(ConnectionHandler handler) { connection_handler_ = std::move(handler); }
  void SetCloseHandler(CloseHandler handler) { close_handler_ = std::move(handler); }
  void SetWriteCompleteHandler(WriteCompleteHandler handler) { write_complete_handler_ = std::move(handler); }

  void HandleRead();
  void HandleWrite();
  void HandleClose();
  void HandleError();

  void ConnectionDestroyed();

  void SendInLoop(std::string_view msg);
};
}  // namespace simple_http::net