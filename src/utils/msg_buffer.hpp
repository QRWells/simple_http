#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <span>
#include <string_view>
#include <vector>

#include <cstddef>

#include <sys/types.h>

namespace simple_http::util {

static constexpr std::size_t kDefaultBufferSize = 1024;

struct MsgBuffer {
 public:
  explicit MsgBuffer(size_t size = kDefaultBufferSize);
  ~MsgBuffer() = default;

  [[nodiscard]] std::size_t           ReadableSize() const { return tail_ - head_; }
  [[nodiscard]] std::size_t           WritableSize() const { return buffer_.size() - tail_; }
  [[nodiscard]] bool                  Empty() const { return ReadableSize() == 0; }
  [[nodiscard]] char const*           Peek() const { return buffer_.data() + head_; }
  [[nodiscard]] std::span<char const> Data() const { return {Peek(), ReadableSize()}; }
  [[nodiscard]] char const*           BeginWrite() const { return Begin() + tail_; }
  [[nodiscard]] char*                 BeginWrite() { return Begin() + tail_; }

  [[nodiscard]] std::span<char const> Read(std::size_t size);
  [[nodiscard]] ssize_t               ReadFile(int fd, int* saved_errno);

  template <std::integral T>
  [[nodiscard]] T Read() {
    auto data = Read(sizeof(T));
    return *reinterpret_cast<T const*>(data.data());
  }
  template <int N>
  void Write(std::array<char, N> const& buf) {
    assert(strnlen(buf, N) == N - 1);
    Write(buf, N - 1);
  }
  void Write(char const* data, std::size_t size);
  void Write(std::span<char const> data);
  void Write(MsgBuffer const& other);

  template <typename T>
  void Write(T data)
    requires std::integral<T>
  {
    Write(reinterpret_cast<char const*>(&data), sizeof(T));
  }

  void RetrieveAll() { tail_ = head_ = 0; }
  void RetrieveUntil(char const* end) { Retrieve(end - Peek()); }
  void Retrieve(std::size_t size) {
    if (size >= ReadableSize()) {
      RetrieveAll();
      return;
    }
    head_ += size;
  }

  void EnsureSize(std::size_t size);
  void Compact();
  void Clear() {
    head_ = 0;
    tail_ = 0;
  }

  void Swap(MsgBuffer& other) {
    using std::swap;
    swap(head_, other.head_);
    swap(tail_, other.tail_);
    buffer_.swap(other.buffer_);
  }

  [[nodiscard]] char const* FindCRLF() const {
    static constexpr auto kCRLF = "\r\n";
    //
    auto const* it = std::search(Peek(), BeginWrite(), kCRLF, kCRLF + 2);
    return it == BeginWrite() ? nullptr : it;
  }

 private:
  std::size_t       head_{0};
  std::size_t       tail_{0};
  std::vector<char> buffer_{};

  [[nodiscard]] char const* Begin() const { return buffer_.data(); }
  [[nodiscard]] char*       Begin() { return buffer_.data(); }
};
}  // namespace simple_http::util