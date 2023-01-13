# simple_http

A simple http server library written in C++20.

## Usage
### Build the library
Requires GCC 12 or higher.

```bash
make lib -j
```
The library will be built in the `./build/lib` directory.
### Build the example

```bash
gcc -std=c++20 -lsimple_http -lpthread -o http_server http_server.cpp
```
## Example
All examples are in the [examples](https://github.com/QRWells/simple_http/tree/v0.1.0/example) directory.

### HTTP Server

For http server, the `wwwroot` directory is the root directory of the server to serve static files. The default port is 80. 

```cpp
#include <net/http/http_server.hpp>

using namespace std;
using namespace simple_http::net;
using namespace simple_http::net::http;

int main(int argc, char* argv[]) {
  EventLoopThread thread;
  thread.Run();

  http::HttpServer server{thread.GetLoop()};

  server.Start();

  thread.Wait();
  return 0;
}
```

### Web API

For web api, it won't serve static files. The default port is 80.

```cpp
#include <net/http/http_server.hpp>

using namespace std;
using namespace simple_http::net;
using namespace simple_http::net::http;

int main(int argc, char* argv[]) {
  EventLoopThread thread;
  thread.Run();

  http::HttpServer server{thread.GetLoop()};

  server.Get("hello", [](HttpRequest const& req, HttpResponse& resp) {
    resp.SetStatusCode(200);
    resp.SetStatusMessage("OK");
    resp.SetBody("Hello World!\n");
  });

  server.Start();

  thread.Wait();
  return 0;
}
```
### TCP Server

```cpp
#include <net/event_loop_thread.hpp>
#include <net/inet_addr.hpp>
#include <net/tcp_server.hpp>

using namespace std;
using namespace simple_http::net;

int main(int argc, char *argv[]) {
  EventLoopThread thread;
  thread.Run();

  InetAddr addr{1234};

  TcpServer server{thread.GetLoop(), addr};
  server.SetEventLoopGroupNum(2);
  server.Start();
  thread.Wait();
  return 0;
}
```

## License

[MIT](LICENSE)