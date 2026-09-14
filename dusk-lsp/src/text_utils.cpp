#include "text_utils.h"

#include <sstream>

namespace dusk::lsp {

std::vector<std::string> splitLines(const std::string &source) {
  std::vector<std::string> lines;
  std::stringstream stream(source);
  std::string line;

  while (std::getline(stream, line)) {
    if (!line.empty() && line.back() == '\r')
      line.pop_back();

    lines.push_back(line);
  }

  if (!source.empty() && source.back() == '\n')
    lines.emplace_back();

  return lines;
}

} // namespace dusk::lsp
