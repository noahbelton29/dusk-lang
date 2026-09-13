#pragma once

#include <string>

enum class TokenType {
  LPAREN,
  RPAREN,
  END_OF_FILE
};

struct Token {
  TokenType type;
  std::string lexeme;
  int line;
};

inline std::string tokenTypeToString(TokenType type) {
  switch (type) {
    case TokenType::LPAREN: 
      return "LPAREN";
    case TokenType::RPAREN: 
      return "RPAREN";
    case TokenType::END_OF_FILE:
      return "END_OF_FILE";
    default:
      return "UNKNOWN";
  }
}
