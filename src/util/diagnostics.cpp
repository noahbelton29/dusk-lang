#include "dusk/diagnostics.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <io.h>
#define ISATTY _isatty
#define FILENO _fileno
#else
#include <unistd.h>
#define ISATTY isatty
#define FILENO fileno
#endif

namespace {

// ansi colour helpers
constexpr const char* RESET = "\033[0m";
constexpr const char* BOLD = "\033[1m";
constexpr const char* RED = "\033[31m";
constexpr const char* YELLOW = "\033[33m";
constexpr const char* BLUE = "\033[36m";
constexpr const char* GRAY = "\033[90m";

bool colorShouldBeEnabled() {
  if (std::getenv("NO_COLOR") != nullptr) {
    return false;
  }
  return ISATTY(FILENO(stderr)) != 0;
}

std::vector<std::string> splitLines(const std::string& source) {
  std::vector<std::string> lines;
  std::string current;
  for (char c : source) {
    if (c == '\n') {
      lines.push_back(current);
      current.clear();
    } else {
      current += c;
    }
  }
  lines.push_back(current);
  return lines;
}

const char* severityColor(Severity severity) {
  switch (severity) {
    case Severity::Error:
      return RED;
    case Severity::Warning:
      return YELLOW;
    case Severity::Note:
      return GRAY;
  }
  return RESET;
}

const char* severityLabel(Severity severity) {
  switch (severity) {
    case Severity::Error:
      return "error";
    case Severity::Warning:
      return "warning";
    case Severity::Note:
      return "note";
  }
  return "diagnostic";
}

} // namespace

DiagnosticEngine::DiagnosticEngine(std::string filename, std::string source)
    : m_filename(std::move(filename)),
      m_lines(splitLines(source)),
      m_colorEnabled(colorShouldBeEnabled()) {}

void DiagnosticEngine::error(std::string message, int line, int column, int length,
                              std::optional<std::string> help) {
  report(Severity::Error, std::move(message), line, column, length, std::move(help));
}

void DiagnosticEngine::warning(std::string message, int line, int column, int length,
                                std::optional<std::string> help) {
  report(Severity::Warning, std::move(message), line, column, length, std::move(help));
}

void DiagnosticEngine::note(std::string message, int line, int column, int length,
                             std::optional<std::string> help) {
  report(Severity::Note, std::move(message), line, column, length, std::move(help));
}

void DiagnosticEngine::report(Severity severity, std::string message, int line, int column,
                               int length, std::optional<std::string> help) {
  m_diagnostics.push_back(Diagnostic{
      severity,
      std::move(message),
      line,
      column,
      std::max(length, 1),
      std::move(help),
  });
}

bool DiagnosticEngine::hasErrors() const {
  return errorCount() > 0;
}

size_t DiagnosticEngine::errorCount() const {
  return static_cast<size_t>(std::count_if(m_diagnostics.begin(), m_diagnostics.end(),
                                            [](const Diagnostic& d) { return d.severity == Severity::Error; }));
}

size_t DiagnosticEngine::warningCount() const {
  return static_cast<size_t>(std::count_if(m_diagnostics.begin(), m_diagnostics.end(),
                                            [](const Diagnostic& d) { return d.severity == Severity::Warning; }));
}

std::string_view DiagnosticEngine::lineText(int line) const {
  size_t idx = static_cast<size_t>(line - 1);
  if (line < 1 || idx >= m_lines.size()) {
    return "";
  }
  return m_lines[idx];
}

void DiagnosticEngine::printDiagnostic(const Diagnostic& diag) const {
  const char* color = m_colorEnabled ? severityColor(diag.severity) : "";
  const char* bold = m_colorEnabled ? BOLD : "";
  const char* blue = m_colorEnabled ? BLUE : "";
  const char* reset = m_colorEnabled ? RESET : "";

  std::cerr << bold << color
            << severityLabel(diag.severity)
            << reset << ": " << diag.message << "\n";

  // Header 
  std::cerr << blue
            << "  " << m_filename
            << ":" << diag.line
            << ":" << diag.column
            << reset << "\n";

  std::string_view text = lineText(diag.line);

  // Source line
  std::cerr << "  " << text << "\n";

  // Pointer
  std::cerr << "  ";

  int col = std::max(diag.column, 1);

  for (int i = 1; i < col; ++i) {
    if (i - 1 < static_cast<int>(text.size()) && text[i - 1] == '\t') {
      std::cerr << '\t';
    } else {
      std::cerr << ' ';
    }
  }

  std::cerr << color << bold;

  for (int i = 0; i < diag.length; ++i) {
    std::cerr << '^';
  }

  std::cerr << reset;

  if (diag.help) {
    std::cerr << " " << *diag.help;
  }

  std::cerr << "\n\n";
}

void DiagnosticEngine::printAll() const {
  for (const Diagnostic& diag : m_diagnostics) {
    printDiagnostic(diag);
  }
}

void DiagnosticEngine::printSummary() const {
  size_t errors = errorCount();
  size_t warnings = warningCount();
  if (errors == 0 && warnings == 0) {
    return;
  }

  const char* bold = m_colorEnabled ? BOLD : "";
  const char* red = m_colorEnabled ? RED : "";
  const char* yellow = m_colorEnabled ? YELLOW : "";
  const char* reset = m_colorEnabled ? RESET : "";

  std::ostringstream out;
  if (errors > 0) {
    out << bold << red << errors << " error" << (errors == 1 ? "" : "s") << reset;
  }
  if (errors > 0 && warnings > 0) {
    out << ", ";
  }
  if (warnings > 0) {
    out << bold << yellow << warnings << " warning" << (warnings == 1 ? "" : "s") << reset;
  }
  out << bold << " generated" << reset;

  std::cerr << out.str() << "\n";
}

void printFatalError(const std::string& message) {
  bool color = colorShouldBeEnabled();
  const char* bold = color ? BOLD : "";
  const char* red = color ? RED : "";
  const char* reset = color ? RESET : "";
  std::cerr << bold << red << "error: " << reset << message << "\n";
}
