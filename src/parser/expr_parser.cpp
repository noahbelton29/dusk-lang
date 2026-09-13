#include "dusk/parser.h"
#include "dusk/ast.h"
#include "dusk/token.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

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
