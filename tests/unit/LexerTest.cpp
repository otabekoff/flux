#include <gtest/gtest.h>

#include "flux/Lexer/Lexer.h"
#include "flux/Lexer/Token.h"

using namespace flux;

// -----------------------------------------------------------------------
// Helper
// -----------------------------------------------------------------------
static std::vector<Token> lex(std::string_view source)
{
    DiagnosticEngine diag;
    Lexer lexer(source, "<test>", diag);
    return lexer.lexAll();
}

// -----------------------------------------------------------------------
// Basic token tests
// -----------------------------------------------------------------------

TEST(LexerTest, EmptyInput)
{
    auto tokens = lex("");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].kind, TokenKind::Eof);
}

TEST(LexerTest, Whitespace)
{
    auto tokens = lex("   \n\t  ");
    ASSERT_EQ(tokens.size(), 1u);
    EXPECT_EQ(tokens[0].kind, TokenKind::Eof);
}

TEST(LexerTest, IntegerLiteral)
{
    auto tokens = lex("42");
    ASSERT_GE(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].kind, TokenKind::IntLiteral);
    EXPECT_EQ(tokens[0].text, "42");
}

TEST(LexerTest, FloatLiteral)
{
    auto tokens = lex("3.14");
    ASSERT_GE(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].kind, TokenKind::FloatLiteral);
    EXPECT_EQ(tokens[0].text, "3.14");
}

TEST(LexerTest, StringLiteral)
{
    auto tokens = lex("\"hello world\"");
    ASSERT_GE(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].kind, TokenKind::StringLiteral);
    EXPECT_EQ(tokens[0].text, "hello world");
}

TEST(LexerTest, CharLiteral)
{
    auto tokens = lex("'a'");
    ASSERT_GE(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].kind, TokenKind::CharLiteral);
}

TEST(LexerTest, BoolLiterals)
{
    auto tokens = lex("true false");
    ASSERT_GE(tokens.size(), 3u);
    EXPECT_EQ(tokens[0].kind, TokenKind::KwTrue);
    EXPECT_EQ(tokens[1].kind, TokenKind::KwFalse);
}

TEST(LexerTest, Keywords)
{
    auto tokens = lex("func let mut const struct class enum trait impl");
    // 9 keywords + EOF
    ASSERT_GE(tokens.size(), 10u);
    EXPECT_EQ(tokens[0].kind, TokenKind::KwFunc);
    EXPECT_EQ(tokens[1].kind, TokenKind::KwLet);
    EXPECT_EQ(tokens[2].kind, TokenKind::KwMut);
    EXPECT_EQ(tokens[3].kind, TokenKind::KwConst);
    EXPECT_EQ(tokens[4].kind, TokenKind::KwStruct);
    EXPECT_EQ(tokens[5].kind, TokenKind::KwClass);
    EXPECT_EQ(tokens[6].kind, TokenKind::KwEnum);
    EXPECT_EQ(tokens[7].kind, TokenKind::KwTrait);
    EXPECT_EQ(tokens[8].kind, TokenKind::KwImpl);
}

TEST(LexerTest, Operators)
{
    auto tokens = lex("+ - * / = == != < > <= >=");
    ASSERT_GE(tokens.size(), 12u);
    EXPECT_EQ(tokens[0].kind, TokenKind::Plus);
    EXPECT_EQ(tokens[1].kind, TokenKind::Minus);
    EXPECT_EQ(tokens[2].kind, TokenKind::Star);
    EXPECT_EQ(tokens[3].kind, TokenKind::Slash);
    EXPECT_EQ(tokens[4].kind, TokenKind::Equal);
    EXPECT_EQ(tokens[5].kind, TokenKind::EqualEqual);
    EXPECT_EQ(tokens[6].kind, TokenKind::BangEqual);
    EXPECT_EQ(tokens[7].kind, TokenKind::Less);
    EXPECT_EQ(tokens[8].kind, TokenKind::Greater);
    EXPECT_EQ(tokens[9].kind, TokenKind::LessEqual);
    EXPECT_EQ(tokens[10].kind, TokenKind::GreaterEqual);
}

TEST(LexerTest, Punctuation)
{
    auto tokens = lex("( ) { } [ ] , : ; .");
    ASSERT_GE(tokens.size(), 11u);
    EXPECT_EQ(tokens[0].kind, TokenKind::LParen);
    EXPECT_EQ(tokens[1].kind, TokenKind::RParen);
    EXPECT_EQ(tokens[2].kind, TokenKind::LBrace);
    EXPECT_EQ(tokens[3].kind, TokenKind::RBrace);
    EXPECT_EQ(tokens[4].kind, TokenKind::LBracket);
    EXPECT_EQ(tokens[5].kind, TokenKind::RBracket);
    EXPECT_EQ(tokens[6].kind, TokenKind::Comma);
    EXPECT_EQ(tokens[7].kind, TokenKind::Colon);
    EXPECT_EQ(tokens[8].kind, TokenKind::Semicolon);
    EXPECT_EQ(tokens[9].kind, TokenKind::Dot);
}

