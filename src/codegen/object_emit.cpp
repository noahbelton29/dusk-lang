#include "dusk/codegen.h"

#include "dusk/diagnostics.h"
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>
#include <string>

void CodeGenerator::emitObjectFile(const std::string &outputPath) {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();

  llvm::Triple targetTriple(llvm::sys::getDefaultTargetTriple());
  m_module->setTargetTriple(targetTriple);

  std::string error;
  const llvm::Target *target =
      llvm::TargetRegistry::lookupTarget(targetTriple, error);
  if (!target) {
    printFatalError("failed to look up compilation target: " + error);
    return;
  }

  llvm::TargetOptions opts;
  auto relocModel = llvm::Reloc::PIC_;
  llvm::TargetMachine *targetMachine = target->createTargetMachine(
      targetTriple, "generic", "", opts, relocModel);

  m_module->setDataLayout(targetMachine->createDataLayout());

  std::error_code ec;
  llvm::raw_fd_ostream dest(outputPath, ec, llvm::sys::fs::OF_None);
  if (ec) {
    printFatalError("could not open output file: " + ec.message());
    return;
  }

  llvm::legacy::PassManager pass;
  if (targetMachine->addPassesToEmitFile(pass, dest, nullptr,
                                         llvm::CodeGenFileType::ObjectFile)) {
    printFatalError(
        "target machine cannot emit an object file for this platform");
    return;
  }

  pass.run(*m_module);
  dest.flush();
}
