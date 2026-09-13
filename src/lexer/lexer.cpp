#include "dusk/lexer.h"
#include "dusk/token.h"

#include <cctype>
#include <string>
#include <unordered_map>
#include <vector>

static const std::unordered_map<std::string, TokenType> keywords = {
  {"use", TokenType::USE},
  {"fn", TokenType::FN},
};

Lexer::Lexer(const std::string& source, DiagnosticEngine& diagnostics)
    : m_source(source), m_pos(0), m_line(1), m_column(1), m_diagnostics(diagnostics) {}

bool Lexer::isAtEnd() {
  return static_cast<size_t>(m_pos) >= m_source.length();
}

char Lexer::peek() {
  if (isAtEnd()) {
    return '\0';
  }
  return m_source[m_pos];
}

char Lexer::advance() {
  char c = m_source[m_pos];
  m_pos++;
  m_column++;
  return c;
}

Token Lexer::scanString(int startLine, int startColumn) {
  std::string value;
  while (peek() != '"' && !isAtEnd()) {
    if (peek() == '\n') {
      m_line++;
      m_column = 0;  // advance() below brings this to 1
    }
    value += advance();
  }

  if (isAtEnd()) {
    m_diagnostics.error("unterminated string literal", startLine, startColumn,
                         static_cast<int>(value.size()) + 1,
                         "add a closing '\"' to terminate the string");
    return Token{TokenType::END_OF_FILE, "", startLine, startColumn};
  }

  advance(); // consume the closing "
  return Token{TokenType::STRING, value, startLine, startColumn};
}

Token Lexer::scanIdentifier(char first, int startLine, int startColumn) {
  std::string ident;
  ident += first;
  while (isalnum(static_cast<unsigned char>(peek())) || peek() == '_')
    ident += advance();

  auto it = keywords.find(ident);
  TokenType type = (it != keywords.end()) ? it->second : TokenType::IDENT;
  return Token{type, ident, startLine, startColumn};
}

std::vector<Token> Lexer::tokenise() {
  std::vector<Token> tokens;

  while (!isAtEnd()) {
    int startLine = m_line;
    int startColumn = m_column;
    char c = advance();

    // detect identifiers
    if (isalpha(static_cast<unsigned char>(c)) || c == '_') {
      tokens.push_back(scanIdentifier(c, startLine, startColumn));
      continue;
    }

    // symbols & punctuation
    switch (c) {
      case '(':
        tokens.push_back(Token{TokenType::LPAREN, "(", startLine, startColumn});
        break;
      case '{':
        tokens.push_back(Token{TokenType::LBRACE, "{", startLine, startColumn});
        break;
      case '}':
        tokens.push_back(Token{TokenType::RBRACE, "}", startLine, startColumn});
        break;
      case ')':
        tokens.push_back(Token{TokenType::RPAREN, ")", startLine, startColumn});
        break;
      case '.':
        tokens.push_back(Token{TokenType::DOT, ".", startLine, startColumn});
        break;
      case '"':
        tokens.push_back(scanString(startLine, startColumn));
        break;
      case ';':
        tokens.push_back(Token{TokenType::SEMICOLON, ";", startLine, startColumn});
        break;
      case ',':
        tokens.push_back(Token{TokenType::COMMA, ",", startLine, startColumn});
        break;
      case ':':
        tokens.push_back(Token{TokenType::COLON, ":", startLine, startColumn});
        break;
      case '\n':
        m_line++;
        m_column = 1;
        break;
      case ' ':
      case '\t':
      case '\r':
        // whitespace has no semantic meaning in this language
        break;
      default: {
        std::string charStr(1, c);
        m_diagnostics.error("unexpected character '" + charStr + "'", startLine, startColumn, 1,
                             "remove this character or check for a typo");
        break;
      }
    }
  }

  Token eof = Token{TokenType::END_OF_FILE, "", m_line, m_column};
  tokens.push_back(eof);
  return tokens;
}
