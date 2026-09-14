#pragma once

#include <string>
#include <unordered_map>

class LspServer {
public:
  void run();

private:
  void handleMessage(const std::string &body);

  void handleInitialize(const std::string &id);
  void handleShutdown(const std::string &id);

  void handleCompletion(const std::string &id, const std::string &uri, int line,
                        int character);

  void sendEmptyCompletion(const std::string &id);

  void handleDidOpen(const std::string &uri, const std::string &text);
  void handleDidChange(const std::string &uri, const std::string &text);
  void handleDidClose(const std::string &uri);

  void publishDiagnostics(const std::string &uri, const std::string &source);

  bool m_shutdown = false;
  std::unordered_map<std::string, std::string> m_documents;
};
