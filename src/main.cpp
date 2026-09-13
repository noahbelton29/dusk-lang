#include "codegen.h"
#include "lexer.h"
#include "token.h"
#include "parser.h"
#include "ast_printer.h"

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

// TODO: add compiler flags to optionally print tokens, AST, and IR, and having a custom file output name
int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: duskc <file.dsk>\n";
    return 1;
  }
  std::string source = readFile(argv[1]);
  std::cerr << source << "\n";

  Lexer lexer(source);
  std::vector<Token> tokens = lexer.tokenise();

  for (const Token& tok : tokens) {
    printToken(tok);
  }

  Parser parser(tokens);
  auto program = parser.parse();
  printAst(program);

  CodeGenerator codegen;
  codegen.generate(program);
  codegen.dump();

  // for now the object name will be predetermined
  // TODO: make the object name be the same as the  filename, or whatever is the output name with an output flag
  codegen.emitObjectFile("out.o");

  int result = system("cc out.o -o out");
  if (result != 0) {
    std::cerr << "Linking failed\n";
    return 1;
  }

  std::cerr << "Compiled successfully.\n";

  return 0;
}
