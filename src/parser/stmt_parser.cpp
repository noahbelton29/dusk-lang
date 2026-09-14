#include "dusk/ast.h"
#include "dusk/parser.h"
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
  Token start = expect(TokenType::USE, "expected 'use'");

  std::string path = expect(TokenType::IDENT, "expected module name").lexeme;
  while (check(TokenType::DOT)) {
    advance();
    path = path + "." +
           expect(TokenType::IDENT, "expected identifier after '.'").lexeme;
  }

  expect(TokenType::SEMICOLON, "expected ';' after use statement");

  auto stmt = std::make_unique<UseStmt>();
  stmt->modulePath = path;
  stmt->line = start.line;
  stmt->column = start.column;
  return stmt;
}

std::unique_ptr<FunctionDecl> Parser::parseFunctionDecl() {
  Token start = expect(TokenType::FN, "expected 'fn'");
  std::string name =
      expect(TokenType::IDENT, "expected function name after 'fn'").lexeme;

  expect(TokenType::LPAREN, "expected '(' after function name");
  // TODO: add parameter support
  expect(TokenType::RPAREN, "expected ')' after parameter list");
  expect(TokenType::COLON, "expected ':' before return type");

  std::string returnType =
      expect(TokenType::IDENT, "expected return type").lexeme;

  expect(TokenType::LBRACE, "expected '{' to start function body");
  std::vector<std::unique_ptr<Stmt>> body;
  while (!check(TokenType::RBRACE) && !isAtEnd()) {
    try {
      body.push_back(parseStmt());
    } catch (const ParseError &) {
      synchronize();
    }
  }
  expect(TokenType::RBRACE, "expected '}' to close function body");

  auto stmt = std::make_unique<FunctionDecl>();
  stmt->name = name;
  stmt->body = std::move(body);
  stmt->returnType = returnType;
  stmt->line = start.line;
  stmt->column = start.column;
  return stmt;
}

std::unique_ptr<ExprStmt> Parser::parseExprStmt() {
  Token start = peek();
  std::unique_ptr<Expr> expr = parseExpr();
  expect(TokenType::SEMICOLON, "expected ';' after expression");
  auto stmt = std::make_unique<ExprStmt>();
  stmt->expr = std::move(expr);
  stmt->line = start.line;
  stmt->column = start.column;
  return stmt;
}
