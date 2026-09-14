#pragma once

#include <string>

struct CompilerOptions {
  std::string inputPath;
  std::string outputPath = "out";
  bool printTokens = false;
  bool printAST = false;
  bool printIR = false;
};

void printUsage();
CompilerOptions parseArgs(int argc, char *argv[]);
