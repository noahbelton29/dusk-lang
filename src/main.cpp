#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

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

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: duskc <file.dsk>\n";
    return 1;
  }
  std::string source = readFile(argv[1]);
  std::cout << source << "\n";
  return 0;
}
