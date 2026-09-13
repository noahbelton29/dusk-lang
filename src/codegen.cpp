#include "codegen.h"
#include "ast.h"

#include <iostream>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Triple.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/IR/LegacyPassManager.h>
#include <memory>
#include <string>

CodeGenerator::CodeGenerator() : m_builder(m_context) {
  m_module = std::make_unique<llvm::Module>("dusk_module", m_context);
  declarePrintf();
} 

void CodeGenerator::declarePrintf() {
  // printf's signature from libc
  llvm::FunctionType* printfType = llvm::FunctionType::get(
      llvm::Type::getInt32Ty(m_context),
      {llvm::PointerType::get(m_context, 0)},
      true // var arg
      );

  m_printfFunc = llvm::Function::Create(
      printfType,
      llvm::Function::ExternalLinkage,
      "printf",
      m_module.get()
      );
}

void CodeGenerator::generate(const std::vector<std::unique_ptr<Stmt>>& program) {
  for (const auto& stmt : program) {
    genStmt(stmt.get());
  }
}

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

llvm::Value* CodeGenerator::genExpr(const Expr* expr) {
  if (auto call = dynamic_cast<const CallExpr*>(expr)) {
    return genCallExpr(call);
  } else if (auto lit = dynamic_cast<const StringLiteral*>(expr)) {
    return genStringLiteral(lit);
  }
  return nullptr;
}

llvm::Value* CodeGenerator::genCallExpr(const CallExpr* call) {
  // hardcoded the println fn for now
  // TODO: proper module/function resolution
  if (call->callee == "io.println") {
    llvm::Value* strArg = genExpr(call->args[0].get());

    llvm::Value* fmt = m_builder.CreateGlobalString("%s\n");

    return m_builder.CreateCall(m_printfFunc, {fmt, strArg});
  }

  std::cerr << "Unknown function: " << call->callee << "\n";
  return nullptr;
}

llvm::Value* CodeGenerator::genStringLiteral(const StringLiteral* lit) {
  return m_builder.CreateGlobalString(lit->value);
}

void CodeGenerator::dump() {
  m_module->print(llvm::outs(), nullptr);
}

void CodeGenerator::emitObjectFile(const std::string& outputPath) {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();

  llvm::Triple targetTriple(llvm::sys::getDefaultTargetTriple());
  m_module->setTargetTriple(targetTriple);

  std::string error;
  const llvm::Target* target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
  if (!target) {
    std::cerr << "Failed to lookup target: " << error << "\n";
    return;
  }

  llvm::TargetOptions opts;
  auto relocModel = llvm::Reloc::PIC_;
  llvm::TargetMachine* targetMachine = target->createTargetMachine(
      targetTriple, "generic", "", opts, relocModel);

  m_module->setDataLayout(targetMachine->createDataLayout());

  std::error_code ec;
  llvm::raw_fd_ostream dest(outputPath, ec, llvm::sys::fs::OF_None);
  if (ec) {
    std::cerr << "Could not open output file: " << ec.message() << "\n";
    return;
  }

  llvm::legacy::PassManager pass;
  if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile)) {
    std::cerr << "TargetMachine can't emit object file\n";
    return;
  }

  pass.run(*m_module);
  dest.flush();
}
