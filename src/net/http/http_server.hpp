#pragma once

#include <chrono>
#include <functional>
#include <map>
#include <memory>
#include <regex>
#include <vector>

#include "net/event_loop.hpp"
#include "net/http/http_request.hpp"
#include "net/http/http_response.hpp"
#include "net/tcp_connection.hpp"
#include "net/tcp_server.hpp"
#include "utils/msg_buffer.hpp"
#include "utils/non_copyable.hpp"

namespace simple_http::net::http {
using HttpHandler  = std::function<void(HttpRequest const&, HttpResponse&)>;
using HttpHandlers = std::vector<std::pair<std::regex, HttpHandler>>;

struct HttpServer final : public util::NonCopyable {
 public:
  HttpServer(EventLoop* loop, InetAddr const& addr, bool web_api = true);

  ~HttpServer() = default;

  void Start();
  void Stop();

  HttpServer& Get(std::string_view path, HttpHandler handler);
  HttpServer& Post(std::string_view path, HttpHandler handler);
  HttpServer& Put(std::string_view path, HttpHandler handler);
  HttpServer& Delete(std::string_view path, HttpHandler handler);

  void SetEventLoopGroupNum(size_t num) { tcp_server_.SetEventLoopGroupNum(num); }

 private:
  bool      web_api_{true};
  TcpServer tcp_server_;

  HttpHandlers get_handlers_;
  HttpHandlers post_handlers_;
  HttpHandlers put_handlers_;
  HttpHandlers delete_handlers_;

  static void OnConnection(TcpConnection* conn);
  void        OnMessage(TcpConnection* conn, util::MsgBuffer& buf, Timepoint const& receive_time);
  void        OnRequest(TcpConnection* conn, HttpRequest const& req);

  bool Route(HttpRequest const& req, HttpResponse& resp);

  static bool Dispatch(HttpRequest const& req, HttpResponse& resp, HttpHandlers const& handlers);
};
}  // namespace simple_http::net::http