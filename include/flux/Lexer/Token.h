#pragma once

#include "flux/Common/SourceLocation.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace flux {

/// Every token kind in the Flux language.
enum class TokenKind : uint16_t {
  // Special
  Eof,
  Invalid,
  Newline,

  // Literals
  IntLiteral,    // 42, 0xFF, 0b1010, 0o77
  FloatLiteral,  // 3.14, 1.0e10
  StringLiteral, // "hello"
  CharLiteral,   // 'a'
  BoolLiteral,   // true, false

  // Identifier
  Identifier, // user-defined names

  // ---- Keywords ----
  // Declarations
  KwModule,   // module
  KwImport,   // import
  KwFunc,     // func
  KwLet,      // let
  KwMut,      // mut
  KwConst,    // const
  KwStruct,   // struct
  KwClass,    // class
  KwEnum,     // enum
  KwTrait,    // trait
  KwImpl,     // impl
  KwType,     // type
  KwSelf,     // self
  KwSelfType, // Self (capital)

  // Control flow
  KwIf,       // if
  KwElse,     // else
  KwMatch,    // match
  KwFor,      // for
  KwWhile,    // while
  KwLoop,     // loop
  KwBreak,    // break
  KwContinue, // continue
  KwReturn,   // return
  KwIn,       // in

  // Ownership & borrowing
  KwMove, // move
  KwRef,  // ref
  KwDrop, // drop

  // Concurrency
  KwAsync, // async
  KwAwait, // await
  KwSpawn, // spawn

  // Safety
  KwUnsafe, // unsafe

  // Visibility
  KwPub,     // pub
  KwPublic,  // public
  KwPrivate, // private

  // Boolean / Logic
  KwTrue,  // true
  KwFalse, // false
  KwAnd,   // and
  KwOr,    // or
  KwNot,   // not

  // Misc keywords
  KwAs,     // as (type casting)
  KwIs,     // is (type checking)
  KwWhere,  // where (generic constraints)
  KwUse,    // use
  KwVoid,   // Void
  KwPanic,  // panic
  KwAssert, // assert

  // Annotations
  KwDoc,        // @doc
  KwDeprecated, // @deprecated
  KwTest,       // @test

  // ---- Punctuation / Operators ----
  // Delimiters
  LParen,   // (
  RParen,   // )
  LBracket, // [
  LBrace,   // {
  RBrace,   // }
  RBracket, // ]

  // Separators
  Comma,      // ,
  Semicolon,  // ;
  Colon,      // :
  ColonColon, // ::
  Dot,        // .
  DotDot,     // ..
  DotDotDot,  // ...
  Arrow,      // ->
  FatArrow,   // =>
  At,         // @
  Hash,       // #
  HashBang,   // #!

  // Arithmetic
  Plus,    // +
  Minus,   // -
  Star,    // *
  Slash,   // /
  Percent, // %

  // Comparison
  Equal,        // =
  EqualEqual,   // ==
  BangEqual,    // !=
  Less,         // <
  LessEqual,    // <=
  Greater,      // >
  GreaterEqual, // >=

  // Bitwise
  Ampersand,  // &
  Pipe,       // |
  Caret,      // ^
  Tilde,      // ~
  ShiftLeft,  // <<
  ShiftRight, // >>

  // Compound assignment
  PlusEqual,      // +=
  MinusEqual,     // -=
  StarEqual,      // *=
  SlashEqual,     // /=
  PercentEqual,   // %=
  AmpersandEqual, // &=
  PipeEqual,      // |=
  CaretEqual,     // ^=

  // Special operators
  Question,   // ?
  Underscore, // _ (wildcard pattern)

  // Lifetime
  Apostrophe, // ' (lifetime marker)
};

/// A token produced by the lexer.
struct Token {
  TokenKind kind = TokenKind::Eof;
  std::string_view text; // view into the source buffer
  SourceLocation location;

  // For numeric literals, the parsed value
  union {
    int64_t intValue;
    double floatValue;
  };

  bool is(TokenKind k) const { return kind == k; }
  bool isNot(TokenKind k) const { return kind != k; }
  bool isOneOf(TokenKind k1, TokenKind k2) const { return is(k1) || is(k2); }

  template <typename... Ts>
  bool isOneOf(TokenKind k1, TokenKind k2, Ts... ks) const {
    return is(k1) || isOneOf(k2, ks...);
  }

  bool isKeyword() const {
    return kind >= TokenKind::KwModule && kind <= TokenKind::KwTest;
  }

  bool isLiteral() const {
    return kind >= TokenKind::IntLiteral && kind <= TokenKind::BoolLiteral;
  }

  bool isOperator() const {
    return kind >= TokenKind::Plus && kind <= TokenKind::Underscore;
  }

  /// Get a human-readable name for the token kind.
  static const char *kindToString(TokenKind kind);
};

} // namespace flux
