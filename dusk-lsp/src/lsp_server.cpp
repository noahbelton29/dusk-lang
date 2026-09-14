#include "lsp_server.h"
#include "completion.h"
#include "diagnostics.h"
#include "protocol.h"

#include <iostream>
#include <utility>

using namespace dusk::lsp;

void LspServer::run() {
  while (!m_shutdown) {
    const std::string body = protocol::readMessage();

    if (body.empty()) {
      if (std::cin.eof() || !std::cin) {
        std::cerr << "[dusk-lsp] stdin closed, exiting\n";
        break;
      }

      continue;
    }

    handleMessage(body);
  }
}

void LspServer::handleMessage(const std::string &body) {
  auto parsed = llvm::json::parse(body);

  if (!parsed) {
    std::cerr << "[dusk-lsp] failed to parse JSON body\n";
    return;
  }

  auto *object = parsed->getAsObject();

  if (!object) {
    std::cerr << "[dusk-lsp] top-level JSON is not an object\n";
    return;
  }

  const auto methodValue = object->getString("method");

  if (!methodValue) {
    std::cerr << "[dusk-lsp] message missing 'method'\n";
    return;
  }

  const std::string method = methodValue->str();

  if (method == "initialize") {
    if (const auto id = object->getInteger("id")) {
      handleInitialize(std::to_string(*id));
    } else {
      std::cerr << "[dusk-lsp] initialize missing 'id'\n";
    }

    return;
  }

  if (method == "initialized")
    return;

  if (method == "shutdown") {
    if (const auto id = object->getInteger("id")) {
      handleShutdown(std::to_string(*id));
    } else {
      std::cerr << "[dusk-lsp] shutdown missing 'id'\n";
    }

    return;
  }

  if (method == "exit") {
    m_shutdown = true;
    return;
  }

  if (method == "textDocument/completion") {
    const auto id = object->getInteger("id");
    auto *params = object->getObject("params");

    if (!id || !params) {
      std::cerr << "[dusk-lsp] completion request missing id/params\n";
      return;
    }

    auto *document = params->getObject("textDocument");
    auto *position = params->getObject("position");

    if (!document || !position) {
      sendEmptyCompletion(std::to_string(*id));
      return;
    }

    const auto uri = document->getString("uri");
    const auto line = position->getInteger("line");
    const auto character = position->getInteger("character");

    if (!uri || !line.has_value() || !character.has_value()) {
      std::cerr << "[dusk-lsp] completion request missing uri/line/character\n";
      sendEmptyCompletion(std::to_string(*id));
      return;
    }

    handleCompletion(std::to_string(*id), uri->str(), static_cast<int>(*line),
                     static_cast<int>(*character));
    return;
  }

  auto *params = object->getObject("params");

  if (!params) {
    std::cerr << "[dusk-lsp] '" << method << "' missing 'params'\n";
    return;
  }

  if (method == "textDocument/didOpen") {
    auto *document = params->getObject("textDocument");

    if (!document) {
      std::cerr << "[dusk-lsp] didOpen missing textDocument\n";
      return;
    }

    const auto uri = document->getString("uri");
    const auto text = document->getString("text");

    if (!uri || !text) {
      std::cerr << "[dusk-lsp] didOpen missing uri/text\n";
      return;
    }

    handleDidOpen(uri->str(), text->str());
    return;
  }

  if (method == "textDocument/didChange") {
    auto *document = params->getObject("textDocument");

    if (!document) {
      std::cerr << "[dusk-lsp] didChange missing textDocument\n";
      return;
    }

    const auto uri = document->getString("uri");

    if (!uri) {
      std::cerr << "[dusk-lsp] didChange missing uri\n";
      return;
    }

    auto *changes = params->getArray("contentChanges");

    if (!changes || changes->empty()) {
      std::cerr << "[dusk-lsp] didChange missing contentChanges\n";
      return;
    }

    auto *change = (*changes)[0].getAsObject();

    if (!change) {
      std::cerr << "[dusk-lsp] didChange contentChanges[0] not an object\n";
      return;
    }

    const auto text = change->getString("text");

    if (!text) {
      std::cerr << "[dusk-lsp] didChange missing text (is sync mode Full?)\n";
      return;
    }

    handleDidChange(uri->str(), text->str());
    return;
  }

  if (method == "textDocument/didClose") {
    auto *document = params->getObject("textDocument");

    if (!document) {
      std::cerr << "[dusk-lsp] didClose missing textDocument\n";
      return;
    }

    const auto uri = document->getString("uri");

    if (!uri) {
      std::cerr << "[dusk-lsp] didClose missing uri\n";
      return;
    }

    handleDidClose(uri->str());
    return;
  }
}