TEST(LexerTest, Identifiers)
{
    auto tokens = lex("foo bar_baz _private myVar123");
    ASSERT_GE(tokens.size(), 5u);
    for (int i = 0; i < 4; ++i)
    {
        EXPECT_EQ(tokens[i].kind, TokenKind::Identifier);
    }
    EXPECT_EQ(tokens[0].text, "foo");
    EXPECT_EQ(tokens[1].text, "bar_baz");
    EXPECT_EQ(tokens[2].text, "_private");
    EXPECT_EQ(tokens[3].text, "myVar123");
}

TEST(LexerTest, LineComment)
{
    auto tokens = lex("42 // this is a comment\n100");
    ASSERT_GE(tokens.size(), 3u);
    EXPECT_EQ(tokens[0].kind, TokenKind::IntLiteral);
    EXPECT_EQ(tokens[0].text, "42");
    EXPECT_EQ(tokens[1].kind, TokenKind::IntLiteral);
    EXPECT_EQ(tokens[1].text, "100");
}

TEST(LexerTest, BlockComment)
{
    auto tokens = lex("42 /* block comment */ 100");
    ASSERT_GE(tokens.size(), 3u);
    EXPECT_EQ(tokens[0].kind, TokenKind::IntLiteral);
    EXPECT_EQ(tokens[1].kind, TokenKind::IntLiteral);
}

TEST(LexerTest, Arrow)
{
    auto tokens = lex("-> =>");
    ASSERT_GE(tokens.size(), 3u);
    EXPECT_EQ(tokens[0].kind, TokenKind::Arrow);
    EXPECT_EQ(tokens[1].kind, TokenKind::FatArrow);
}

TEST(LexerTest, ColonColon)
{
    auto tokens = lex("std::io::println");
    ASSERT_GE(tokens.size(), 6u);
    EXPECT_EQ(tokens[0].kind, TokenKind::Identifier);
    EXPECT_EQ(tokens[1].kind, TokenKind::ColonColon);
    EXPECT_EQ(tokens[2].kind, TokenKind::Identifier);
    EXPECT_EQ(tokens[3].kind, TokenKind::ColonColon);
    EXPECT_EQ(tokens[4].kind, TokenKind::Identifier);
}

TEST(LexerTest, HexLiteral)
{
    auto tokens = lex("0xFF 0x1A2B");
    ASSERT_GE(tokens.size(), 3u);
    EXPECT_EQ(tokens[0].kind, TokenKind::IntLiteral);
    EXPECT_EQ(tokens[1].kind, TokenKind::IntLiteral);
}

TEST(LexerTest, BinaryLiteral)
{
    auto tokens = lex("0b1010 0b11001100");
    ASSERT_GE(tokens.size(), 3u);
    EXPECT_EQ(tokens[0].kind, TokenKind::IntLiteral);
    EXPECT_EQ(tokens[1].kind, TokenKind::IntLiteral);
}

TEST(LexerTest, NumberWithUnderscores)
{
    auto tokens = lex("1_000_000");
    ASSERT_GE(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].kind, TokenKind::IntLiteral);
}

TEST(LexerTest, SourceLocation)
{
    auto tokens = lex("let x: Int32 = 42;");
    EXPECT_EQ(tokens[0].location.line, 1u);
    EXPECT_EQ(tokens[0].location.column, 1u);
}

// -----------------------------------------------------------------------
// Flux-specific syntax
// -----------------------------------------------------------------------

TEST(LexerTest, FunctionDeclaration)
{
    auto tokens = lex("func add(a: Int32, b: Int32) -> Int32 { return a + b; }");
    ASSERT_GE(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].kind, TokenKind::KwFunc);
    EXPECT_EQ(tokens[1].kind, TokenKind::Identifier);
    EXPECT_EQ(tokens[1].text, "add");
}

TEST(LexerTest, AsyncAwait)
{
    auto tokens = lex("async func fetch() -> String { let x: String = await getData(); return x; }");
    ASSERT_GE(tokens.size(), 2u);
    EXPECT_EQ(tokens[0].kind, TokenKind::KwAsync);
    EXPECT_EQ(tokens[1].kind, TokenKind::KwFunc);
}

TEST(LexerTest, Annotation)
{
    auto tokens = lex("@inline\nfunc testAdd() -> Void {}");
    ASSERT_GE(tokens.size(), 3u);
    EXPECT_EQ(tokens[0].kind, TokenKind::At);
    EXPECT_EQ(tokens[1].kind, TokenKind::Identifier);
    EXPECT_EQ(tokens[1].text, "inline");
}
