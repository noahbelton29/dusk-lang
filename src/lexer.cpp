#include "lexer.h"
#include "token.h"

#include <string>
#include <utility>
#include <vector>

Lexer::Lexer(const std::string& source) : m_source(std::move(source)), m_pos(0), m_line(0) {}

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

std::vector<Token> Lexer::tokenise() {
  std::vector<Token> tokens;

  while (!isAtEnd()) {
    char c = advance();
    
    switch (c) {
      case '(':
        tokens.push_back(Token{TokenType::LPAREN, "(", m_line});
        break;
      case ')':
        tokens.push_back(Token{TokenType::RPAREN, ")", m_line});
        break;
      case '\n':
        m_line++;
        break;
      case ' ':
      case '\t':
      case '\r':
        break;
      default:
        break;
    }
  }
  Token eof = Token{TokenType::END_OF_FILE, "", m_line};
  tokens.push_back(eof);
  return tokens;
}
