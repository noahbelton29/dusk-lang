add_executable(dusk-lsp
  src/main.cpp
  src/lsp_server.cpp
  src/protocol.cpp
  src/diagnostics.cpp
  src/completion.cpp
  src/text_utils.cpp
)

target_include_directories(dusk-lsp PRIVATE
  ${PROJECT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${LLVM_INCLUDE_DIRS}
)

target_link_libraries(dusk-lsp PRIVATE
  dusk_lexer
  dusk_parser
  dusk_ast
  dusk_util
  LLVMSupport
)

target_compile_features(dusk-lsp PRIVATE cxx_std_20)
