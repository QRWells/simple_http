#pragma once

#include "net/http/http.hpp"
#include "net/http/http_request.hpp"
#include "utils/msg_buffer.hpp"

namespace simple_http::net::http {
struct HttpContext {
 public:
  enum class HttpRequestParseState {
    kExpectRequestLine,
    kExpectHeaders,
    kExpectBody,
    kComplete,
  };

  HttpContext()  = default;
  ~HttpContext() = default;

  [[nodiscard]] HttpRequest const& GetRequest() const { return request_; }
  [[nodiscard]] HttpRequest&       GetRequest() { return request_; }

  [[nodiscard]] bool ParseRequest(util::MsgBuffer& buf, Timepoint receive_time);

  [[nodiscard]] bool Complete() const { return state_ == HttpRequestParseState::kComplete; }

  void Reset() {
    state_ = HttpRequestParseState::kExpectRequestLine;
    HttpRequest dummy;
    request_.Swap(dummy);
  }

 private:
  bool ProcessRequestLine(char const* begin, char const* end);

  HttpRequestParseState state_{};
  HttpRequest           request_;
};
}  // namespace simple_http::net::http