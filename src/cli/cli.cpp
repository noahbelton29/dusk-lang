#include "dusk/compiler_options.h"

#include <cstdlib>
#include <iostream>
#include <string>

void printUsage() {
  std::cerr << "Usage: duskc <file.dsk> [options]\n"
             << "Options:\n"
             << "  -o <name>       Set output executable name (default: out)\n"
             << "  --tokens        Print lexer tokens\n"
             << "  --ast           Print parsed AST\n"
             << "  --ir            Print generated LLVM IR\n"
             << "  --all           Print tokens, AST, and IR\n";
}

CompilerOptions parseArgs(int argc, char* argv[]) {
  CompilerOptions opts;

  if (argc < 2) {
    printUsage();
    exit(1);
  }

  opts.inputPath = argv[1];

  for (int i = 2; i < argc; i++) {
    std::string arg = argv[i];

    if (arg == "-o") {
      if (i + 1 >= argc) {
        std::cerr << "Error: -o requires a filename\n";
        exit(1);
      }
      opts.outputPath = argv[++i];
    } else if (arg == "--tokens") {
      opts.printTokens = true;
    } else if (arg == "--ast") {
      opts.printAST = true;
    } else if (arg == "--ir") {
      opts.printIR = true;
    } else if (arg == "--all") {
      opts.printTokens = true;
      opts.printAST = true;
      opts.printIR = true;
    } else {
      std::cerr << "Unknown option: " << arg << "\n";
      printUsage();
      exit(1);
    }
  }

  return opts;
}
