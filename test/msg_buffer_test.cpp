#include <iostream>
#include <string_view>

#include <cstdint>

#include "test.hpp"

#include "utils/msg_buffer.hpp"

int main(int argc, char* const argv[]) {
  using simple_http::util::MsgBuffer;
  using namespace std::literals;

  MsgBuffer buf(128);

  Equals(buf.ReadableSize(), 0);
  Equals(buf.WritableSize(), 128);

  buf.Write("Hello, world!"sv);

  Equals(buf.ReadableSize(), 13);
  Equals(buf.WritableSize(), 115);

  buf.Write(25);

  Equals(buf.ReadableSize(), 17);

  Equals(buf.Read(13), "Hello, world!"sv);
  Equals(buf.Read<uint32_t>(), 25);

  buf.Compact();

  Equals(buf.ReadableSize(), 0);
  Equals(buf.WritableSize(), 121);

  auto [pass, total] = GetTestInfo();

  if (pass != total) {
    std::cout << "Passed " << pass << " out of " << total << " tests." << std::endl;
  } else {
    std::cout << "All tests passed!" << std::endl;
  }

  return 0;
}