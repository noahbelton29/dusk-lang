#pragma once

#include <string>

#include <llvm/Support/JSON.h>

namespace dusk::lsp::protocol {

std::string jsonString(const llvm::json::Value &value);
std::string readMessage();
void send(const std::string &message);

} // namespace dusk::lsp::protocol
