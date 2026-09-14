#include "completion.h"
#include "text_utils.h"

#include <cctype>
#include <cstddef>
#include <string_view>
#include <utility>
#include <vector>

namespace dusk::lsp {

namespace {

constexpr int COMPLETION_TEXT = 1;
constexpr int COMPLETION_METHOD = 2;
constexpr int COMPLETION_MODULE = 9;
constexpr int COMPLETION_KEYWORD = 14;
constexpr int COMPLETION_SNIPPET = 15;

constexpr int INSERT_TEXT_PLAIN = 1;
constexpr int INSERT_TEXT_SNIPPET = 2;

struct Completion {
  std::string label;
  int kind;
  std::string detail;
  std::string insertText;
  int insertTextFormat = INSERT_TEXT_PLAIN;
  bool autoImportStdIo = false;
};

bool isIdentifierCharacter(char c) {
  return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

std::string getCompletionExpression(const std::string &line, int character) {
  if (character < 0)
    return {};

  const std::size_t cursor = static_cast<std::size_t>(character);

  if (cursor > line.size())
    return {};

  std::string beforeCursor = line.substr(0, cursor);
  const auto first = beforeCursor.find_first_not_of(" \t");

  if (first != std::string::npos && beforeCursor.substr(first, 4) == "use ")
    return beforeCursor.substr(first + 4);

  std::size_t start = cursor;

  while (start > 0) {
    const char c = line[start - 1];

    if (!isIdentifierCharacter(c) && c != '.')
      break;

    --start;
  }

  return line.substr(start, cursor - start);
}

std::string getCompletionPrefix(const std::string &expression) {
  const std::size_t dot = expression.find_last_of('.');

  if (dot == std::string::npos)
    return expression;

  return expression.substr(dot + 1);
}

std::vector<std::string> getCompletionPath(const std::string &expression) {
  std::vector<std::string> parts;
  std::size_t start = 0;

  while (start <= expression.size()) {
    const std::size_t dot = expression.find('.', start);

    if (dot == std::string::npos) {
      if (start < expression.size())
        parts.push_back(expression.substr(start));

      break;
    }

    parts.push_back(expression.substr(start, dot - start));
    start = dot + 1;

    if (start == expression.size())
      break;
  }

  return parts;
}

bool startsWith(std::string_view value, std::string_view prefix) {
  return value.starts_with(prefix);
}

bool hasImport(const std::string &source, std::string_view module) {
  const auto lines = splitLines(source);

  for (const auto &line : lines) {
    std::string trimmed = line;
    const auto first = trimmed.find_first_not_of(" \t");

    if (first != std::string::npos)
      trimmed.erase(0, first);

    if (!trimmed.starts_with("use "))
      continue;

    std::string imported = trimmed.substr(4);
    const auto semicolon = imported.find(';');

    if (semicolon != std::string::npos)
      imported.erase(semicolon);

    if (imported == module)
      return true;
  }

  return false;
}

bool isUseStatement(const std::string &line, int character) {
  if (character < 0)
    return false;

  const std::size_t cursor = static_cast<std::size_t>(character);

  if (cursor > line.size())
    return false;

  const std::string before = line.substr(0, cursor);
  const auto first = before.find_first_not_of(" \t");

  if (first == std::string::npos)
    return false;

  return before.substr(first, 4) == "use ";
}

void addCompletion(llvm::json::Array &items, const Completion &completion,
                   const std::string &source) {
  llvm::json::Object item{{"label", completion.label},
                          {"kind", completion.kind},
                          {"detail", completion.detail}};

  if (!completion.insertText.empty()) {
    item["insertText"] = completion.insertText;
    item["insertTextFormat"] = completion.insertTextFormat;
  }

  if (completion.autoImportStdIo && !hasImport(source, "std.io")) {
    llvm::json::Object edit{
        {"range",
         llvm::json::Object{
             {"start", llvm::json::Object{{"line", 0}, {"character", 0}}},
             {"end", llvm::json::Object{{"line", 0}, {"character", 0}}}}},
        {"newText", "use std.io;\n"}};

    item["additionalTextEdits"] = llvm::json::Array{std::move(edit)};
  }

  items.push_back(std::move(item));
}

} // namespace

std::string getCurrentLine(const std::string &source, int line) {
  int currentLine = 0;
  std::size_t start = 0;

  for (std::size_t i = 0; i < source.size(); ++i) {
    if (source[i] != '\n')
      continue;

    if (currentLine == line)
      return source.substr(start, i - start);

    start = i + 1;
    ++currentLine;
  }

  if (currentLine == line)
    return source.substr(start);

  return {};
}

llvm::json::Array makeCompletions(const std::string &source,
                                  const std::string &line, int character) {
  llvm::json::Array items;

  const std::string expression = getCompletionExpression(line, character);
  const std::string prefix = getCompletionPrefix(expression);
  const auto path = getCompletionPath(expression);
  const bool inUse = isUseStatement(line, character);

  if (inUse) {
    const auto firstDot = expression.find('.');

    if (firstDot == std::string::npos) {
      if (startsWith("std", expression)) {
        addCompletion(items,
                      {"std", COMPLETION_MODULE, "Dusk standard library", "std",
                       INSERT_TEXT_PLAIN},
                      source);
      }

      return items;
    }

    const std::string module = expression.substr(0, firstDot);
    const std::string modulePrefix = expression.substr(firstDot + 1);

    if (module == "std" && startsWith("io", modulePrefix)) {
      addCompletion(items,
                    {"io", COMPLETION_MODULE, "Standard input/output module",
                     "io", INSERT_TEXT_PLAIN},
                    source);
    }

    return items;
  }

  if (path.empty() || (path.size() == 1 && path[0] != "std" &&
                       expression.find('.') == std::string::npos)) {
    if (startsWith("std", prefix)) {
      addCompletion(items,
                    {"std", COMPLETION_MODULE, "Dusk standard library", "std",
                     INSERT_TEXT_PLAIN},
                    source);
    }

    if (startsWith("fn", prefix)) {
      addCompletion(items,
                    {"fn", COMPLETION_KEYWORD, "Function declaration",
                     "fn ${1:name}(): ${2:void} {\n\t$0\n}",
                     INSERT_TEXT_SNIPPET},
                    source);
    }

    if (startsWith("main", prefix)) {
      addCompletion(items,
                    {"main", COMPLETION_SNIPPET, "Main function entry point",
                     "fn main(): void {\n\t$0\n}", INSERT_TEXT_SNIPPET},
                    source);
    }

    if (startsWith("use", prefix)) {
      addCompletion(items,
                    {"use", COMPLETION_KEYWORD, "Module import",
                     "use ${1:std.io};", INSERT_TEXT_SNIPPET},
                    source);
    }

    if (startsWith("println", prefix)) {
      addCompletion(items,
                    {"println", COMPLETION_METHOD,
                     "Print a line to stdout (io.println)",
                     "io.println(${1:\"\"});", INSERT_TEXT_SNIPPET, true},
                    source);
    }

    if (hasImport(source, "std.io") && startsWith("io", prefix)) {
      addCompletion(items,
                    {"io", COMPLETION_MODULE, "Standard input/output module",
                     "io", INSERT_TEXT_PLAIN},
                    source);
    }

    return items;
  }

  if (path.size() == 1 && path[0] == "std") {
    if (startsWith("io", prefix)) {
      addCompletion(items,
                    {"io", COMPLETION_MODULE, "Standard input/output module",
                     "io", INSERT_TEXT_PLAIN},
                    source);
    }

    return items;
  }

  if (path.size() == 2 && path[0] == "std" && path[1] == "io") {
    if (startsWith("println", prefix)) {
      addCompletion(items,
                    {"println", COMPLETION_METHOD, "Print a line to stdout",
                     "println(${1:\"\"});", INSERT_TEXT_SNIPPET},
                    source);
    }

    return items;
  }

  if (path.size() == 1 && path[0] == "io" && hasImport(source, "std.io")) {
    if (startsWith("println", prefix)) {
      addCompletion(items,
                    {"println", COMPLETION_METHOD, "Print a line to stdout",
                     "println(${1:\"\"});", INSERT_TEXT_SNIPPET},
                    source);
    }

    return items;
  }

  return items;
}

} // namespace dusk::lsp
