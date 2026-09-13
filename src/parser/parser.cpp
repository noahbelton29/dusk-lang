#include "dusk/parser.h"
#include "dusk/ast.h"
#include "dusk/token.h"

#include <memory>
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
  while (!isAtEnd())
    stmts.push_back(parseStmt());
  return stmts;
}

std::unique_ptr<Stmt> Parser::parseStmt() {
  if (check(TokenType::USE)) {
    return parseUseStmt();
  } else if (check(TokenType::FN)) {
    return parseFunctionDecl();
  } else {
    return parseExprStmt();
  }
}

std::unique_ptr<UseStmt> Parser::parseUseStmt() {
  expect(TokenType::USE, "expected 'use'");

  std::string path = expect(TokenType::IDENT, "expected module name").lexeme;
  while (check(TokenType::DOT)) {
    advance(); // consume .
    path = path + "." + expect(TokenType::IDENT, "expected identifier").lexeme;
  }

  expect(TokenType::SEMICOLON, "expected ';' after use statement");

  auto stmt = std::make_unique<UseStmt>();
  stmt->modulePath = path;
  return stmt;
}


std::unique_ptr<FunctionDecl> Parser::parseFunctionDecl() {
  expect(TokenType::FN, "expected 'fn'");
  std::string name = expect(TokenType::IDENT, "expected function name").lexeme;

  expect(TokenType::LPAREN, "expected '('");
  // TODO: add parameter support
  expect(TokenType::RPAREN, "expected ')'"); 
  expect(TokenType::COLON, "expected ':'");

  std::string returnType = expect(TokenType::IDENT, "expected return type").lexeme;

  expect(TokenType::LBRACE, "expected '{'");
  std::vector<std::unique_ptr<Stmt>> body;
  while (!check(TokenType::RBRACE) && !isAtEnd()) {
    body.push_back(parseStmt());
  }
  expect(TokenType::RBRACE, "expected '}'");

  auto stmt = std::make_unique<FunctionDecl>();
  stmt->name = name;
  stmt->body = std::move(body);
  stmt->returnType = returnType;
  return stmt;
}

std::unique_ptr<ExprStmt> Parser::parseExprStmt() {
  std::unique_ptr<Expr> expr = parseExpr();
  expect(TokenType::SEMICOLON, "expected ';'");
  auto stmt = std::make_unique<ExprStmt>();
  stmt->expr = std::move(expr);
  return stmt;
}

std::unique_ptr<Expr> Parser::parseExpr() {
  if (check(TokenType::STRING)) {
    auto lit = std::make_unique<StringLiteral>();
    lit->value = advance().lexeme;
    return lit;
  }

  std::string callee = expect(TokenType::IDENT, "expected identifier").lexeme;
  while (check(TokenType::DOT)) {
    advance();
    callee = callee + "." + expect(TokenType::IDENT, "expected identifier").lexeme;
  }

  expect(TokenType::LPAREN, "expected '('");
  std::vector<std::unique_ptr<Expr>> args;
  if (!check(TokenType::RPAREN)) {
    args.push_back(parseExpr());
    while (check(TokenType::COMMA)) {
      advance();
      args.push_back(parseExpr());
    }
  }
  expect(TokenType::RPAREN, "expected ')'");

  auto expr = std::make_unique<CallExpr>();
  expr->callee = callee;
  expr->args = std::move(args);
  return expr;
}
