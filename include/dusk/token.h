#pragma once

#include <string>

enum class TokenType {
  // Keywords
  USE,
  FN,

  // Literals / Identifiers
  STRING,
  IDENT,

  // Punctuation / Symbols
  DOT,
  COMMA,
  LPAREN,
  RPAREN,
  LBRACE,
  RBRACE,
  SEMICOLON,
  COLON,

  // Special
  END_OF_FILE
};

struct Token {
  TokenType type;
  std::string lexeme;
  int line;
};

inline std::string tokenTypeToString(TokenType type) {
  switch (type) {
    case TokenType::USE:
      return "USE";
    case TokenType::FN:
      return "FN";
    case TokenType::STRING:
      return "STRING";
    case TokenType::IDENT:
      return "IDENT";
    case TokenType::DOT:
      return "DOT";
    case TokenType::LPAREN: 
      return "LPAREN";
    case TokenType::RPAREN: 
      return "RPAREN";
    case TokenType::LBRACE: 
      return "LBRACE";
    case TokenType::RBRACE: 
      return "RBRACE";
    case TokenType::SEMICOLON:
      return "SEMICOLON";
    case TokenType::COLON:
      return "COLON";
    case TokenType::COMMA:
      return "COMMA";
    case TokenType::END_OF_FILE:
      return "END_OF_FILE";
    default:
      return "UNKNOWN";
  }
}
