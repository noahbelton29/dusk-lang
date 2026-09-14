#include "dusk/ast.h"
#include "dusk/codegen.h"

llvm::Value *CodeGenerator::genExpr(const Expr *expr) {
  if (auto call = dynamic_cast<const CallExpr *>(expr)) {
    return genCallExpr(call);
  } else if (auto lit = dynamic_cast<const StringLiteral *>(expr)) {
    return genStringLiteral(lit);
  }
  return nullptr;
}

llvm::Value *CodeGenerator::genCallExpr(const CallExpr *call) {
  // hardcoded the println fn for now
  // TODO: proper module/function resolution
  if (call->callee == "io.println") {
    llvm::Value *strArg = genExpr(call->args[0].get());

    llvm::Value *fmt = m_builder.CreateGlobalString("%s\n");

    return m_builder.CreateCall(m_printfFunc, {fmt, strArg});
  }

  m_diagnostics.error("unknown function '" + call->callee + "'", call->line,
                      call->column, static_cast<int>(call->callee.size()),
                      "'" + call->callee +
                          "' isn't declared anywhere - check for a typo");
  return nullptr;
}

llvm::Value *CodeGenerator::genStringLiteral(const StringLiteral *lit) {
  return m_builder.CreateGlobalString(lit->value);
}
