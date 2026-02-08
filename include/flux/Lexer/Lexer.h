#pragma once

#include "flux/Common/Diagnostics.h"
#include "flux/Common/SourceLocation.h"
#include "flux/Lexer/Token.h"

#include <string>
#include <string_view>
#include <vector>

namespace flux {

/// Lexer for the Flux programming language.
/// Transforms source text into a stream of tokens.
///
/// The lexer handles:
/// - All Flux keywords (module, import, func, let, mut, struct, class, etc.)
/// - Numeric literals (decimal, hex, octal, binary, float)
/// - String and character literals with escape sequences
/// - All operators and punctuation
/// - Comments (// line and /* block */)
/// - Lifetime markers ('a)
/// - Annotations (@doc, @deprecated, @test)
///
class Lexer {
public:
  /// Construct a lexer for the given source text.
  /// The source text must outlive the lexer.
  Lexer(std::string_view source, std::string_view filename,
        DiagnosticEngine &diag);

  /// Lex the next token from the source.
  Token nextToken();

  /// Peek at the next token without consuming it.
  Token peekToken();

  /// Lex all tokens from the source.
  std::vector<Token> lexAll();

  /// Check if we've reached the end of the source.
  bool isAtEnd() const;

  /// State snapshot for save/restore (used by Parser lookahead).
  struct State {
    uint32_t current;
    uint32_t tokenStart;
    uint32_t line;
    uint32_t column;
    uint32_t tokenLine;
    uint32_t tokenColumn;
    bool hasPeeked;
    Token peekedToken;
  };
  State saveState() const;
  void restoreState(const State &s);

private:
  // Character inspection
  char peek() const;
  char peekNext() const;
  char advance();
  bool match(char expected);

  // Skipping
  void skipWhitespace();
  void skipLineComment();
  bool skipBlockComment();

  // Token producers
  Token makeToken(TokenKind kind) const;
  Token makeToken(TokenKind kind, std::string_view text) const;
  Token errorToken(const std::string &message);

  // Lexing specific token types
  Token lexIdentifierOrKeyword();
  Token lexNumber();
  Token lexString();
  Token lexChar();
  Token lexAnnotation();

  // Keyword lookup
  TokenKind identifierKind(std::string_view text) const;

  // Source state
  std::string_view source_;
  std::string_view filename_;
  uint32_t current_ = 0;    // current byte position
  uint32_t tokenStart_ = 0; // start of the current token
  uint32_t line_ = 1;
  uint32_t column_ = 1;
  uint32_t tokenLine_ = 1;
  uint32_t tokenColumn_ = 1;

  // Peeked token cache
  bool hasPeeked_ = false;
  Token peekedToken_;

  // Diagnostics
  DiagnosticEngine &diag_;
};

} // namespace flux
