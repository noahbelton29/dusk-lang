#pragma once

#include <memory>
#include <string>
#include <vector>

// Expressions
struct Expr {
  virtual ~Expr() = default;

  int line = 0;
  int column = 0;
};

struct StringLiteral : Expr {
  std::string value;
};

struct CallExpr : Expr {
  std::string callee;
  std::vector<std::unique_ptr<Expr>> args;
};

// Statements
struct Stmt {
  virtual ~Stmt() = default;

  int line = 0;
  int column = 0;
};

struct ExprStmt : Stmt {
  std::unique_ptr<Expr> expr;
};

struct UseStmt : Stmt {
  std::string modulePath;
};

struct FunctionDecl : Stmt {
  std::string name;
  std::string returnType;
  std::vector<std::unique_ptr<Stmt>> body;
};
