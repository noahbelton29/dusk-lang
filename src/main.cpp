#include "lexer.h"
#include "token.h"
#include "parser.h"
#include "ast_printer.h"

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
  std::cout << tokenTypeToString(tok.type)
    << " \"" << tok.lexeme << "\""
    << " (line " << tok.line << ")\n";
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: duskc <file.dsk>\n";
    return 1;
  }
  std::string source = readFile(argv[1]);
  std::cout << source << "\n";
  Lexer lexer(source);
  std::vector<Token> tokens = lexer.tokenise();

  for (const Token& tok : tokens) {
    printToken(tok);
  }

  Parser parser(tokens);
  auto program = parser.parse();
  printAst(program);

  return 0;
}
