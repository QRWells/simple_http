/**
 * @file tcp_server.cpp
 * @author Qirui Wang (qirui.wang@moegi.waseda.jp)
 * @brief
 * @version 0.1
 * @date 2023-01-11
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <condition_variable>
#include <iostream>
#include <mutex>

#include <net/event_loop_thread.hpp>
#include <net/inet_addr.hpp>
#include <net/tcp_server.hpp>

using namespace std;
using namespace simple_http::net;

int main(int argc, char *argv[]) {
  EventLoopThread thread;
  thread.Run();

  InetAddr addr{8888};

  TcpServer server{thread.GetLoop(), addr};
  server.SetEventLoopGroupNum(2);
  server.Start();
  thread.Wait();
  return 0;
}