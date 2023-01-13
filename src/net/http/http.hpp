#pragma once

#include <algorithm>
#include <chrono>
#include <map>
#include <sstream>
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
  auto const* p = s.data();
  while (*p != 0) {
    if (*p == '\r' || *p == '\n') {
      return true;
    }
    p++;
  }
  return false;
}

inline static constexpr auto kCrlf = "\r\n";

inline constexpr bool EnsureHeadingSlash(std::string_view& s) {
  if (s.empty()) {
    return false;
  }
  int i = 0;
  while (s[i] == '/') {
    i++;
  }
  if (i > 1) {
    s.remove_prefix(i - 1);
    return true;
  }
  return false;
}

inline constexpr void RemoveTrailingSlash(std::string_view& s) {
  if (s.empty()) {
    return;
  }
  while (s.ends_with('/')) {
    s.remove_suffix(1);
  }
}

inline auto NormalizePath(std::string_view path) {
  std::stringstream ss;
  if (!EnsureHeadingSlash(path)) {
    ss << '/';
  }
  RemoveTrailingSlash(path);
  ss << path;
  return ss.str();
}

const std::map<std::string, std::string> kMimeTypes{
    {".txt", "text/plain"},
    {".html", "text/html"},
    {".css", "text/css"},
    {".js", "application/javascript"},
    {".json", "application/json"},
    {".png", "image/png"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".gif", "image/gif"},
    {".ico", "image/x-icon"},
    {".svg", "image/svg+xml"},
    {".wav", "audio/wav"},
    {".mp3", "audio/mpeg"},
    {".mp4", "video/mp4"},
    {".avi", "video/x-msvideo"},
    {".doc", "application/msword"},
    {".pdf", "application/pdf"},
    {".zip", "application/zip"},
    {".gz", "application/gzip"},
    {".tar", "application/x-tar"},
    {".htm", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".ttf", "application/x-font-ttf"},
    {".otf", "application/x-font-opentype"},
    {".woff", "application/font-woff"},
    {".woff2", "application/font-woff2"},
    {".eot", "application/vnd.ms-fontobject"},
    {".sfnt", "application/font-sfnt"},
    {".bin", "application/octet-stream"},
    {".exe", "application/octet-stream"},
    {".dll", "application/octet-stream"},
    {".deb", "application/octet-stream"},
    {".dmg", "application/octet-stream"},
    {".iso", "application/octet-stream"},
    {".img", "application/octet-stream"},
    {".msi", "application/octet-stream"},
    {".msp", "application/octet-stream"},
    {".msm", "application/octet-stream"},
    {".mid", "audio/midi"},
    {".midi", "audio/midi"},
    {".kar", "audio/midi"},
    {".rtf", "application/rtf"},
    {".wasm", "application/wasm"},
    {".mjs", "application/javascript"},
    {".cjs", "application/javascript"},
    {".m3u", "audio/x-mpegurl"},
    {".ts", "video/mp2t"},
    {".ogv", "video/ogg"},
    {".ogx", "application/ogg"},
    {".ogm", "video/ogg"},
    {".oga", "audio/ogg"},
    {".ogg", "audio/ogg"},
    {".ogx", "application/ogg"},
    {".ogm", "video/ogg"},
    {".oga", "audio/oga"},
};

}  // namespace simple_http::net::http