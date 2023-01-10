#include <stdexcept>

#include <netinet/in.h>
#include <sys/uio.h>

#include "msg_buffer.hpp"

namespace simple_http::util {

using DiffType = std::vector<char>::iterator::difference_type;

constexpr DiffType ToDiff(std::size_t size) { return static_cast<DiffType>(size); }

MsgBuffer::MsgBuffer(size_t size) : buffer_(size) {}

void MsgBuffer::Write(char const* data, std::size_t size) {
  EnsureSize(size);
  std::copy(data, data + size, buffer_.begin() + static_cast<DiffType>(tail_));
  tail_ += size;
}

void MsgBuffer::Write(std::span<char const> data) { Write(data.data(), data.size()); }

void MsgBuffer::Write(MsgBuffer const& other) {
  EnsureSize(other.ReadableSize());
  std::copy(other.buffer_.begin() + ToDiff(other.head_), other.buffer_.begin() + ToDiff(other.tail_),
            buffer_.begin() + ToDiff(tail_));
}

std::span<char const> MsgBuffer::Read(std::size_t size) {
  if (size > ReadableSize()) {
    size = ReadableSize();
  }
  auto ret = std::span<char const>(Peek(), size);
  head_    += size;
  return ret;
}

ssize_t MsgBuffer::ReadFile(int fd, int* saved_errno) {
  char         ext_buffer[8192];  // NOLINT
  struct iovec vec[2];            // NOLINT
  size_t       writable = WritableSize();
  vec[0].iov_base       = Begin() + tail_;
  vec[0].iov_len        = static_cast<int>(writable);
  vec[1].iov_base       = ext_buffer;
  vec[1].iov_len        = sizeof(ext_buffer);
  int const iovcnt      = (writable < sizeof ext_buffer) ? 2 : 1;
  ssize_t   n           = ::readv(fd, vec, iovcnt);
  if (n < 0) {
    *saved_errno = errno;
  } else if (static_cast<size_t>(n) <= writable) {
    tail_ += n;
  } else {
    tail_ = buffer_.size();
    Write(ext_buffer, n - writable);
  }
  return n;
}

void MsgBuffer::EnsureSize(std::size_t size) {
  if (WritableSize() >= size) {
    return;
  }
  if (head_ + WritableSize() >= size) {
    Compact();
    return;
  }
  size_t new_len;
  if ((buffer_.size() * 2) > (ReadableSize() + size)) {
    new_len = buffer_.size() * 2;
  } else {
    new_len = ReadableSize() + size;
  }
  MsgBuffer newbuffer(new_len);
  newbuffer.Write(*this);
  Swap(newbuffer);
}

void MsgBuffer::Compact() {
  if (head_ == 0) {
    return;
  }
  if (tail_ == head_) {
    return Clear();
  }
  std::copy(buffer_.begin() + ToDiff(head_), buffer_.begin() + ToDiff(tail_), buffer_.begin());
  tail_ -= head_;
  head_ = 0;
}

}  // namespace simple_http::util