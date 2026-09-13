#pragma once

#include "token.h"
#include "diagnostics.h"

#include <string>
#include <vector>

class Lexer {
  public:
    Lexer(const std::string& source, DiagnosticEngine& diagnostics);

    bool isAtEnd();
    char peek();
    char advance();

    Token scanString(int startLine, int startColumn);
    Token scanIdentifier(char first, int startLine, int startColumn);

    std::vector<Token> tokenise();

  private:
    std::string m_source;
    int m_pos;
    int m_line;
    int m_column;
    DiagnosticEngine& m_diagnostics;
};
