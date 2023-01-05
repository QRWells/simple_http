/**
 * @file main.cpp
 * @author Qirui Wang (qirui.wang@moegi.waseda.jp)
 * @brief
 * @version 0.1
 * @date 2023-01-06
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

constexpr auto kDefaultPort = 8080;
constexpr auto kMaxConn     = 16;
constexpr auto kMaxEvents   = 32;
constexpr auto kBufSize     = 16;
constexpr auto kMaxLine     = 256;

void ServerRun();

int main(int argc, char *argv[]) {
  ServerRun();
  return 0;
}

static void EpollCtlAdd(int epfd, int fd, uint32_t events) {
  struct epoll_event ev {};
  ev.events  = events;
  ev.data.fd = fd;
  if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
    perror("epoll_ctl()\n");
    exit(1);
  }
}

static void SetSockaddr(struct sockaddr_in *addr) {
  bzero(addr, sizeof(struct sockaddr_in));
  addr->sin_family      = AF_INET;
  addr->sin_addr.s_addr = INADDR_ANY;
  addr->sin_port        = htons(kDefaultPort);
}

static int Setnonblocking(int sockfd) {
  if (fcntl(sockfd, F_SETFD, fcntl(sockfd, F_GETFD, 0) | O_NONBLOCK) == -1) {
    return -1;
  }
  return 0;
}

/*
 * epoll echo server
 */
void ServerRun() {
  int epfd          = 0;
  int nfds          = 0;
  int listen_sock   = 0;
  int conn_sock     = 0;
  socklen_t socklen = 0;
  char buf[kBufSize];
  struct sockaddr_in srv_addr {};
  struct sockaddr_in cli_addr {};
  struct epoll_event events[kMaxEvents];

  listen_sock = socket(AF_INET, SOCK_STREAM, 0);

  SetSockaddr(&srv_addr);
  auto res = bind(listen_sock, reinterpret_cast<struct sockaddr *>(&srv_addr),
                  sizeof(srv_addr));

  Setnonblocking(listen_sock);
  listen(listen_sock, kMaxConn);

  epfd = epoll_create(1);
  EpollCtlAdd(epfd, listen_sock, EPOLLIN | EPOLLOUT | EPOLLET);

  socklen = sizeof(cli_addr);
  for (;;) {
    nfds = epoll_wait(epfd, events, kMaxEvents, -1);
    for (auto i = 0; i < nfds; i++) {
      if (events[i].data.fd == listen_sock) {  // handle new connection
        conn_sock =
            accept(listen_sock, reinterpret_cast<struct sockaddr *>(&cli_addr),
                   &socklen);

        inet_ntop(AF_INET, (char *)&(cli_addr.sin_addr), buf, sizeof(cli_addr));
        printf("[+] connected with %s:%d\n", buf, ntohs(cli_addr.sin_port));

        Setnonblocking(conn_sock);
        EpollCtlAdd(epfd, conn_sock, EPOLLIN | EPOLLET | EPOLLRDHUP | EPOLLHUP);
      } else if ((events[i].events & EPOLLIN) != 0U) {  // handle read
        for (;;) {
          bzero(buf, sizeof(buf));
          auto n = read(events[i].data.fd, buf, sizeof(buf));
          if (n <= 0) {
            break;
          }
          printf("[+] data: %s\n", buf);
          write(events[i].data.fd, buf, strlen(buf));
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