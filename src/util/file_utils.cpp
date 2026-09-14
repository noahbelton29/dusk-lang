#include "dusk/file_utils.h"
#include "dusk/diagnostics.h"

#include <cstdlib>
#include <fstream>
#include <sstream>

std::string readFile(const std::string &path) {
  std::ifstream file(path);
  if (!file) {
    printFatalError("could not open file '" + path + "'");
    exit(1);
  }
  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}
