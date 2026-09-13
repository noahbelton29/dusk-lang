add_library(dusk_util STATIC file_utils.cpp diagnostics.cpp)
target_include_directories(dusk_util PUBLIC ${CMAKE_SOURCE_DIR}/include)
