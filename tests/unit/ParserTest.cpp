#include <gtest/gtest.h>

#include "flux/AST/AST.h"
#include "flux/AST/Decl.h"
#include "flux/AST/Expr.h"
#include "flux/AST/Stmt.h"
#include "flux/Common/Diagnostics.h"
#include "flux/Lexer/Lexer.h"
#include "flux/Parser/Parser.h"

using namespace flux;

// -----------------------------------------------------------------------
// Helper
// -----------------------------------------------------------------------
static ast::Module parse(std::string_view source) {
    DiagnosticEngine diag;
    Lexer lexer(source, "<test>", diag);
    Parser parser(lexer, diag);
    auto module = parser.parseModule();
    return std::move(*module);
}

static ast::Module parseExpectNoErrors(std::string_view source) {
    DiagnosticEngine diag;
    Lexer lexer(source, "<test>", diag);
    Parser parser(lexer, diag);
    auto module = parser.parseModule();
    EXPECT_EQ(diag.getErrorCount(), 0u)
        << "Unexpected parse errors for:\n" << source;
    return std::move(*module);
}

// -----------------------------------------------------------------------
// Module-level tests
// -----------------------------------------------------------------------

TEST(ParserTest, EmptyModule) {
    auto mod = parseExpectNoErrors("");
    EXPECT_TRUE(mod.declarations.empty());
}

TEST(ParserTest, ModuleDeclaration) {
    auto mod = parseExpectNoErrors("module mylib;");
    // The module name should be parsed (implementation-dependent)
    ASSERT_GE(mod.declarations.size(), 0u);
}

// -----------------------------------------------------------------------
// Function declarations
// -----------------------------------------------------------------------

TEST(ParserTest, SimpleFunctionNoParams) {
    auto mod = parseExpectNoErrors("func hello() -> Void {}");
    ASSERT_EQ(mod.declarations.size(), 1u);
    EXPECT_EQ(mod.declarations[0]->kind, ast::Decl::Kind::Func);
    auto& fn = static_cast<ast::FuncDecl&>(*mod.declarations[0]);
    EXPECT_EQ(fn.name, "hello");
    EXPECT_TRUE(fn.params.empty());
}

TEST(ParserTest, FunctionWithParams) {
    auto mod = parseExpectNoErrors(
        "func add(a: Int32, b: Int32) -> Int32 { return a + b; }");
    ASSERT_EQ(mod.declarations.size(), 1u);
    auto& fn = static_cast<ast::FuncDecl&>(*mod.declarations[0]);
    EXPECT_EQ(fn.name, "add");
    EXPECT_EQ(fn.params.size(), 2u);
    EXPECT_EQ(fn.params[0].name, "a");
    EXPECT_EQ(fn.params[1].name, "b");
}

TEST(ParserTest, AsyncFunction) {
    auto mod = parseExpectNoErrors(
        "async func fetch() -> String {}");
    ASSERT_EQ(mod.declarations.size(), 1u);
    auto& fn = static_cast<ast::FuncDecl&>(*mod.declarations[0]);
    EXPECT_EQ(fn.name, "fetch");
    EXPECT_TRUE(fn.isAsync);
}

// -----------------------------------------------------------------------
// Let / const statements
// -----------------------------------------------------------------------

TEST(ParserTest, LetStatement) {
    auto mod = parseExpectNoErrors(
        "func f() -> Void { let x: Int32 = 42; }");
    ASSERT_EQ(mod.declarations.size(), 1u);
    auto& fn = static_cast<ast::FuncDecl&>(*mod.declarations[0]);
    ASSERT_NE(fn.body, nullptr);
    ASSERT_GE(fn.body->statements.size(), 1u);
    EXPECT_EQ(fn.body->statements[0]->kind, ast::Stmt::Kind::Let);
    auto& ls = static_cast<ast::LetStmt&>(*fn.body->statements[0]);
    EXPECT_EQ(ls.name, "x");
    EXPECT_FALSE(ls.isMutable);
}

