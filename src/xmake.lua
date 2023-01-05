target("server")
  set_default(true)
  set_kind("binary")

  add_files("src/**.cpp")

  add_syslinks("pthread")

target("shared_lib")
  set_kind("shared")

  add_files("**.cpp")
  remove_files("src/main.cpp")

target("static_lib")
  set_kind("static")

  add_files("**.cpp")
  remove_files("src/main.cpp")