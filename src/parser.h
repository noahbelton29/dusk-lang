#pragma once

#include "token.h"
#include "ast.h"
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

class Parser {
  public:
    Parser(std::vector<Token> tokens);
    std::vector<std::unique_ptr<Stmt>> parse();

  private:
    std::vector<Token> m_tokens;
    size_t m_pos;

    Token peek();
    Token advance();

    bool check(TokenType type);
    Token expect(TokenType type, std::string errorMessage);
    bool isAtEnd();

    std::unique_ptr<Stmt> parseStmt();
    std::unique_ptr<UseStmt> parseUseStmt();
    std::unique_ptr<FunctionDecl> parseFunctionDecl();
    std::unique_ptr<ExprStmt> parseExprStmt();
    std::unique_ptr<Expr> parseExpr();
};
