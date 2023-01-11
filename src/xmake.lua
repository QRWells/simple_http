target("shared_lib")
  set_kind("shared")

  add_files("**.cpp")

target("static_lib")
  set_kind("static")

  add_files("**.cpp")
