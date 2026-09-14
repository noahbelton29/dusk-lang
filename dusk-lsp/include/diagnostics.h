#pragma once

#include <string>
#include <vector>

#include <llvm/Support/JSON.h>

namespace dusk::lsp {

struct Diagnostic {
  int line;
  int character;
  int length;
  std::string message;
};

std::vector<Diagnostic> diagnose(const std::string &source);
llvm::json::Array makeDiagnostics(const std::vector<Diagnostic> &diagnostics);

} // namespace dusk::lsp
