#include "dusk/codegen.h"

#include <iostream>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Triple.h>
#include <llvm/MC/TargetRegistry.h>
#include <string>

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
