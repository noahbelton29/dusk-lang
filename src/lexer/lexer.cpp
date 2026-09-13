#include "dusk/lexer.h"
#include "dusk/token.h"

#include <cctype>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

static const std::unordered_map<std::string, TokenType> keywords = {
  {"use", TokenType::USE},
  {"fn", TokenType::FN},
};

Lexer::Lexer(const std::string& source) : m_source(std::move(source)), m_pos(0), m_line(1) {}

bool Lexer::isAtEnd() {
  return static_cast<size_t>(m_pos) >= m_source.length();
}

char Lexer::peek() {
  if (isAtEnd()) {
    return '\0';
  }
  return m_source[m_pos];
}

char Lexer::advance() {
  char c = m_source[m_pos];
  m_pos++;
  return c;
}

Token Lexer::scanString() {
  std::string value;
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n')
      m_line++;
    value += advance();
  }

  if (isAtEnd()) {
    // TODO: replace with proper error reporting once i have a diagnostics system
    std::cerr << "Unterminated string at line " << m_line << '\n';
    return Token{TokenType::END_OF_FILE, "", m_line}; // temporary placeholder for error case 
  }

  advance(); // consume the closing "
  return Token{TokenType::STRING, value, m_line};
}

Token Lexer::scanIdentifier(char first) {
  std::string ident;
  ident += first;
  while (isalnum(static_cast<unsigned char>(peek())) || peek() == '_')
    ident += advance();

  auto it = keywords.find(ident);
  TokenType type = (it != keywords.end()) ? it->second : TokenType::IDENT;
  return Token{type, ident, m_line};
}

std::vector<Token> Lexer::tokenise() {
  std::vector<Token> tokens;

  while (!isAtEnd()) {
    char c = advance();

    // detect identifiers
    if (isalpha(static_cast<unsigned char>(c)) || c == '_') {
      tokens.push_back(scanIdentifier(c));
      continue;
    }

    // symbols & punctuation
    switch (c) {
      case '(':
        tokens.push_back(Token{TokenType::LPAREN, "(", m_line});
        break;
      case '{':
        tokens.push_back(Token{TokenType::LBRACE, "{", m_line});
        break;
      case '}':
        tokens.push_back(Token{TokenType::RBRACE, "}", m_line});
        break;
      case ')':
        tokens.push_back(Token{TokenType::RPAREN, ")", m_line});
        break;
      case '.':
        tokens.push_back(Token{TokenType::DOT, ".", m_line});
        break;
      case '"':
        tokens.push_back(scanString());
        break;
      case ';':
        tokens.push_back(Token{TokenType::SEMICOLON, ";", m_line});
        break;
      case ',':
        tokens.push_back(Token{TokenType::COMMA, ",", m_line});
        break;
      case ':':
        tokens.push_back(Token{TokenType::COLON, ":", m_line});
        break;
      case '\n':
        m_line++;
        break;
      case ' ':
      case '\t':
      case '\r':
        // whitespace has no semantic meaning in this language
        break;
      default:
        // TODO: replace with proper error reporting once i have a diagnostics system
        std::cerr << "Unexpected character '" << c << "' at line " << m_line << "\n";
        break;
    }
  }

  Token eof = Token{TokenType::END_OF_FILE, "", m_line};
  tokens.push_back(eof);
  return tokens;
}
