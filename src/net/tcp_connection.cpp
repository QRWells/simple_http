#include <memory>
#include <mutex>

#include <unistd.h>

#include "utils/msg_buffer.hpp"

#include "tcp_connection.hpp"

namespace simple_http::net {
TcpConnection::TcpConnection(EventLoop *event_loop, int fd, InetAddr const &local_addr, InetAddr const &peer_addr)
    : event_loop_(event_loop),
      channel_(std::make_unique<Channel>(event_loop, fd)),
      socket_(std::make_unique<Socket>(fd)),
      local_addr_(local_addr),
      peer_addr_(peer_addr) {
  channel_->SetReadEventHandler([this]() { HandleRead(); });
  channel_->SetWriteEventHandler([this]() { HandleWrite(); });
  channel_->SetCloseEventHandler([this]() { HandleClose(); });
  channel_->SetErrorEventHandler([this]() { HandleError(); });

  socket_->SetKeepAlive(true);
}

void TcpConnection::Send(std::string_view msg) {
  if (event_loop_->IsInLoopThread()) {
    std::lock_guard<std::mutex> lock(send_mutex_);
    if (send_count_ == 0) {
      SendInLoop(msg);
    } else {
      ++send_count_;
      event_loop_->QueueInLoop([conn = shared_from_this(), msg]() {
        conn->SendInLoop(msg);
        std::lock_guard<std::mutex> lock2(conn->send_mutex_);
        --conn->send_count_;
      });
    }
  } else {
    std::lock_guard<std::mutex> lock(send_mutex_);
    ++send_count_;
    event_loop_->QueueInLoop([conn = shared_from_this(), msg]() {
      conn->SendInLoop(msg);
      std::lock_guard<std::mutex> lock2(conn->send_mutex_);
      --conn->send_count_;
    });
  }
}

void TcpConnection::Send(util::MsgBuffer &msg) {
  if (event_loop_->IsInLoopThread()) {
    std::lock_guard<std::mutex> lock(send_mutex_);
    if (send_count_ == 0) {
      SendInLoop({msg.Peek(), msg.ReadableSize()});
    } else {
      ++send_count_;
      event_loop_->QueueInLoop([conn = shared_from_this(), &msg]() {
        conn->SendInLoop({msg.Peek(), msg.ReadableSize()});
        std::lock_guard<std::mutex> lock2(conn->send_mutex_);
        --conn->send_count_;
      });
    }
  } else {
    std::lock_guard<std::mutex> lock(send_mutex_);
    ++send_count_;
    event_loop_->QueueInLoop([conn = shared_from_this(), &msg]() {
      conn->SendInLoop({msg.Peek(), msg.ReadableSize()});
      std::lock_guard<std::mutex> lock2(conn->send_mutex_);
      --conn->send_count_;
    });
  }
}

void TcpConnection::SendInLoop(std::string_view msg) {
  if (state_ != ConnectionState::kConnected) {
    return;
  }
  size_t  remain_len  = msg.size();
  ssize_t send_len    = 0;
  bool    fault_error = false;
  if (!channel_->IsWriting() && write_buffer_.empty()) {
    send_len = ::write(socket_->GetFd(), msg.data(), msg.size());
    if (send_len >= 0) {
      remain_len = msg.size() - send_len;
      if (remain_len == 0 && write_complete_handler_) {
        event_loop_->QueueInLoop([conn = shared_from_this()] { conn->write_complete_handler_(conn); });
      }
    } else {
      send_len = 0;
      if (errno != EWOULDBLOCK) {
        if (errno == EPIPE || errno == ECONNRESET) {
          fault_error = true;
        }
      }
    }
  }
  if (!fault_error && remain_len > 0) {
    if (write_buffer_.empty()) {
      write_buffer_.emplace_back(std::make_unique<util::MsgBuffer>());
    }
    write_buffer_.back()->Write(msg.data() + send_len, remain_len);
    if (!channel_->IsWriting()) {
      channel_->EnableWriting();
    }
  }
}

void TcpConnection::Shutdown() {
  event_loop_->RunInLoop([this_ptr = shared_from_this()]() {
    if (this_ptr->state_ == ConnectionState::kConnected) {
      this_ptr->state_ = ConnectionState::kDisconnecting;
      if (!this_ptr->channel_->IsWriting()) {
        this_ptr->socket_->Shutdown();
      }
    }
  });
}

void TcpConnection::ForceClose() {
  event_loop_->RunInLoop([this_ptr = shared_from_this()]() {
    if (this_ptr->state_ == ConnectionState::kConnected || this_ptr->state_ == ConnectionState::kDisconnecting) {
      this_ptr->state_ = ConnectionState::kDisconnecting;
      this_ptr->HandleClose();
    }
  });
}

void TcpConnection::InformConnected() {
  auto this_ptr = shared_from_this();
  event_loop_->RunInLoop([this_ptr]() {
    this_ptr->channel_->EnableReading();
    this_ptr->state_ = ConnectionState::kConnected;
    if (this_ptr->connection_handler_) {
      this_ptr->connection_handler_(this_ptr);
    }
  });
}

void TcpConnection::ConnectionDestroyed() {
  if (state_ == ConnectionState::kConnected) {
    state_ = ConnectionState::kDisconnected;
    channel_->DisableAll();

    connection_handler_(shared_from_this());
  }
  channel_->Remove();
}

void TcpConnection::HandleRead() {
  int     ret = 0;
  ssize_t n   = read_buffer_.ReadFile(socket_->GetFd(), &ret);

  if (n == 0) {
    // socket is closed by peer
    HandleClose();
  } else if (n < 0) {
    if (errno == EPIPE || errno == ECONNRESET) {
      return;
    }
    if (errno == EAGAIN) {
      return;
    }
    HandleClose();
    return;
  }
  if (n > 0) {
    if (receive_message_handler_) {
      receive_message_handler_(shared_from_this(), read_buffer_);
    }
  }
}
void TcpConnection::HandleWrite() {
  if (!channel_->IsWriting()) {
    // TODO: log error
    return;
  }

  auto write_buf = write_buffer_.front();
  if (write_buf->ReadableSize() == 0) {
    write_buffer_.pop_front();
    if (write_buffer_.empty()) {
      channel_->DisableWriting();
      if (write_complete_handler_) {
        write_complete_handler_(shared_from_this());
      }
      if (state_ == ConnectionState::kDisconnecting) {
        Shutdown();
      }
    } else {
      auto next = write_buffer_.front();
    }
  } else {
    auto n = ::write(socket_->GetFd(), write_buf->Peek(), write_buf->ReadableSize());
    if (n >= 0) {
      write_buf->Retrieve(n);
    } else {
      // TODO: log error
    }
  }
}

void TcpConnection::HandleClose() {
  state_ = ConnectionState::kDisconnected;
  channel_->DisableAll();
  //  ioChannelPtr_->remove();
  auto guard_this = shared_from_this();
  if (connection_handler_) {
    connection_handler_(guard_this);
  }
  if (close_handler_) {
    close_handler_(guard_this);
  }
}

void TcpConnection::HandleError() {
  auto err = socket_->GetSocketError();
  if (err == 0) {
    return;
  }
  // TODO: log error
}

}  // namespace simple_http::net