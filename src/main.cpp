#include "dusk/ast_printer.h"
#include "dusk/codegen.h"
#include "dusk/compiler_options.h"
#include "dusk/diagnostics.h"
#include "dusk/file_utils.h"
#include "dusk/lexer.h"
#include "dusk/parser.h"
#include "dusk/token.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

void printToken(const Token &tok) {
  std::cerr << tokenTypeToString(tok.type) << " \"" << tok.lexeme << "\""
            << " (line " << tok.line << ", col " << tok.column << ")\n";
}

int main(int argc, char *argv[]) {
  CompilerOptions opts = parseArgs(argc, argv);

  std::string source = readFile(opts.inputPath);
  DiagnosticEngine diagnostics(opts.inputPath, source);

  Lexer lexer(source, diagnostics);
  std::vector<Token> tokens = lexer.tokenise();

  if (opts.printTokens) {
    for (const Token &tok : tokens) {
      printToken(tok);
    }
  }

  if (diagnostics.hasErrors()) {
    diagnostics.printAll();
    diagnostics.printSummary();
    return 1;
  }

  Parser parser(tokens, diagnostics);
  auto program = parser.parse();

  if (opts.printAST) {
    printAst(program);
  }

  if (diagnostics.hasErrors()) {
    diagnostics.printAll();
    diagnostics.printSummary();
    return 1;
  }

  CodeGenerator codegen(diagnostics);
  codegen.generate(program);

  if (opts.printIR) {
    codegen.dump();
  }

  if (diagnostics.hasErrors()) {
    diagnostics.printAll();
    diagnostics.printSummary();
    return 1;
  }

  std::string objPath = opts.outputPath + ".o";
  codegen.emitObjectFile(objPath);

  std::string linkCmd = "cc " + objPath + " -o " + opts.outputPath;
  int result = system(linkCmd.c_str());
  if (result != 0) {
    printFatalError("linking failed");
    return 1;
  }

  return 0;
}
