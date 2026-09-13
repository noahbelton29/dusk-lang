#include "dusk/parser.h"
#include "dusk/ast.h"
#include "dusk/token.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

std::unique_ptr<Expr> Parser::parseExpr() {
  Token start = peek();

  if (check(TokenType::STRING)) {
    auto lit = std::make_unique<StringLiteral>();
    lit->value = advance().lexeme;
    lit->line = start.line;
    lit->column = start.column;
    return lit;
  }

  std::string callee = expect(TokenType::IDENT, "expected an expression").lexeme;
  while (check(TokenType::DOT)) {
    advance();
    callee = callee + "." + expect(TokenType::IDENT, "expected identifier after '.'").lexeme;
  }

  expect(TokenType::LPAREN, "expected '(' after function name");
  std::vector<std::unique_ptr<Expr>> args;
  if (!check(TokenType::RPAREN)) {
    args.push_back(parseExpr());
    while (check(TokenType::COMMA)) {
      advance();
      args.push_back(parseExpr());
    }
  }
  expect(TokenType::RPAREN, "expected ')' after arguments");

  auto expr = std::make_unique<CallExpr>();
  expr->callee = callee;
  expr->args = std::move(args);
  expr->line = start.line;
  expr->column = start.column;
  return expr;
}
