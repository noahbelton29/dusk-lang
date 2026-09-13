add_library(dusk_parser STATIC
  parser.cpp
  stmt_parser.cpp
  expr_parser.cpp
)
target_include_directories(dusk_parser PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(dusk_parser PUBLIC dusk_lexer)
