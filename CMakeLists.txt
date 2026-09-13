cmake_minimum_required(VERSION 3.16)
project(dusk CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (MVSC)
  add_compile_options(/W4) 
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

add_executable(duskc
  src/main.cpp
  src/lexer.cpp
  src/parser.cpp
  src/ast_printer.cpp
)

target_include_directories(duskc PRIVATE src)
