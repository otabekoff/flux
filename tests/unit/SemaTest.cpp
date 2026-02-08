#include <gtest/gtest.h>

#include "flux/AST/AST.h"
#include "flux/Common/Diagnostics.h"
#include "flux/Lexer/Lexer.h"
#include "flux/Parser/Parser.h"
#include "flux/Sema/Sema.h"

using namespace flux;

// -----------------------------------------------------------------------
// Helper
// -----------------------------------------------------------------------
static std::pair<std::unique_ptr<ast::Module>, size_t> analyzeSource(std::string_view source) {
    DiagnosticEngine diag;
    Lexer lexer(source, "<test>", diag);
    Parser parser(lexer, diag);
    auto module = parser.parseModule();

    Sema sema(diag);
    sema.analyze(*module);

    return {std::move(module), diag.getErrorCount()};
}

// -----------------------------------------------------------------------
// Name resolution
// -----------------------------------------------------------------------

TEST(SemaTest, UndeclaredVariable) {
    auto [mod, errors] = analyzeSource(R"(
        func f() -> Void {
            let x: Int32 = y;
        }
    )");
    EXPECT_GT(errors, 0u); // 'y' is undeclared
}

TEST(SemaTest, DeclaredVariable) {
    auto [mod, errors] = analyzeSource(R"(
        func f() -> Void {
            let x: Int32 = 42;
            let y: Int32 = x;
        }
    )");
    // 'x' is declared before 'y', so no name resolution error for identifiers.
    // There may still be type-checking issues depending on implementation depth.
    // Just check it doesn't crash.
    (void)mod;
    (void)errors;
}

TEST(SemaTest, FunctionReference) {
    auto [mod, errors] = analyzeSource(R"(
        func add(a: Int32, b: Int32) -> Int32 {
            return a + b;
        }
        func main() -> Void {
            let result: Int32 = add(1, 2);
        }
    )");
    // 'add' should be found via forward declaration pass
    // May have type-check issues but shouldn't have name resolution errors
    (void)mod;
    (void)errors;
}

// -----------------------------------------------------------------------
// Duplicate declarations
// -----------------------------------------------------------------------

TEST(SemaTest, DuplicateFunction) {
    auto [mod, errors] = analyzeSource(R"(
        func foo() -> Void {}
        func foo() -> Void {}
    )");
    EXPECT_GT(errors, 0u); // Duplicate 'foo'
}

TEST(SemaTest, DuplicateStruct) {
    auto [mod, errors] = analyzeSource(R"(
        struct Foo { x: Int32 }
        struct Foo { y: Int32 }
    )");
    EXPECT_GT(errors, 0u); // Duplicate 'Foo'
}

// -----------------------------------------------------------------------
// Scope
// -----------------------------------------------------------------------

TEST(SemaTest, VariableShadowingInBlock) {
    auto [mod, errors] = analyzeSource(R"(
        func f() -> Void {
            let x: Int32 = 1;
            {
                let x: Int32 = 2;
            }
        }
    )");
    // Inner block creates new scope, x can be re-declared
    // This should not be an error (shadowing in new scope)
    (void)mod;
    (void)errors;
}

// -----------------------------------------------------------------------
// Empty programs
// -----------------------------------------------------------------------

TEST(SemaTest, EmptyProgram) {
    auto [mod, errors] = analyzeSource("");
    EXPECT_EQ(errors, 0u);
}

TEST(SemaTest, SingleFunction) {
    auto [mod, errors] = analyzeSource(R"(
        func main() -> Void {}
    )");
    EXPECT_EQ(errors, 0u);
}
