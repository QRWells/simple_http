#include <stdexcept>

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