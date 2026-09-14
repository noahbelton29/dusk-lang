#include "dusk/codegen.h"

#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>

CodeGenerator::CodeGenerator(DiagnosticEngine &diagnostics)
    : m_builder(m_context), m_diagnostics(diagnostics) {
  m_module = std::make_unique<llvm::Module>("dusk_module", m_context);
  declarePrintf();
}

bool CodeGenerator::hasErrors() const { return m_diagnostics.hasErrors(); }

void CodeGenerator::declarePrintf() {
  // printf's signature from libc
  llvm::FunctionType *printfType = llvm::FunctionType::get(
      llvm::Type::getInt32Ty(m_context), {llvm::PointerType::get(m_context, 0)},
      true // var arg
  );

  m_printfFunc = llvm::Function::Create(
      printfType, llvm::Function::ExternalLinkage, "printf", m_module.get());
}

void CodeGenerator::generate(
    const std::vector<std::unique_ptr<Stmt>> &program) {
  for (const auto &stmt : program) {
    genStmt(stmt.get());
  }
}

void CodeGenerator::dump() { m_module->print(llvm::outs(), nullptr); }
