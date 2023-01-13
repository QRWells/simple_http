#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>

#include <sys/stat.h>
#include <unistd.h>

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

HttpServer::HttpServer(EventLoop* loop, bool web_api, InetAddr const& addr)
    : web_api_{web_api}, tcp_server_{loop, addr} {
  tcp_server_.OnConnection([this](std::shared_ptr<TcpConnection> const& conn) { OnConnection(conn.get()); });
  tcp_server_.OnReceiveMessage([this](std::shared_ptr<TcpConnection> const& conn, MsgBuffer& buf) {
    OnMessage(conn.get(), buf, std::chrono::steady_clock::now());
  });
  if (!web_api) {
    struct stat st;
    if (stat("wwwroot", &st) == -1) {
      mkdir("wwwroot", 0755);
      // create index.html
      std::ofstream ofs("wwwroot/index.html", std::ios::trunc);
      ofs << R"(<html>
<head>
<title>This is title</title>
</head>
<body>
<h1>Hello</h1>
</body>
</html>)";
      ofs.close();
    }
    ::chdir("wwwroot");

    Get("", [](HttpRequest const& /*unused*/, HttpResponse& resp) {
      resp.SetStatusCode(StatusCode::k200Ok);
      resp.SetStatusMessage("OK");
      resp.SetContentType("text/html");
      resp.SetFileBody("index.html");
    });
  }
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
    if (web_api_ || req.GetPath() == "/") {
      return Dispatch(req, resp, get_handlers_);
    }

    std::string_view path = req.GetPath();
    if (path[0] == '/') {
      path.remove_prefix(1);
    }

    std::ifstream         file(path.data(), std::ios::binary);
    std::filesystem::path file_path(path.data());
    if (file.is_open()) {
      resp.SetStatusCode(StatusCode::k200Ok);
      resp.SetStatusMessage("OK");
      auto extension = file_path.extension().string();
      if (file_path.has_extension() && kMimeTypes.contains(extension)) {
        resp.SetContentType(kMimeTypes.at(extension));
      } else {
        resp.SetContentType("text/plain");
      }
      resp.SetFileBody(path);
      return true;
    }
    return false;
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

HttpServer& HttpServer::Get(std::string_view path, HttpHandler handler) {
  get_handlers_.emplace_back(std::regex{NormalizePath(path)}, std::move(handler));
  return *this;
}

HttpServer& HttpServer::Post(std::string_view path, HttpHandler handler) {
  post_handlers_.emplace_back(std::regex{NormalizePath(path)}, std::move(handler));
  return *this;
}

HttpServer& HttpServer::Put(std::string_view path, HttpHandler handler) {
  put_handlers_.emplace_back(std::regex{NormalizePath(path)}, std::move(handler));
  return *this;
}

HttpServer& HttpServer::Delete(std::string_view path, HttpHandler handler) {
  delete_handlers_.emplace_back(std::regex{NormalizePath(path)}, std::move(handler));
  return *this;
}

}  // namespace simple_http::net::http