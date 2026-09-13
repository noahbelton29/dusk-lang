#include "dusk/codegen.h"
#include "dusk/lexer.h"
#include "dusk/token.h"
#include "dusk/parser.h"
#include "dusk/ast_printer.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

std::string readFile(const std::string& path) {
  std::ifstream file(path);
  if (!file) {
    std::cerr << "Could not open file: " << path << "\n";
    exit(1);
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

void printToken(const Token& tok) {
  std::cerr << tokenTypeToString(tok.type)
    << " \"" << tok.lexeme << "\""
    << " (line " << tok.line << ")\n";
}

struct CompilerOptions {
  std::string inputPath;
  std::string outputPath = "out";
  bool printTokens = false;
  bool printAST = false;
  bool printIR = false;
};

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

int main(int argc, char *argv[]) {
  CompilerOptions opts = parseArgs(argc, argv);

  std::string source = readFile(opts.inputPath);

  Lexer lexer(source);
  std::vector<Token> tokens = lexer.tokenise();

  if (opts.printTokens) {
    for (const Token& tok : tokens) {
      printToken(tok);
    }
  }

  Parser parser(tokens);
  auto program = parser.parse();

  if (opts.printAST) {
    printAst(program);
  }

  CodeGenerator codegen;
  codegen.generate(program);

  if (opts.printIR) {
    codegen.dump();
  }

  std::string objPath = opts.outputPath + ".o";
  codegen.emitObjectFile(objPath);

  std::string linkCmd = "cc " + objPath + " -o " + opts.outputPath;
  int result = system(linkCmd.c_str());
  if (result != 0) {
    std::cerr << "Linking failed\n";
    return 1;
  }

  return 0;
}
