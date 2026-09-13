#include "dusk/parser.h"
#include "dusk/ast.h"
#include "dusk/token.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

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
    advance();
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
