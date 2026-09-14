#pragma once

#include <string>

#include <llvm/Support/JSON.h>

namespace dusk::lsp {

std::string getCurrentLine(const std::string &source, int line);

llvm::json::Array makeCompletions(const std::string &source,
                                  const std::string &line, int character);

} // namespace dusk::lsp
