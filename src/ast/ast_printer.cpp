#include "dusk/ast_printer.h"

#include <format>
#include <iostream>

void printExpr(const Expr *expr, const std::string &prefix) {
  if (auto lit = dynamic_cast<const StringLiteral *>(expr)) {
    std::cerr << std::format("{}StringLiteral \"{}\"\n", prefix, lit->value);
  } else if (auto call = dynamic_cast<const CallExpr *>(expr)) {
    std::cerr << std::format("{}Call {}\n", prefix, call->callee);
    for (const auto &arg : call->args) {
      printExpr(arg.get(), prefix + "  ");
    }
  } else {
    std::cerr << std::format("{}<unknown expr>\n", prefix);
  }
}

void printStmt(const Stmt *stmt, const std::string &prefix) {
  if (auto use = dynamic_cast<const UseStmt *>(stmt)) {
    std::cerr << std::format("{}Use {}\n", prefix, use->modulePath);
  } else if (auto fn = dynamic_cast<const FunctionDecl *>(stmt)) {
    std::cerr << std::format("{}Fn {} -> {}\n", prefix, fn->name,
                             fn->returnType);
    for (const auto &s : fn->body) {
      printStmt(s.get(), prefix + "  ");
    }
  } else if (auto exprStmt = dynamic_cast<const ExprStmt *>(stmt)) {
    printExpr(exprStmt->expr.get(), prefix);
  } else {
    std::cerr << std::format("{}<unknown stmt>\n", prefix);
  }
}

void printAst(const std::vector<std::unique_ptr<Stmt>> &program) {
  std::cerr << "AST:\n";
  for (const auto &stmt : program) {
    printStmt(stmt.get());
  }
}
