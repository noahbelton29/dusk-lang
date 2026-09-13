add_library(dusk_lexer STATIC lexer.cpp)
target_include_directories(dusk_lexer PUBLIC ${CMAKE_SOURCE_DIR}/include)
