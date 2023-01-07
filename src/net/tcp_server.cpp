#include <array>
#include <exception>

#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <vector>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <unistd.h>

#include "net/InetAddr.hpp"
#include "net/epoll.hpp"
#include "net/socket.hpp"

#include "tcp_server.hpp"

namespace simple_http::net {

TcpServer::TcpServer(uint16_t port) : listen_socket_(Socket::CreateNonBlockingSocket()), addr_(port) {
  listen_socket_.Bind(addr_);
  listen_socket_.Listen();
  epoll_.Add(listen_socket_.GetFd(), EPOLLIN | EPOLLOUT | EPOLLET);
}

TcpServer::~TcpServer() { Stop(); }

void TcpServer::Start() {
  std::vector<struct epoll_event> events;
  std::array<char, kBufSize>      buf;
  InetAddr                        cli_addr{};

  running_.store(true, std::memory_order_release);

  while (running_.load(std::memory_order_acquire)) {
    epoll_.Wait(-1, events);
    for (auto event : events) {
      if (event.data.fd == listen_socket_.GetFd()) {  // handle new connection
        auto conn_sock = listen_socket_.Accept(cli_addr);

        std::cout << "[+] connected with " << cli_addr.ToIpPort() << std::endl;

        epoll_.Add(conn_sock, EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP);
      } else if ((event.events & EPOLLIN) != 0U) {  // handle read
        for (;;) {
          bzero(buf.data(), buf.size());
          auto n = read(event.data.fd, buf.data(), buf.size());
          if (n <= 0) {
            break;
          }
          printf("[+] data: %s\n", buf.data());
          write(event.data.fd, buf.data(), strlen(buf.data()));
        }
      } else {
        printf("[+] unexpected\n");
      }
      // check if the connection is closing
      if ((event.events & (EPOLLRDHUP | EPOLLHUP)) != 0U) {
        printf("[+] connection closed\n");
        epoll_.Del(event.data.fd);
        close(event.data.fd);
        continue;
      }
    }
  }
}

void TcpServer::Stop() { running_.store(false, std::memory_order_release); }

}  // namespace simple_http::net