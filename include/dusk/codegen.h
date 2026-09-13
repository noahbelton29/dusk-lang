#pragma once

#include "ast.h"
#include "diagnostics.h"

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <memory>
#include <vector>

class CodeGenerator {
  public:
    CodeGenerator(DiagnosticEngine& diagnostics);

    void generate(const std::vector<std::unique_ptr<Stmt>>& program);
    void dump();
    void emitObjectFile(const std::string& outputPath);

    bool hasErrors() const;

  private:
    llvm::LLVMContext m_context;
    llvm::IRBuilder<> m_builder;
    std::unique_ptr<llvm::Module> m_module;
    DiagnosticEngine& m_diagnostics;

    llvm::Function* m_printfFunc  = nullptr;

    void declarePrintf();

    void genStmt(const Stmt* stmt);
    void genFunctionDecl(const FunctionDecl* fn);
    void genExprStmt(const ExprStmt* stmt);

    llvm::Value* genExpr(const Expr* expr);
    llvm::Value* genCallExpr(const CallExpr* call);
    llvm::Value* genStringLiteral(const StringLiteral* lit);
};