void LspServer::handleInitialize(const std::string &id) {
  llvm::json::Object capabilities{
      {"textDocumentSync",
       llvm::json::Object{{"openClose", true}, {"change", 1}}},
      {"completionProvider",
       llvm::json::Object{{"triggerCharacters", llvm::json::Array{".", "_"}},
                          {"resolveProvider", false}}}};

  llvm::json::Object serverInfo{{"name", "dusk-lsp"}, {"version", "0.1.0"}};

  llvm::json::Object result{{"capabilities", std::move(capabilities)},
                            {"serverInfo", std::move(serverInfo)}};

  llvm::json::Object response{
      {"jsonrpc", "2.0"}, {"id", std::stoi(id)}, {"result", std::move(result)}};

  protocol::send(protocol::jsonString(llvm::json::Value(std::move(response))));
}

void LspServer::handleShutdown(const std::string &id) {
  llvm::json::Object response{
      {"jsonrpc", "2.0"}, {"id", std::stoi(id)}, {"result", nullptr}};

  protocol::send(protocol::jsonString(llvm::json::Value(std::move(response))));

  m_shutdown = true;
}

void LspServer::sendEmptyCompletion(const std::string &id) {
  llvm::json::Object response{
      {"jsonrpc", "2.0"},
      {"id", std::stoi(id)},
      {"result", llvm::json::Object{{"isIncomplete", false},
                                    {"items", llvm::json::Array{}}}}};

  protocol::send(protocol::jsonString(llvm::json::Value(std::move(response))));
}

void LspServer::handleCompletion(const std::string &id, const std::string &uri,
                                 int line, int character) {
  const auto document = m_documents.find(uri);

  if (document == m_documents.end()) {
    std::cerr << "[dusk-lsp] completion for unknown document: " << uri << "\n";
    sendEmptyCompletion(id);
    return;
  }

  const std::string currentLine = getCurrentLine(document->second, line);

  llvm::json::Array items =
      makeCompletions(document->second, currentLine, character);

  llvm::json::Object result{{"isIncomplete", false},
                            {"items", std::move(items)}};

  llvm::json::Object response{
      {"jsonrpc", "2.0"}, {"id", std::stoi(id)}, {"result", std::move(result)}};

  protocol::send(protocol::jsonString(llvm::json::Value(std::move(response))));
}

void LspServer::handleDidOpen(const std::string &uri, const std::string &text) {
  m_documents[uri] = text;
  publishDiagnostics(uri, text);
}

void LspServer::handleDidChange(const std::string &uri,
                                const std::string &text) {
  m_documents[uri] = text;
  publishDiagnostics(uri, text);
}

void LspServer::handleDidClose(const std::string &uri) {
  m_documents.erase(uri);

  llvm::json::Object notification{
      {"jsonrpc", "2.0"},
      {"method", "textDocument/publishDiagnostics"},
      {"params",
       llvm::json::Object{{"uri", uri}, {"diagnostics", llvm::json::Array{}}}}};

  protocol::send(
      protocol::jsonString(llvm::json::Value(std::move(notification))));
}

void LspServer::publishDiagnostics(const std::string &uri,
                                   const std::string &source) {
  const auto diagnostics = diagnose(source);

  llvm::json::Object notification{
      {"jsonrpc", "2.0"},
      {"method", "textDocument/publishDiagnostics"},
      {"params",
       llvm::json::Object{{"uri", uri},
                          {"diagnostics", makeDiagnostics(diagnostics)}}}};

  protocol::send(
      protocol::jsonString(llvm::json::Value(std::move(notification))));
}
