set_kind("binary")
add_syslinks("pthread")

target("example_tcp_server")
  add_deps("simple_http_static")
  add_files("tcp_server.cpp")

target("example_http_server")
  add_deps("simple_http_static")
  add_files("http_server.cpp")

target("example_web_api")
  add_deps("simple_http_static")
  add_files("web_api.cpp")