set_project("SimpleHttp")
set_version("0.1.0")

add_rules("mode.debug", "mode.release")

add_includedirs("include")
add_includedirs("src")

set_toolchains("gcc")
set_languages("c++20")

includes("test")
includes("src")
includes("example")