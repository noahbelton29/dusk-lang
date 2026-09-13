add_library(dusk_ast STATIC ast_printer.cpp)
target_include_directories(dusk_ast PUBLIC ${CMAKE_SOURCE_DIR}/include)
