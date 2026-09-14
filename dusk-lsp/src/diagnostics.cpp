#include "diagnostics.h"
#include "text_utils.h"

#include <cstddef>
#include <utility>

namespace dusk::lsp {

namespace {
constexpr int ERROR_SEVERITY = 1;
}

std::vector<Diagnostic> diagnose(const std::string &source) {
  std::vector<Diagnostic> diagnostics;
  const auto lines = splitLines(source);

  int braceDepth = 0;

  for (std::size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex) {
    const std::string &line = lines[lineIndex];

    for (char c : line) {
      if (c == '{')
        ++braceDepth;
      else if (c == '}')
        --braceDepth;
    }

    if (line.find_first_not_of(" \t\r") == std::string::npos)
      continue;

    if (line.find("fn ") != std::string::npos &&
        line.find('{') != std::string::npos)
      continue;

    if (line.find("use ") != std::string::npos) {
      if (line.find(';') == std::string::npos) {
        diagnostics.push_back({static_cast<int>(lineIndex),
                               static_cast<int>(line.size()), 0,
                               "expected ';' after expression"});
      }

      continue;
    }

    if (line.find('}') != std::string::npos)
      continue;

    if (braceDepth > 0) {
      std::string trimmed = line;
      const auto first = trimmed.find_first_not_of(" \t");

      if (first != std::string::npos)
        trimmed.erase(0, first);

      const auto last = trimmed.find_last_not_of(" \t\r");

      if (last != std::string::npos)
        trimmed.erase(last + 1);
      else
        trimmed.clear();

      if (!trimmed.empty() && trimmed.back() != ';' && trimmed.back() != '{') {
        diagnostics.push_back({static_cast<int>(lineIndex),
                               static_cast<int>(line.size()), 0,
                               "expected ';' after expression"});
      }
    }
  }

  if (braceDepth > 0) {
    diagnostics.push_back({static_cast<int>(lines.size()), 0, 0,
                           "expected '}' to close function body"});
  }

  return diagnostics;
}

llvm::json::Array makeDiagnostics(const std::vector<Diagnostic> &diagnostics) {
  llvm::json::Array result;

  for (const auto &diagnostic : diagnostics) {
    llvm::json::Object range{
        {"start", llvm::json::Object{{"line", diagnostic.line},
                                     {"character", diagnostic.character}}},
        {"end", llvm::json::Object{
                    {"line", diagnostic.line},
                    {"character", diagnostic.character + diagnostic.length}}}};

    result.push_back(llvm::json::Object{{"range", std::move(range)},
                                        {"severity", ERROR_SEVERITY},
                                        {"source", "dusk"},
                                        {"message", diagnostic.message}});
  }

  return result;
}

} // namespace dusk::lsp
