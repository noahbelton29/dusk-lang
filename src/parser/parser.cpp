#include "dusk/parser.h"
#include "dusk/token.h"

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

Parser::Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)), m_pos(0) {}

bool Parser::isAtEnd() {
  return peek().type == TokenType::END_OF_FILE;
}

Token Parser::peek() {
  return m_tokens[m_pos];
}

Token Parser::advance() {
  Token t = m_tokens[m_pos];
  m_pos++;
  return t;
}

bool Parser::check(TokenType type) {
  if (isAtEnd()) {
    return false;
  }
  return peek().type == type;
}

Token Parser::expect(TokenType type, std::string errorMessage) {
  if (check(type)) {
    return advance();
  } else {
    throw std::runtime_error(errorMessage + " at line " + std::to_string(peek().line));
  }
}

std::vector<std::unique_ptr<Stmt>> Parser::parse() {
  std::vector<std::unique_ptr<Stmt>> stmts;
  while (!isAtEnd()) {
    stmts.push_back(parseStmt());
  }
  return stmts;
}
