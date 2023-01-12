#pragma once

#include <sstream>
#include <string>
#include <string_view>

#include "net/http/http.hpp"
#include "utils/msg_buffer.hpp"

namespace simple_http::net::http {
struct HttpResponse {
 public:
  explicit HttpResponse(bool close) : closeConnection_(close) {}

  [[nodiscard]] std::string_view GetBody() const { return body_; }
  [[nodiscard]] std::string_view GetStatusMessage() const { return statusMessage_; }

  void SetStatusCode(StatusCode code) { statusCode_ = code; }
  void SetStatusCode(unsigned int code) { statusCode_ = static_cast<StatusCode>(code); }
  void SetStatusMessage(std::string_view message) { statusMessage_ = message; }
  void SetCloseConnection(bool on) { closeConnection_ = on; }
  void SetContentType(std::string_view content_type) { SetHeader("Content-Type", content_type); }
  void SetBody(std::string_view body) { body_ = body; }

  [[nodiscard]] bool IsCloseConnection() const { return closeConnection_; }

  void SetHeader(std::string_view key, std::string_view val) {
    if (key.empty()) {
      return;
    }
    if (HasCrlf(key) || HasCrlf(val)) {
      return;
    }

    headers_.emplace(key, val);
  }

  void WriteTo(util::MsgBuffer& output) const {
    std::stringstream ss;

    ss << "HTTP/1.1 " << static_cast<int>(statusCode_) << " " << statusMessage_ << "\r\n";

    if (closeConnection_) {
      ss << "Connection: close\r\n";
    } else {
      ss << "Content-Length: " << body_.size() << "\r\n"
         << "Connection: Keep-Alive\r\n";
    }

    for (auto const& header : headers_) {
      ss << header.first << ": " << header.second << "\r\n";
    }

    ss << "\r\n" << body_;

    output.Write(ss.str());
  }

 private:
  Headers     headers_;
  StatusCode  statusCode_{};
  Version     version_{Version::kHttp10};
  std::string statusMessage_;
  bool        closeConnection_;
  std::string body_;
};
}  // namespace simple_http::net::http