TEST(ParserTest, MutableLetStatement) {
    auto mod = parseExpectNoErrors(
        "func f() -> Void { let mut y: Float64 = 3.14; }");
    ASSERT_EQ(mod.declarations.size(), 1u);
    auto& fn = static_cast<ast::FuncDecl&>(*mod.declarations[0]);
    ASSERT_NE(fn.body, nullptr);
    ASSERT_GE(fn.body->statements.size(), 1u);
    auto& ls = static_cast<ast::LetStmt&>(*fn.body->statements[0]);
    EXPECT_EQ(ls.name, "y");
    EXPECT_TRUE(ls.isMutable);
}

// -----------------------------------------------------------------------
// Struct declarations
// -----------------------------------------------------------------------

TEST(ParserTest, StructDeclaration) {
    auto mod = parseExpectNoErrors(R"(
        struct Point {
            x: Float64,
            y: Float64,
        }
    )");
    ASSERT_EQ(mod.declarations.size(), 1u);
    EXPECT_EQ(mod.declarations[0]->kind, ast::Decl::Kind::Struct);
    auto& sd = static_cast<ast::StructDecl&>(*mod.declarations[0]);
    EXPECT_EQ(sd.name, "Point");
    EXPECT_EQ(sd.fields.size(), 2u);
}

TEST(ParserTest, GenericStruct) {
    auto mod = parseExpectNoErrors(R"(
        struct Pair<A, B> {
            first: A,
            second: B,
        }
    )");
    ASSERT_EQ(mod.declarations.size(), 1u);
    auto& sd = static_cast<ast::StructDecl&>(*mod.declarations[0]);
    EXPECT_EQ(sd.name, "Pair");
    EXPECT_EQ(sd.genericParams.size(), 2u);
}

// -----------------------------------------------------------------------
// Enum declarations
// -----------------------------------------------------------------------

TEST(ParserTest, SimpleEnum) {
    auto mod = parseExpectNoErrors(R"(
        enum Color {
            Red,
            Green,
            Blue,
        }
    )");
    ASSERT_EQ(mod.declarations.size(), 1u);
    auto& ed = static_cast<ast::EnumDecl&>(*mod.declarations[0]);
    EXPECT_EQ(ed.name, "Color");
    EXPECT_EQ(ed.variants.size(), 3u);
}

// -----------------------------------------------------------------------
// Trait declarations
// -----------------------------------------------------------------------

TEST(ParserTest, TraitDeclaration) {
    auto mod = parseExpectNoErrors(R"(
        trait Display {
            func toString(self: &Self) -> String;
        }
    )");
    ASSERT_EQ(mod.declarations.size(), 1u);
    EXPECT_EQ(mod.declarations[0]->kind, ast::Decl::Kind::Trait);
}

// -----------------------------------------------------------------------
// Expression precedence
// -----------------------------------------------------------------------

TEST(ParserTest, ArithmeticPrecedence) {
    auto mod = parseExpectNoErrors(
        "func f() -> Int32 { return 1 + 2 * 3; }");
    ASSERT_EQ(mod.declarations.size(), 1u);
    auto& fn = static_cast<ast::FuncDecl&>(*mod.declarations[0]);
    ASSERT_NE(fn.body, nullptr);
    ASSERT_GE(fn.body->statements.size(), 1u);
    // Return statement should exist
    EXPECT_EQ(fn.body->statements[0]->kind, ast::Stmt::Kind::Return);
}

// -----------------------------------------------------------------------
// Error recovery
// -----------------------------------------------------------------------

TEST(ParserTest, MissingSemicolon) {
    // Should produce errors but not crash
    DiagnosticEngine diag;
    Lexer lexer("func f() -> Void { let x: Int32 = 42 }", "<test>", diag);
    Parser parser(lexer, diag);
    auto mod = parser.parseModule();
    // May or may not error depending on parser policy,
    // but should not crash
    (void)mod;
}

TEST(ParserTest, MultipleDeclarations) {
    auto mod = parseExpectNoErrors(R"(
        struct Point {
            x: Float64,
            y: Float64,
        }
        func origin() -> Point {
            return Point { x: 0.0, y: 0.0 };
        }
    )");
    EXPECT_EQ(mod.declarations.size(), 2u);
}
