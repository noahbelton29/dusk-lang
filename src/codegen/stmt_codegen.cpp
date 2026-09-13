#include "dusk/codegen.h"
#include "dusk/ast.h"

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>

void CodeGenerator::genStmt(const Stmt* stmt) {
  if (auto fn = dynamic_cast<const FunctionDecl*>(stmt)) {
    genFunctionDecl(fn);
  } else if (auto exprStmt = dynamic_cast<const ExprStmt*>(stmt)) {
    genExprStmt(exprStmt);
  }
}

void CodeGenerator::genFunctionDecl(const FunctionDecl* fn) {
  llvm::FunctionType* fnType = llvm::FunctionType::get(
      llvm::Type::getVoidTy(m_context),
      false // varags
      );

  llvm::Function* llvmFn = llvm::Function::Create(
      fnType,
      llvm::Function::ExternalLinkage,
      fn->name,
      m_module.get()
      );

  llvm::BasicBlock* entry = llvm::BasicBlock::Create(m_context, "entry", llvmFn);
  m_builder.SetInsertPoint(entry);

  for (const auto& stmt : fn->body) {
    genStmt(stmt.get());
  }

  m_builder.CreateRetVoid();
}

void CodeGenerator::genExprStmt(const ExprStmt* stmt) {
  genExpr(stmt->expr.get());
}
