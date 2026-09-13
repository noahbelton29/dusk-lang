add_library(dusk_cli STATIC cli.cpp)
target_include_directories(dusk_cli PUBLIC ${CMAKE_SOURCE_DIR}/include)
