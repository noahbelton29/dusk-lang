#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <vector>

enum class Severity {
  Error,
  Warning,
  Note,
};

struct Diagnostic {
  Severity severity;
  std::string message;
  int line;
  int column;
  int length;
  std::optional<std::string> help;
};

class DiagnosticEngine {
  public:
    DiagnosticEngine(std::string filename, std::string source);

    void error(std::string message, int line, int column, int length = 1,
               std::optional<std::string> help = std::nullopt);
    void warning(std::string message, int line, int column, int length = 1,
                 std::optional<std::string> help = std::nullopt);
    void note(std::string message, int line, int column, int length = 1,
              std::optional<std::string> help = std::nullopt);

    bool hasErrors() const;
    size_t errorCount() const;
    size_t warningCount() const;

    void printAll() const;
    void printSummary() const;

  private:
    std::string m_filename;
    std::vector<std::string> m_lines;
    std::vector<Diagnostic> m_diagnostics;
    bool m_colorEnabled;

    void report(Severity severity, std::string message, int line, int column,
                int length, std::optional<std::string> help);
    void printDiagnostic(const Diagnostic& diag) const;
    std::string_view lineText(int line) const;
};

void printFatalError(const std::string& message);
