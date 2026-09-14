#include "ast.h"

#include <string>

void printExpr(const Expr *expr, const std::string &prefix);
void printStmt(const Stmt *stmt, const std::string &prefix = "");
void printAst(const std::vector<std::unique_ptr<Stmt>> &program);
