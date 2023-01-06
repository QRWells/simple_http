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

#include <net/tcp_server.hpp>

int main(int argc, char *argv[]) {
  auto server = simple_http::net::TcpServer(8888);  // NOLINT
  server.Start();
  return 0;
}