#include "dusk/file_utils.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>

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
