target("example_tcp_server")
  set_default(true)
  set_kind("binary")

  add_deps("static_lib")

  add_files("tcp_server.cpp")

  add_syslinks("pthread")

target("example_http_server")
  set_default(true)
  set_kind("binary")

  add_deps("static_lib")

  add_files("http_server.cpp")

  add_syslinks("pthread")