target("simple_http_shared")
  set_kind("shared")
  set_basename("simple_http_$(mode)_$(arch)")

  add_files("**.cpp")

target("simple_http_static")
  set_kind("static")
  set_basename("simple_http_$(mode)_$(arch)")

  add_files("**.cpp")
