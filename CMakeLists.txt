cmake_minimum_required(VERSION 3.16)
project(dusk CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST UNIX_COMMAND "${LLVM_DEFINITIONS}")
add_definitions(${LLVM_DEFINITIONS_LIST})

add_subdirectory(src/lexer)
add_subdirectory(src/parser)
add_subdirectory(src/ast)
add_subdirectory(src/codegen)

add_executable(duskc src/main.cpp)
target_include_directories(duskc PRIVATE include)
target_link_libraries(duskc PRIVATE
  dusk_lexer
  dusk_parser
  dusk_ast
  dusk_codegen
)
