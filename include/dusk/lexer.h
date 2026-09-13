#pragma once

#include "token.h"

#include <string>
#include <vector>

class Lexer {
  public:
    Lexer(const std::string& source);

    bool isAtEnd();
    char peek();
    char advance();
    
    Token scanString();
    Token scanIdentifier(char first);

    std::vector<Token> tokenise();

  private:
    std::string m_source;
    int m_pos;
    int m_line;
};
