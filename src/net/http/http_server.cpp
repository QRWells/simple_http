#include <chrono>
#include <iostream>

#include "http_context.hpp"
#include "http_server.hpp"
#include "net/http/http.hpp"
#include "utils/msg_buffer.hpp"

namespace simple_http::net::http {

using util::MsgBuffer;

void DefaultHttpCallback(HttpRequest const& /*unused*/, HttpResponse& resp) {
  resp.SetStatusCode(StatusCode::k404NotFound);
  resp.SetStatusMessage("Not Found");
  resp.SetCloseConnection(true);
}

HttpServer::HttpServer(EventLoop* loop, InetAddr const& addr) : tcp_server_{loop, addr} {
  tcp_server_.OnConnection([this](std::shared_ptr<TcpConnection> const& conn) { OnConnection(conn.get()); });
  tcp_server_.OnReceiveMessage([this](std::shared_ptr<TcpConnection> const& conn, MsgBuffer& buf) {
    OnMessage(conn.get(), buf, std::chrono::steady_clock::now());
  });
}

void HttpServer::Start() { tcp_server_.Start(); }

void HttpServer::Stop() { tcp_server_.Stop(); }

void HttpServer::OnConnection(TcpConnection* conn) {
  if (conn->IsConnected()) {
    conn->SetContext(HttpContext());
  }
}

void HttpServer::OnMessage(TcpConnection* conn, util::MsgBuffer& buf, Timepoint const& receive_time) {
  auto context = std::any_cast<HttpContext>(conn->GetContext());

  if (!context.ParseRequest(buf, receive_time)) {
    conn->Send("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->Shutdown();
  }

  if (context.Complete()) {
    OnRequest(conn, context.GetRequest());
    context.Reset();
  }
}

void HttpServer::OnRequest(TcpConnection* conn, HttpRequest const& req) {
  auto connection = req.GetHeader("Connection");
  auto close      = connection == "close" || (req.GetVersion() == Version::kHttp10 && connection != "Keep-Alive");

  HttpResponse response(close);
  if (!Route(req, response)) {
    DefaultHttpCallback(req, response);
  }
  MsgBuffer buf;
  response.WriteTo(buf);
  conn->Send(buf);
  if (response.IsCloseConnection()) {
    conn->Shutdown();
  }
}

bool HttpServer::Route(HttpRequest const& req, HttpResponse& resp) {
  auto const& method = req.GetMethod();

  if (method == Method::kGet) {
    return Dispatch(req, resp, get_handlers_);
  }

  if (method == Method::kPost) {
    return Dispatch(req, resp, post_handlers_);
  }

  if (method == Method::kPut) {
    return Dispatch(req, resp, put_handlers_);
  }

  if (method == Method::kDelete) {
    return Dispatch(req, resp, delete_handlers_);
  }

  resp.SetStatusCode(StatusCode::k400BadRequest);

  return false;
}

bool HttpServer::Dispatch(HttpRequest const& req, HttpResponse& resp, HttpHandlers const& handlers) {
  for (auto const& x : handlers) {
    auto const& pattern = x.first;
    auto const& handler = x.second;

    std::smatch matches;

    if (std::regex_match(req.GetPath(), matches, pattern)) {
      handler(req, resp);
      return true;
    }
  }

  return false;
}

void HttpServer::Get(std::string_view path, HttpHandler handler) {
  get_handlers_.emplace_back(std::regex{NormalizePath(path)}, std::move(handler));
}

void HttpServer::Post(std::string_view path, HttpHandler handler) {
  post_handlers_.emplace_back(std::regex{NormalizePath(path)}, std::move(handler));
}

void HttpServer::Put(std::string_view path, HttpHandler handler) {
  put_handlers_.emplace_back(std::regex{NormalizePath(path)}, std::move(handler));
}

void HttpServer::Delete(std::string_view path, HttpHandler handler) {
  delete_handlers_.emplace_back(std::regex{NormalizePath(path)}, std::move(handler));
}

}  // namespace simple_http::net::http