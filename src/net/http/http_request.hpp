#pragma once

#include <chrono>
#include <map>
#include <string>
#include <string_view>
#include <type_traits>

#include "net/http/http.hpp"

namespace simple_http::net::http {
struct HttpRequest {
 public:
  HttpRequest() = default;

  [[nodiscard]] auto const& GetVersion() const { return version_; }
  [[nodiscard]] auto const& GetMethod() const { return method_; }
  [[nodiscard]] auto const& GetPath() const { return path_; }
  [[nodiscard]] auto const& GetQuery() const { return query_; }
  [[nodiscard]] auto const& GetReceiveTime() const { return receiveTime_; }
  [[nodiscard]] auto const& GetHeaders() const { return headers_; }

  void SetVersion(Version v) { version_ = v; }
  void SetMethod(Method m) { method_ = m; }
  void SetPath(std::string_view path) { path_ = path; }
  void SetQuery(std::string_view query) { query_ = query; }
  void SetReceiveTime(std::chrono::steady_clock::time_point time) { receiveTime_ = time; }
  void SetHeaders(Headers headers) { headers_ = std::move(headers); }

  [[nodiscard]] bool TrySetVersion(std::string_view version) {
    version_ = ParseVersion(version);
    return version_ != Version::kUnknown;
  }

  [[nodiscard]] bool TrySetMethod(std::string_view method) {
    method_ = ParseMethod(method);
    return method_ != Method::kInvalid;
  }

  [[nodiscard]] constexpr auto VersionString() const { return VersionName(version_); }
  [[nodiscard]] constexpr auto MethodString() const { return MethodName(method_); }

  void SetHeader(std::string_view key, std::string_view val) {
    if (key.empty()) {
      return;
    }
    if (HasCrlf(key) || HasCrlf(val)) {
      return;
    }

    headers_.emplace(key, val);
  }

  [[nodiscard]] std::string GetHeader(std::string_view key) const {
    auto it = headers_.find(std::string{key});
    if (it != headers_.end()) {
      return it->second;
    }
    return {};
  }

  void RemoveHeader(std::string_view key) {
    auto it = headers_.find(std::string{key});
    if (it != headers_.end()) {
      headers_.erase(it);
    }
  }

  [[nodiscard]] bool HasHeader(std::string_view key) const { return headers_.find(std::string{key}) != headers_.end(); }

  void Swap(HttpRequest& that) {
    std::swap(method_, that.method_);
    std::swap(version_, that.version_);
    path_.swap(that.path_);
    query_.swap(that.query_);
    std::swap(receiveTime_, that.receiveTime_);
    headers_.swap(that.headers_);
  }

 private:
  Method                                method_{};
  std::string                           path_;
  Headers                               headers_;
  std::string                           body_;
  Version                               version_{};
  std::string                           query_;
  std::chrono::steady_clock::time_point receiveTime_;
};
}  // namespace simple_http::net::http