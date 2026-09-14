#include "dusk/parser.h"
#include "dusk/token.h"

#include <string>
#include <utility>
#include <vector>

Parser::Parser(std::vector<Token> tokens, DiagnosticEngine &diagnostics)
    : m_tokens(std::move(tokens)), m_pos(0), m_diagnostics(diagnostics) {}

bool Parser::isAtEnd() { return peek().type == TokenType::END_OF_FILE; }

Token Parser::peek() { return m_tokens[m_pos]; }

Token Parser::previous() { return m_tokens[m_pos > 0 ? m_pos - 1 : 0]; }

Token Parser::advance() {
  Token t = m_tokens[m_pos];
  if (!isAtEnd()) {
    m_pos++;
  }
  return t;
}

bool Parser::check(TokenType type) {
  if (isAtEnd()) {
    return false;
  }
  return peek().type == type;
}

void Parser::errorAt(const Token &token, std::string message) {
  int length = token.type == TokenType::END_OF_FILE
                   ? 1
                   : static_cast<int>(token.lexeme.size());
  std::string found = token.type == TokenType::END_OF_FILE
                          ? "end of file"
                          : "'" + token.lexeme + "'";
  m_diagnostics.error(message, token.line, token.column,
                      length > 0 ? length : 1, "found " + found);
  throw ParseError{};
}

void Parser::errorAfter(const Token &token, std::string message) {
  int column = token.column + static_cast<int>(token.lexeme.size());
  Token next = peek();
  std::string found = next.type == TokenType::END_OF_FILE
                          ? "end of file"
                          : "'" + next.lexeme + "'";
  m_diagnostics.error(message, token.line, column, 1, "found " + found);
  throw ParseError{};
}

Token Parser::expect(TokenType type, std::string errorMessage) {
  if (check(type)) {
    return advance();
  }
  if (m_pos > 0 && errorMessage.find(" after ") != std::string::npos) {
    errorAfter(previous(), errorMessage);
  }
  errorAt(peek(), errorMessage);
}

void Parser::synchronize() {
  while (!isAtEnd()) {
    if (previous().type == TokenType::SEMICOLON) {
      return;
    }
    switch (peek().type) {
    case TokenType::USE:
    case TokenType::FN:
    case TokenType::RBRACE:
      return;
    default:
      advance();
    }
  }
}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
  std::vector<std::unique_ptr<Stmt>> stmts;
  while (!isAtEnd()) {
    try {
      stmts.push_back(parseStmt());
    } catch (const ParseError &) {
      synchronize();
    }
  }
  return stmts;
}
