#pragma once

#include <string>
#include <string_view>

#include <cstdint>
#include <cstring>

#include <arpa/inet.h>
#include <netinet/in.h>

static inline constexpr in_addr_t kInaddrAny      = INADDR_ANY;
static inline constexpr in_addr_t kInaddrLoopback = INADDR_LOOPBACK;

namespace simple_http::net {
struct InetAddr {
 public:
  InetAddr(uint16_t port = 0, bool loopback_only = false) : is_valid_(true) {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family      = AF_INET;
    in_addr_t ip          = loopback_only ? kInaddrLoopback : kInaddrAny;
    addr_.sin_addr.s_addr = htonl(ip);
    addr_.sin_port        = htons(port);
  }

  InetAddr(std::string_view ip, uint16_t port) : is_valid_(true) {
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port   = htons(port);
    if (::inet_pton(AF_INET, ip.data(), &addr_.sin_addr) <= 0) {
      return;
    }
  }

  explicit InetAddr(const struct sockaddr_in& addr) : addr_(addr), is_valid_(true) {}

  [[nodiscard]] sa_family_t            GetFamily() const { return addr_.sin_family; }
  [[nodiscard]] uint32_t               GetIpInNetEndian() const { return addr_.sin_addr.s_addr; }
  [[nodiscard]] uint16_t               GetPortInNetEndian() const { return addr_.sin_port; }
  [[nodiscard]] uint16_t               GetPort() const { return ntohs(addr_.sin_port); }
  [[nodiscard]] const struct sockaddr* GetSockAddr() const { return reinterpret_cast<const struct sockaddr*>(&addr_); }

  [[nodiscard]] std::string ToIp() const {
    char buf[32];  // NOLINT
    ::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf));
    return buf;
  }

  [[nodiscard]] std::string ToIpPort() const {
    char     buf[64] = "";  // NOLINT
    uint16_t port    = ntohs(addr_.sin_port);
    snprintf(buf, sizeof(buf), ":%u", port);
    return ToIp() + std::string(buf);
  }

  void SetSockAddr(const struct sockaddr_in& addr) {
    addr_     = addr;
    is_valid_ = true;
  }
  void SetIpInNetEndian(uint32_t ip) { addr_.sin_addr.s_addr = ip; }
  void SetPortInNetEndian(uint16_t port) { addr_.sin_port = port; }

  [[nodiscard]] bool IsValid() const { return is_valid_; }

 private:
  struct sockaddr_in addr_ {};
  bool               is_valid_{false};
};
}  // namespace simple_http::net