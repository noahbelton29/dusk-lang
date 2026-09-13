#include "dusk/codegen.h"
#include "dusk/lexer.h"
#include "dusk/token.h"
#include "dusk/parser.h"
#include "dusk/ast_printer.h"
#include "dusk/compiler_options.h"
#include "dusk/file_utils.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

void printToken(const Token& tok) {
  std::cerr << tokenTypeToString(tok.type)
    << " \"" << tok.lexeme << "\""
    << " (line " << tok.line << ")\n";
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
