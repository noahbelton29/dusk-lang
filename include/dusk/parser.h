#pragma once

#include "ast.h"
#include "diagnostics.h"
#include "token.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

// thrown internally when a statement can't be parsed
struct ParseError {};

class Parser {
public:
  Parser(std::vector<Token> tokens, DiagnosticEngine &diagnostics);
  std::vector<std::unique_ptr<Stmt>> parse();

private:
  std::vector<Token> m_tokens;
  size_t m_pos;
  DiagnosticEngine &m_diagnostics;

  Token peek();
  Token previous();
  Token advance();

  bool check(TokenType type);
  Token expect(TokenType type, std::string errorMessage);
  [[noreturn]] void errorAt(const Token &token, std::string message);
  [[noreturn]] void errorAfter(const Token &token, std::string message);
  bool isAtEnd();
  void synchronize();

  std::unique_ptr<Stmt> parseStmt();
  std::unique_ptr<UseStmt> parseUseStmt();
  std::unique_ptr<FunctionDecl> parseFunctionDecl();
  std::unique_ptr<ExprStmt> parseExprStmt();
  std::unique_ptr<Expr> parseExpr();
};
