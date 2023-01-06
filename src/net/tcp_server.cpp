#include <array>
#include <exception>

#include <cstddef>
#include <cstdio>
#include <cstring>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "tcp_server.hpp"

namespace simple_http::net {
static void EpollCtlAdd(int epfd, int fd, uint32_t events) {
  struct epoll_event ev {};
  ev.events  = events;
  ev.data.fd = fd;
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
    perror("epoll_ctl()\n");
    std::terminate();
  }
}

static void SetSockaddr(struct sockaddr_in &addr, uint16_t port = kDefaultPort) {
  bzero(&addr, sizeof(struct sockaddr_in));
  addr.sin_family      = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port        = htons(port);
}

static int Setnonblocking(int sockfd) {
  if (fcntl(sockfd, F_SETFD, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK) == -1) {
    return -1;
  }
  return 0;
}

TcpServer::TcpServer(uint16_t port) : port_(port) {}

void TcpServer::Start() const {
  int                                        epfd        = 0;
  int                                        nfds        = 0;
  int                                        listen_sock = 0;
  int                                        conn_sock   = 0;
  socklen_t                                  socklen     = 0;
  std::array<char, kBufSize>                 buf;
  struct sockaddr_in                         srv_addr {};
  struct sockaddr_in                         cli_addr {};
  std::array<struct epoll_event, kMaxEvents> events;

  listen_sock = socket(AF_INET, SOCK_STREAM, 0);

  SetSockaddr(srv_addr, port_);
  auto res = bind(listen_sock, reinterpret_cast<struct sockaddr *>(&srv_addr), sizeof(srv_addr));

  Setnonblocking(listen_sock);
  listen(listen_sock, kMaxConn);

  epfd = epoll_create(1);
  EpollCtlAdd(epfd, listen_sock, EPOLLIN | EPOLLOUT | EPOLLET);

  socklen = sizeof(cli_addr);
  for (;;) {
    nfds = epoll_wait(epfd, events.data(), kMaxEvents, -1);
    for (auto i = 0; i < nfds; i++) {
      if (events[i].data.fd == listen_sock) {  // handle new connection
        conn_sock = accept4(listen_sock, reinterpret_cast<struct sockaddr *>(&cli_addr), &socklen,
                            SOCK_NONBLOCK | SOCK_CLOEXEC);

        inet_ntop(AF_INET, (char *)&(cli_addr.sin_addr), buf.data(), sizeof(cli_addr));
        printf("[+] connected with %s:%d\n", buf.data(), ntohs(cli_addr.sin_port));

        EpollCtlAdd(epfd, conn_sock, EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP);
      } else if ((events[i].events & EPOLLIN) != 0U) {  // handle read
        for (;;) {
          bzero(buf.data(), buf.size());
          auto n = read(events.at(i).data.fd, buf.data(), buf.size());
          if (n <= 0) {
            break;
          }
          printf("[+] data: %s\n", buf.data());
          write(events.at(i).data.fd, buf.data(), strlen(buf.data()));
        }
      } else {
        printf("[+] unexpected\n");
      }
      // check if the connection is closing
      if ((events[i].events & (EPOLLRDHUP | EPOLLHUP)) != 0U) {
        printf("[+] connection closed\n");
        epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
        close(events[i].data.fd);
        continue;
      }
    }
  }
}
}  // namespace simple_http::net