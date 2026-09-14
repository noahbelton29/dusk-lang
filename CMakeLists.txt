cmake_minimum_required(VERSION 3.16)
project(dusk CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options(-Wall -Wextra -Wpedantic)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-O3 -ffunction-sections -fdata-sections)
add_link_options(-Wl,--gc-sections)

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  add_link_options(-s)
endif()

include(CheckIPOSupported)
check_ipo_supported(RESULT IPO_SUPPORTED OUTPUT IPO_ERROR)
if (IPO_SUPPORTED)
  set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
else()
  message(STATUS "IPO/LTO not supported: ${IPO_ERROR}")
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
add_subdirectory(src/cli)
add_subdirectory(src/util)
add_subdirectory(dusk-lsp)

add_executable(duskc src/main.cpp)
target_include_directories(duskc PRIVATE include)
target_link_libraries(duskc PRIVATE
  dusk_lexer
  dusk_parser
  dusk_ast
  dusk_codegen
  dusk_cli
  dusk_util
)
