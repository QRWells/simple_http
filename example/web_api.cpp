/**
 * @file web_api.cpp
 * @author Qirui Wang (qirui.wang@moegi.waseda.jp)
 * @brief
 * @version 0.1
 * @date 2023-01-13
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <net/http/http_server.hpp>

using namespace std;
using namespace simple_http::net;
using namespace simple_http::net::http;

int main(int argc, char* argv[]) {
  EventLoopThread thread;
  thread.Run();

  http::HttpServer server{thread.GetLoop()};

  server.Get("hello", [](HttpRequest const& req, HttpResponse& resp) {
    resp.SetStatusCode(200);
    resp.SetStatusMessage("OK");
    resp.SetBody("Hello World!\n");
  });

  server.Start();

  thread.Wait();
  return 0;
}