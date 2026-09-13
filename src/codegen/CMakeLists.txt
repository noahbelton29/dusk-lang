add_library(dusk_codegen STATIC
  codegen.cpp
  stmt_codegen.cpp
  expr_codegen.cpp
  object_emit.cpp
)
target_include_directories(dusk_codegen PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(dusk_codegen PUBLIC /usr/lib/libLLVM.so)
