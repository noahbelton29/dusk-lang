#include "protocol.h"

#include <iostream>
#include <string_view>

#include <llvm/Support/raw_ostream.h>

namespace dusk::lsp::protocol {

std::string jsonString(const llvm::json::Value &value) {
  std::string result;

  llvm::raw_string_ostream stream(result);
  stream << value;
  stream.flush();

  return result;
}

std::string readMessage() {
  std::string line;
  std::size_t contentLength = 0;
  bool sawHeader = false;

  while (std::getline(std::cin, line)) {
    if (!line.empty() && line.back() == '\r')
      line.pop_back();

    if (line.empty())
      break;

    constexpr std::string_view prefix = "Content-Length:";

    if (line.starts_with(prefix)) {
      std::string length = line.substr(prefix.size());
      const auto first = length.find_first_not_of(" \t");

      if (first != std::string::npos)
        length.erase(0, first);

      try {
        contentLength = static_cast<std::size_t>(std::stoul(length));
        sawHeader = true;
      } catch (const std::exception &e) {
        std::cerr << "[dusk-lsp] malformed Content-Length '" << length
                  << "': " << e.what() << "\n";
        return {};
      }
    }
  }

  if (std::cin.eof())
    return {};

  if (!sawHeader || contentLength == 0) {
    std::cerr << "[dusk-lsp] message with no Content-Length header, skipping\n";
    return {};
  }

  std::string body(contentLength, '\0');

  std::cin.read(body.data(), static_cast<std::streamsize>(contentLength));

  if (!std::cin) {
    std::cerr << "[dusk-lsp] short read: expected " << contentLength
              << " bytes\n";
    return {};
  }

  return body;
}

void send(const std::string &message) {
  std::cout << "Content-Length: " << message.size() << "\r\n\r\n"
            << message << std::flush;
}

} // namespace dusk::lsp::protocol
