#pragma once

#include <algorithm>
#include <chrono>
#include <map>
#include <string_view>

#include <cctype>

namespace simple_http::net::http {
using Timepoint = std::chrono::steady_clock::time_point;

enum class Method { kInvalid, kGet, kPost, kHead, kPut, kDelete };

inline static constexpr auto MethodName(Method method) {
  switch (method) {
    case Method::kGet:
      return "GET";
    case Method::kPost:
      return "POST";
    case Method::kHead:
      return "HEAD";
    case Method::kPut:
      return "PUT";
    case Method::kDelete:
      return "DELETE";
    default:
      return "UNKNOWN";
  }
}

inline static constexpr Method ParseMethod(std::string_view method) {
  if (method == "GET") {
    return Method::kGet;
  }
  if (method == "POST") {
    return Method::kPost;
  }
  if (method == "HEAD") {
    return Method::kHead;
  }
  if (method == "PUT") {
    return Method::kPut;
  }
  if (method == "DELETE") {
    return Method::kDelete;
  }
  return Method::kInvalid;
}

enum class Version { kUnknown, kHttp10, kHttp11 };

inline static constexpr auto VersionName(Version version) {
  switch (version) {
    case Version::kHttp10:
      return "HTTP/1.0";
    case Version::kHttp11:
      return "HTTP/1.1";
    default:
      return "UNKNOWN";
  }
}

inline static constexpr Version ParseVersion(std::string_view version) {
  if (version == "HTTP/1.0") {
    return Version::kHttp10;
  }
  if (version == "HTTP/1.1") {
    return Version::kHttp11;
  }
  return Version::kUnknown;
}

enum class StatusCode {
  kUnknown             = 0,
  k200Ok               = 200,
  k301MovedPermanently = 301,
  k400BadRequest       = 400,
  k404NotFound         = 404,
  k501NotImplemented   = 501
};

struct Ci {
  bool operator()(std::string_view s1, std::string_view s2) const {
    return std::lexicographical_compare(
        s1.begin(), s1.end(), s2.begin(), s2.end(),
        [](unsigned char c1, unsigned char c2) { return ::tolower(c1) < ::tolower(c2); });
  }
};

using Headers = std::map<std::string, std::string, Ci>;

inline constexpr bool HasCrlf(std::string_view s) {
  // return s.find('\r') != std::string_view::npos || s.find('\n') != std::string_view::npos;
  auto const *p = s.data();
  while (*p != 0) {
    if (*p == '\r' || *p == '\n') {
      return true;
    }
    p++;
  }
  return false;
}

inline static constexpr auto kCrlf = "\r\n";

inline constexpr auto RemoveTrailingSlash(std::string_view s) {
  if (s.empty()) {
    return s;
  }
  while (s.back() == '/') {
    s.remove_suffix(1);
  }
  return s;
}

}  // namespace simple_http::net::http