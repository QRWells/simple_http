#include "net/http/http.hpp"
#include "net/http/http_server.hpp"
#include "utils/msg_buffer.hpp"

#include "http_context.hpp"

namespace simple_http::net::http {
bool HttpContext::ProcessRequestLine(char const* begin, char const* end) {
  bool        succeed = false;
  char const* start   = begin;
  char const* space   = std::find(start, end, ' ');
  if (space != end && request_.TrySetMethod({start, space})) {
    start = space + 1;
    space = std::find(start, end, ' ');
    if (space != end) {
      char const* question = std::find(start, space, '?');
      if (question != space) {
        request_.SetPath({start, question});
        request_.SetQuery({question, space});
      } else {
        request_.SetPath({start, space});
      }
      start   = space + 1;
      succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
      if (succeed) {
        if (*(end - 1) == '1') {
          request_.SetVersion(Version::kHttp11);
        } else if (*(end - 1) == '0') {
          request_.SetVersion(Version::kHttp10);
        } else {
          succeed = false;
        }
      }
    }
  }
  return succeed;
}

// return false if any error
bool HttpContext::ParseRequest(util::MsgBuffer& buf, Timepoint receive_time) {
  bool ok       = true;
  bool has_more = true;
  while (has_more) {
    if (state_ == HttpRequestParseState::kExpectRequestLine) {
      auto const* crlf = buf.FindCRLF();
      if (crlf != nullptr) {
        ok = ProcessRequestLine(buf.Peek(), crlf);
        if (ok) {
          request_.SetReceiveTime(receive_time);
          buf.RetrieveUntil(crlf + 2);
          state_ = HttpRequestParseState::kExpectHeaders;
        } else {
          has_more = false;
        }
      } else {
        has_more = false;
      }
    } else if (state_ == HttpRequestParseState::kExpectHeaders) {
      auto const* crlf = buf.FindCRLF();
      if (crlf != nullptr) {
        auto const* colon = std::find(buf.Peek(), crlf, ':');
        if (colon != crlf) {
          request_.SetHeader({buf.Peek(), colon}, {colon + 2, crlf});
        } else {
          state_   = HttpRequestParseState::kComplete;
          has_more = false;
        }
        buf.RetrieveUntil(crlf + 2);
      } else {
        has_more = false;
      }
    } else if (state_ == HttpRequestParseState::kExpectBody) {
      // TODO
    }
  }
  return ok;
}
}  // namespace simple_http::net::http