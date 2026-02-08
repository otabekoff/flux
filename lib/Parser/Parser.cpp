#include "flux/Parser/Parser.h"

#include <cassert>
#include <sstream>

namespace flux {

Parser::Parser(Lexer &lexer, DiagnosticEngine &diag)
    : lexer_(lexer), diag_(diag) {
  // Prime the parser with the first token
  current_ = lexer_.nextToken();
}

// ============================================================================
// Token Management
// ============================================================================

Token Parser::advance() {
  previous_ = current_;
  current_ = lexer_.nextToken();
  return previous_;
}

bool Parser::check(TokenKind kind) const { return current_.kind == kind; }

bool Parser::match(TokenKind kind) {
  if (check(kind)) {
    advance();
    return true;
  }
  return false;
}

Token Parser::expect(TokenKind kind, const std::string &message) {
  if (check(kind)) {
    return advance();
  }
  diag_.emitError(current_.location,
                  message + ", got '" + std::string(current_.text) + "'");
  return current_;
}

Token Parser::expectSemicolon() {
  return expect(TokenKind::Semicolon, "expected ';'");
}

// ============================================================================
// Module parsing (top-level entry point)
// ============================================================================

std::unique_ptr<ast::Module> Parser::parseModule() {
  auto module = std::make_unique<ast::Module>();
  module->location = current_.location;

  // Optional module declaration
  if (check(TokenKind::KwModule)) {
    auto moduleDecl = parseModuleDecl();
    if (moduleDecl) {
      // Join path segments into module name
      std::string name;
      for (size_t i = 0; i < moduleDecl->path.size(); ++i) {
        if (i > 0)
          name += "::";
        name += moduleDecl->path[i];
      }
      module->name = std::move(name);
    }
  }

  // Import declarations
  while (check(TokenKind::KwImport)) {
    auto importDecl = parseImportDecl();
    if (importDecl) {
      std::string path;
      for (size_t i = 0; i < importDecl->path.size(); ++i) {
        if (i > 0)
          path += "::";
        path += importDecl->path[i];
      }
      module->imports.push_back(std::move(path));
    }
  }

  // Top-level declarations
  while (!check(TokenKind::Eof)) {
    auto decl = parseDeclaration();
    if (decl) {
      module->declarations.push_back(std::move(decl));
    } else {
      // Error recovery: skip token
      if (!check(TokenKind::Eof)) {
        advance();
      }
    }
  }

  return module;
}

// ============================================================================
// Declaration parsing
// ============================================================================

ast::DeclPtr Parser::parseDeclaration() {
  // Skip annotations for now
  while (check(TokenKind::At) || check(TokenKind::KwDoc) ||
         check(TokenKind::KwDeprecated) || check(TokenKind::KwTest) ||
         check(TokenKind::Hash) || check(TokenKind::HashBang)) {
    advance();
    // Skip annotation arguments
    if (check(TokenKind::LParen)) {
      advance();
      int depth = 1;
      while (depth > 0 && !check(TokenKind::Eof)) {
        if (check(TokenKind::LParen))
          depth++;
        if (check(TokenKind::RParen))
          depth--;
        advance();
      }
    }
  }

  if (check(TokenKind::KwFunc))
    return parseFuncDecl();
  if (check(TokenKind::KwAsync)) {
    advance();
    return parseFuncDecl(/*isAsync=*/true);
  }
  if (check(TokenKind::KwStruct))
    return parseStructDecl();
  if (check(TokenKind::KwClass))
    return parseClassDecl();
  if (check(TokenKind::KwEnum))
    return parseEnumDecl();
  if (check(TokenKind::KwTrait))
    return parseTraitDecl();
  if (check(TokenKind::KwImpl))
    return parseImplDecl();
  if (check(TokenKind::KwType))
    return parseTypeAliasDecl();
  if (check(TokenKind::KwPub) || check(TokenKind::KwPublic) ||
      check(TokenKind::KwPrivate)) {
    advance();                 // consume visibility modifier
    return parseDeclaration(); // parse the actual declaration
  }

  // Not a declaration — try statement (for top-level let/const or function
  // body)
  if (check(TokenKind::KwLet) || check(TokenKind::KwConst)) {
    // Wrap in a function for now; real module-level vars come later
    diag_.emitError(current_.location, "top-level let/const statements are not "
                                       "yet supported outside functions");
    synchronize();
    return nullptr;
  }

  diag_.emitError(
      current_.location,
      "expected declaration (func, struct, class, enum, trait, impl, type)");
  synchronize();
  return nullptr;
}

std::unique_ptr<ast::ModuleDecl> Parser::parseModuleDecl() {
  auto loc = current_.location;
  expect(TokenKind::KwModule, "expected 'module'");
  auto path = parsePath();
  expectSemicolon();
  return std::make_unique<ast::ModuleDecl>(std::move(path), loc);
}

std::unique_ptr<ast::ImportDecl> Parser::parseImportDecl() {
  auto loc = current_.location;
  expect(TokenKind::KwImport, "expected 'import'");
  auto path = parsePath();
  expectSemicolon();
  return std::make_unique<ast::ImportDecl>(std::move(path), std::nullopt, loc);
}

std::vector<std::string> Parser::parsePath() {
  std::vector<std::string> segments;

  auto tok = expect(TokenKind::Identifier, "expected identifier in path");
  segments.push_back(std::string(tok.text));

  while (match(TokenKind::ColonColon)) {
    tok = expect(TokenKind::Identifier, "expected identifier after '::'");
    segments.push_back(std::string(tok.text));
  }

  return segments;
}

std::unique_ptr<ast::FuncDecl> Parser::parseFuncDecl(bool isAsync) {
  auto loc = current_.location;
  expect(TokenKind::KwFunc, "expected 'func'");

  auto nameTok = expect(TokenKind::Identifier, "expected function name");
  std::string name(nameTok.text);

  // Optional generic parameters
  auto genericParams = parseGenericParams();

  // Parameters
  expect(TokenKind::LParen, "expected '(' in function declaration");
  auto params = parseFuncParams();
  expect(TokenKind::RParen, "expected ')' after parameters");

  // Return type
  ast::TypeNodePtr returnType;
  if (match(TokenKind::Arrow)) {
    returnType = parseType();
  }

  // Body
  std::unique_ptr<ast::BlockStmt> body;
  if (check(TokenKind::LBrace)) {
    body = parseBlock();
  } else {
    expectSemicolon(); // trait method declaration with no body
  }

  auto func = std::make_unique<ast::FuncDecl>(
      std::move(name), std::move(params), std::move(returnType),
      std::move(body), loc);
  func->genericParams = std::move(genericParams);
  func->isAsync = isAsync;
  return func;
}

std::vector<ast::GenericParam> Parser::parseGenericParams() {
  std::vector<ast::GenericParam> params;
  if (!match(TokenKind::Less))
    return params;

  while (!check(TokenKind::Greater) && !check(TokenKind::Eof)) {
    ast::GenericParam param;
    param.location = current_.location;

    // Lifetime parameter
    if (check(TokenKind::Apostrophe)) {
      advance();
      auto nameTok = expect(TokenKind::Identifier, "expected lifetime name");
      param.lifetime = std::string(nameTok.text);
    } else {
      auto nameTok =
          expect(TokenKind::Identifier, "expected type parameter name");
      param.name = std::string(nameTok.text);

      // Trait bounds: T: Comparable + Clone
      if (match(TokenKind::Colon)) {
        auto boundTok = expect(TokenKind::Identifier, "expected trait bound");
        param.traitBounds.push_back(std::string(boundTok.text));

        while (match(TokenKind::Plus)) {
          boundTok = expect(TokenKind::Identifier, "expected trait bound");
          param.traitBounds.push_back(std::string(boundTok.text));
        }
      }
    }

    params.push_back(std::move(param));
    if (!match(TokenKind::Comma))
      break;
  }

  expect(TokenKind::Greater, "expected '>' after generic parameters");
  return params;
}

std::vector<ast::FuncParam> Parser::parseFuncParams() {
  std::vector<ast::FuncParam> params;
  if (check(TokenKind::RParen))
    return params;

  params.push_back(parseFuncParam());
  while (match(TokenKind::Comma)) {
    if (check(TokenKind::RParen))
      break;
    params.push_back(parseFuncParam());
  }

  return params;
}

ast::FuncParam Parser::parseFuncParam() {
  ast::FuncParam param;
  param.location = current_.location;

  // Check for 'mut' modifier
  if (check(TokenKind::KwMut)) {
    param.isMutable = true;
    advance();

    // Check for 'mut ref' or 'mut self'
    if (check(TokenKind::KwRef)) {
      param.isMutRef = true;
      advance();
    }
  }

  // Check for 'ref' modifier (without mut)
  if (check(TokenKind::KwRef) && !param.isMutRef) {
    param.isRef = true;
    advance();
  }

  // Parameter name — accept 'self' keyword as parameter name too
  Token nameTok;
  if (check(TokenKind::KwSelf)) {
    nameTok = advance();
    param.isSelf = true;
  } else {
    nameTok = expect(TokenKind::Identifier, "expected parameter name");
  }
  param.name = std::string(nameTok.text);

  // Type annotation
  expect(TokenKind::Colon, "expected ':' after parameter name");
  param.type = parseType();

  return param;
}

std::unique_ptr<ast::StructDecl> Parser::parseStructDecl() {
  auto loc = current_.location;
  expect(TokenKind::KwStruct, "expected 'struct'");

  auto nameTok = expect(TokenKind::Identifier, "expected struct name");
  std::string name(nameTok.text);

  auto genericParams = parseGenericParams();

  expect(TokenKind::LBrace, "expected '{' in struct declaration");
  auto fields = parseStructFields();
  expect(TokenKind::RBrace, "expected '}' after struct fields");

  auto decl = std::make_unique<ast::StructDecl>(std::move(name),
                                                std::move(fields), loc);
  decl->genericParams = std::move(genericParams);
  return decl;
}

std::vector<ast::FieldDecl> Parser::parseStructFields() {
  std::vector<ast::FieldDecl> fields;

  while (!check(TokenKind::RBrace) && !check(TokenKind::Eof)) {
    ast::FieldDecl field;
    field.location = current_.location;

    auto nameTok = expect(TokenKind::Identifier, "expected field name");
    field.name = std::string(nameTok.text);

    expect(TokenKind::Colon, "expected ':' after field name");
    field.type = parseType();

    fields.push_back(std::move(field));

    if (!match(TokenKind::Comma))
      break;
  }

  return fields;
}

std::unique_ptr<ast::ClassDecl> Parser::parseClassDecl() {
  auto loc = current_.location;
  expect(TokenKind::KwClass, "expected 'class'");

  auto nameTok = expect(TokenKind::Identifier, "expected class name");
  std::string name(nameTok.text);

  auto genericParams = parseGenericParams();

  expect(TokenKind::LBrace, "expected '{' in class declaration");
  auto fields = parseClassFields();
  expect(TokenKind::RBrace, "expected '}' after class fields");

  auto decl =
      std::make_unique<ast::ClassDecl>(std::move(name), std::move(fields), loc);
  decl->genericParams = std::move(genericParams);
  return decl;
}

std::vector<ast::FieldDecl> Parser::parseClassFields() {
  std::vector<ast::FieldDecl> fields;

  while (!check(TokenKind::RBrace) && !check(TokenKind::Eof)) {
    ast::FieldDecl field;
    field.location = current_.location;

    // Visibility modifier
    if (match(TokenKind::KwPublic) || match(TokenKind::KwPub)) {
      field.visibility = ast::Decl::Visibility::Public;
    } else if (match(TokenKind::KwPrivate)) {
      field.visibility = ast::Decl::Visibility::Private;
    }

    auto nameTok = expect(TokenKind::Identifier, "expected field name");
    field.name = std::string(nameTok.text);

    expect(TokenKind::Colon, "expected ':' after field name");
    field.type = parseType();

    fields.push_back(std::move(field));

    if (!match(TokenKind::Comma))
      break;
  }

  return fields;
}

std::unique_ptr<ast::EnumDecl> Parser::parseEnumDecl() {
  auto loc = current_.location;
  expect(TokenKind::KwEnum, "expected 'enum'");

  auto nameTok = expect(TokenKind::Identifier, "expected enum name");
  std::string name(nameTok.text);

  auto genericParams = parseGenericParams();

  expect(TokenKind::LBrace, "expected '{' in enum declaration");
  auto variants = parseEnumVariants();
  expect(TokenKind::RBrace, "expected '}' after enum variants");

  auto decl = std::make_unique<ast::EnumDecl>(std::move(name),
                                              std::move(variants), loc);
  decl->genericParams = std::move(genericParams);
  return decl;
}

std::vector<ast::EnumVariant> Parser::parseEnumVariants() {
  std::vector<ast::EnumVariant> variants;

  while (!check(TokenKind::RBrace) && !check(TokenKind::Eof)) {
    ast::EnumVariant variant;
    variant.location = current_.location;

    auto nameTok = expect(TokenKind::Identifier, "expected variant name");
    variant.name = std::string(nameTok.text);

    if (match(TokenKind::LParen)) {
      // Tuple variant: Write(String)
      variant.variantKind = ast::EnumVariant::VariantKind::Tuple;
      while (!check(TokenKind::RParen) && !check(TokenKind::Eof)) {
        variant.tupleFields.push_back(parseType());
        if (!match(TokenKind::Comma))
          break;
      }
      expect(TokenKind::RParen, "expected ')' after tuple variant fields");
    } else if (match(TokenKind::LBrace)) {
      // Struct variant: Move { x: Int32, y: Int32 }
      variant.variantKind = ast::EnumVariant::VariantKind::Struct;
      while (!check(TokenKind::RBrace) && !check(TokenKind::Eof)) {
        ast::FieldDecl field;
        field.location = current_.location;
        auto fieldName = expect(TokenKind::Identifier, "expected field name");
        field.name = std::string(fieldName.text);
        expect(TokenKind::Colon, "expected ':' after field name");
        field.type = parseType();
        variant.structFields.push_back(std::move(field));
        if (!match(TokenKind::Comma))
          break;
      }
      expect(TokenKind::RBrace, "expected '}' after struct variant fields");
    } else {
      variant.variantKind = ast::EnumVariant::VariantKind::Unit;
    }

    variants.push_back(std::move(variant));
    if (!match(TokenKind::Comma))
      break;
  }

  return variants;
}

std::unique_ptr<ast::TraitDecl> Parser::parseTraitDecl() {
  auto loc = current_.location;
  expect(TokenKind::KwTrait, "expected 'trait'");

  auto nameTok = expect(TokenKind::Identifier, "expected trait name");
  std::string name(nameTok.text);

  auto genericParams = parseGenericParams();

  std::vector<std::string> superTraits;
  if (match(TokenKind::Colon)) {
    auto traitTok = expect(TokenKind::Identifier, "expected super trait name");
    superTraits.push_back(std::string(traitTok.text));
    while (match(TokenKind::Plus)) {
      traitTok = expect(TokenKind::Identifier, "expected trait name");
      superTraits.push_back(std::string(traitTok.text));
    }
  }

  expect(TokenKind::LBrace, "expected '{' in trait declaration");

  std::vector<std::unique_ptr<ast::FuncDecl>> methods;
  while (!check(TokenKind::RBrace) && !check(TokenKind::Eof)) {
    bool async = false;
    if (check(TokenKind::KwAsync)) {
      async = true;
      advance();
    }
    if (check(TokenKind::KwFunc)) {
      methods.push_back(parseFuncDecl(async));
    } else {
      diag_.emitError(current_.location,
                      "expected method declaration in trait");
      advance();
    }
  }

  expect(TokenKind::RBrace, "expected '}' after trait methods");

  auto decl = std::make_unique<ast::TraitDecl>(std::move(name),
                                               std::move(methods), loc);
  decl->genericParams = std::move(genericParams);
  decl->superTraits = std::move(superTraits);
  return decl;
}

std::unique_ptr<ast::ImplDecl> Parser::parseImplDecl() {
  auto loc = current_.location;
  expect(TokenKind::KwImpl, "expected 'impl'");

  auto genericParams = parseGenericParams();

  // Parse the type or trait name
  auto firstType = parseType();

  std::optional<std::string> traitName;
  ast::TypeNodePtr targetType;

  // Check if this is "impl Trait for Type"
  if (match(TokenKind::KwFor)) {
    // firstType was actually the trait name
    if (firstType->kind == ast::TypeNode::Kind::Named) {
      traitName = static_cast<ast::NamedType *>(firstType.get())->path.back();
    }
    targetType = parseType();
  } else {
    targetType = std::move(firstType);
  }

  expect(TokenKind::LBrace, "expected '{' in impl block");

  std::vector<std::unique_ptr<ast::FuncDecl>> methods;
  while (!check(TokenKind::RBrace) && !check(TokenKind::Eof)) {
    bool async = false;
    if (check(TokenKind::KwAsync)) {
      async = true;
      advance();
    }
    if (check(TokenKind::KwFunc)) {
      methods.push_back(parseFuncDecl(async));
    } else {
      diag_.emitError(current_.location,
                      "expected method declaration in impl block");
      advance();
    }
  }

  expect(TokenKind::RBrace, "expected '}' after impl block");

  auto decl = std::make_unique<ast::ImplDecl>(
      std::move(targetType), std::move(traitName), std::move(methods), loc);
  decl->genericParams = std::move(genericParams);
  return decl;
}

std::unique_ptr<ast::TypeAliasDecl> Parser::parseTypeAliasDecl() {
  auto loc = current_.location;
  expect(TokenKind::KwType, "expected 'type'");

  auto nameTok = expect(TokenKind::Identifier, "expected type alias name");
  std::string name(nameTok.text);

  auto genericParams = parseGenericParams();

  expect(TokenKind::Equal, "expected '=' in type alias");
  auto aliasedType = parseType();
  expectSemicolon();

  auto decl = std::make_unique<ast::TypeAliasDecl>(std::move(name),
                                                   std::move(aliasedType), loc);
  decl->genericParams = std::move(genericParams);
  return decl;
}

// ============================================================================
// Statement parsing
// ============================================================================

ast::StmtPtr Parser::parseStatement() {
  if (check(TokenKind::KwLet))
    return parseLetStmt();
  if (check(TokenKind::KwConst))
    return parseConstStmt();
  if (check(TokenKind::KwReturn))
    return parseReturnStmt();
  if (check(TokenKind::KwIf))
    return parseIfStmt();
  if (check(TokenKind::KwMatch))
    return parseMatchStmt();
  if (check(TokenKind::KwFor))
    return parseForStmt();
  if (check(TokenKind::KwWhile))
    return parseWhileStmt();
  if (check(TokenKind::KwLoop))
    return parseLoopStmt();
  if (check(TokenKind::KwBreak)) {
    auto loc = current_.location;
    advance();
    expectSemicolon();
    return std::make_unique<ast::BreakStmt>(loc);
  }
  if (check(TokenKind::KwContinue)) {
    auto loc = current_.location;
    advance();
    expectSemicolon();
    return std::make_unique<ast::ContinueStmt>(loc);
  }
  if (check(TokenKind::LBrace))
    return parseBlock();

  // Expression statement
  auto loc = current_.location;
  auto expr = parseExpression();
  if (!expr) {
    synchronize();
    return nullptr;
  }
  expectSemicolon();
  return std::make_unique<ast::ExprStmt>(std::move(expr), loc);
}

std::unique_ptr<ast::LetStmt> Parser::parseLetStmt() {
  auto loc = current_.location;
  expect(TokenKind::KwLet, "expected 'let'");

  bool isMutable = match(TokenKind::KwMut);

  auto nameTok = expect(TokenKind::Identifier, "expected variable name");
  std::string name(nameTok.text);

  expect(TokenKind::Colon,
         "expected ':' after variable name (Flux requires explicit types)");
  auto type = parseType();

  ast::ExprPtr init;
  if (match(TokenKind::Equal)) {
    init = parseExpression();
  }

  expectSemicolon();
  return std::make_unique<ast::LetStmt>(std::move(name), std::move(type),
                                        std::move(init), isMutable, loc);
}

std::unique_ptr<ast::ConstStmt> Parser::parseConstStmt() {
  auto loc = current_.location;
  expect(TokenKind::KwConst, "expected 'const'");

  auto nameTok = expect(TokenKind::Identifier, "expected constant name");
  std::string name(nameTok.text);

  expect(TokenKind::Colon, "expected ':' after constant name");
  auto type = parseType();

  expect(TokenKind::Equal, "expected '=' in constant declaration");
  auto value = parseExpression();

  expectSemicolon();
  return std::make_unique<ast::ConstStmt>(std::move(name), std::move(type),
                                          std::move(value), loc);
}

std::unique_ptr<ast::ReturnStmt> Parser::parseReturnStmt() {
  auto loc = current_.location;
  expect(TokenKind::KwReturn, "expected 'return'");

  ast::ExprPtr value;
  if (!check(TokenKind::Semicolon) && !check(TokenKind::RBrace)) {
    value = parseExpression();
  }

  expectSemicolon();
  return std::make_unique<ast::ReturnStmt>(std::move(value), loc);
}

std::unique_ptr<ast::IfStmt> Parser::parseIfStmt() {
  auto loc = current_.location;
  expect(TokenKind::KwIf, "expected 'if'");

  auto condition = parseExpression();
  auto thenBranch = parseBlock();

  ast::StmtPtr elseBranch;
  if (match(TokenKind::KwElse)) {
    if (check(TokenKind::KwIf)) {
      elseBranch = parseIfStmt();
    } else {
      elseBranch = parseBlock();
    }
  }

  return std::make_unique<ast::IfStmt>(
      std::move(condition), std::move(thenBranch), std::move(elseBranch), loc);
}

std::unique_ptr<ast::MatchStmt> Parser::parseMatchStmt() {
  auto loc = current_.location;
  expect(TokenKind::KwMatch, "expected 'match'");

  auto scrutinee = parseExpression();
  expect(TokenKind::LBrace, "expected '{' in match statement");

  std::vector<ast::MatchArm> arms;
  while (!check(TokenKind::RBrace) && !check(TokenKind::Eof)) {
    arms.push_back(parseMatchArm());
    // Allow optional comma between arms
    match(TokenKind::Comma);
  }

  expect(TokenKind::RBrace, "expected '}' after match arms");

  return std::make_unique<ast::MatchStmt>(std::move(scrutinee), std::move(arms),
                                          loc);
}

ast::MatchArm Parser::parseMatchArm() {
  ast::MatchArm arm;
  arm.location = current_.location;
  arm.pattern = parsePattern();

  // Optional guard
  if (match(TokenKind::KwIf)) {
    arm.guard = parseExpression();
  }

  expect(TokenKind::FatArrow, "expected '=>' in match arm");

  // Body can be a block or an expression
  if (check(TokenKind::LBrace)) {
    arm.body = parseBlockExpr();
  } else {
    arm.body = parseExpression();
  }

  return arm;
}

std::unique_ptr<ast::ForStmt> Parser::parseForStmt() {
  auto loc = current_.location;
  expect(TokenKind::KwFor, "expected 'for'");

  auto varTok = expect(TokenKind::Identifier, "expected loop variable name");
  std::string varName(varTok.text);

  expect(TokenKind::Colon, "expected ':' after loop variable name");
  auto varType = parseType();

  expect(TokenKind::KwIn, "expected 'in' in for loop");
  auto iterable = parseExpression();

  auto body = parseBlock();

  return std::make_unique<ast::ForStmt>(std::move(varName), std::move(varType),
                                        std::move(iterable), std::move(body),
                                        loc);
}

std::unique_ptr<ast::WhileStmt> Parser::parseWhileStmt() {
  auto loc = current_.location;
  expect(TokenKind::KwWhile, "expected 'while'");

  auto condition = parseExpression();
  auto body = parseBlock();

  return std::make_unique<ast::WhileStmt>(std::move(condition), std::move(body),
                                          loc);
}

std::unique_ptr<ast::LoopStmt> Parser::parseLoopStmt() {
  auto loc = current_.location;
  expect(TokenKind::KwLoop, "expected 'loop'");

  auto body = parseBlock();

  return std::make_unique<ast::LoopStmt>(std::move(body), loc);
}

std::unique_ptr<ast::BlockStmt> Parser::parseBlock() {
  auto loc = current_.location;
  expect(TokenKind::LBrace, "expected '{'");

  ast::StmtList stmts;
  while (!check(TokenKind::RBrace) && !check(TokenKind::Eof)) {
    auto stmt = parseStatement();
    if (stmt) {
      stmts.push_back(std::move(stmt));
    }
  }

  expect(TokenKind::RBrace, "expected '}'");
  return std::make_unique<ast::BlockStmt>(std::move(stmts), loc);
}

// ============================================================================
// Expression parsing (precedence climbing)
// ============================================================================

ast::ExprPtr Parser::parseExpression() { return parseAssignment(); }

ast::ExprPtr Parser::parseAssignment() {
  auto expr = parseOr();

  if (check(TokenKind::Equal)) {
    auto loc = current_.location;
    advance();
    auto value = parseAssignment(); // right-associative
    return std::make_unique<ast::AssignExpr>(std::move(expr), std::move(value),
                                             loc);
  }

  // Compound assignment
  auto compoundOp = [&]() -> std::optional<ast::CompoundAssignOp> {
    switch (current_.kind) {
    case TokenKind::PlusEqual:
      return ast::CompoundAssignOp::AddAssign;
    case TokenKind::MinusEqual:
      return ast::CompoundAssignOp::SubAssign;
    case TokenKind::StarEqual:
      return ast::CompoundAssignOp::MulAssign;
    case TokenKind::SlashEqual:
      return ast::CompoundAssignOp::DivAssign;
    case TokenKind::PercentEqual:
      return ast::CompoundAssignOp::ModAssign;
    case TokenKind::AmpersandEqual:
      return ast::CompoundAssignOp::AndAssign;
    case TokenKind::PipeEqual:
      return ast::CompoundAssignOp::OrAssign;
    case TokenKind::CaretEqual:
      return ast::CompoundAssignOp::XorAssign;
    default:
      return std::nullopt;
    }
  }();

  if (compoundOp) {
    auto loc = current_.location;
    advance();
    auto value = parseAssignment();
    return std::make_unique<ast::CompoundAssignExpr>(
        *compoundOp, std::move(expr), std::move(value), loc);
  }

  return expr;
}

ast::ExprPtr Parser::parseOr() {
  auto left = parseAnd();
  while (check(TokenKind::KwOr)) {
    auto loc = current_.location;
    advance();
    auto right = parseAnd();
    left = std::make_unique<ast::BinaryExpr>(ast::BinaryOp::Or, std::move(left),
                                             std::move(right), loc);
  }
  return left;
}

ast::ExprPtr Parser::parseAnd() {
  auto left = parseEquality();
  while (check(TokenKind::KwAnd)) {
    auto loc = current_.location;
    advance();
    auto right = parseEquality();
    left = std::make_unique<ast::BinaryExpr>(
        ast::BinaryOp::And, std::move(left), std::move(right), loc);
  }
  return left;
}

ast::ExprPtr Parser::parseEquality() {
  auto left = parseComparison();
  while (check(TokenKind::EqualEqual) || check(TokenKind::BangEqual)) {
    auto loc = current_.location;
    auto op = current_.kind == TokenKind::EqualEqual ? ast::BinaryOp::Equal
                                                     : ast::BinaryOp::NotEqual;
    advance();
    auto right = parseComparison();
    left = std::make_unique<ast::BinaryExpr>(op, std::move(left),
                                             std::move(right), loc);
  }
  return left;
}

ast::ExprPtr Parser::parseComparison() {
  auto left = parseBitwiseOr();
  while (check(TokenKind::Less) || check(TokenKind::LessEqual) ||
         check(TokenKind::Greater) || check(TokenKind::GreaterEqual)) {
    auto loc = current_.location;
    ast::BinaryOp op;
    switch (current_.kind) {
    case TokenKind::Less:
      op = ast::BinaryOp::Less;
      break;
    case TokenKind::LessEqual:
      op = ast::BinaryOp::LessEqual;
      break;
    case TokenKind::Greater:
      op = ast::BinaryOp::Greater;
      break;
    case TokenKind::GreaterEqual:
      op = ast::BinaryOp::GreaterEqual;
      break;
    default:
      op = ast::BinaryOp::Less;
      break; // unreachable
    }
    advance();
    auto right = parseBitwiseOr();
    left = std::make_unique<ast::BinaryExpr>(op, std::move(left),
                                             std::move(right), loc);
  }
  return left;
}

ast::ExprPtr Parser::parseBitwiseOr() {
  auto left = parseBitwiseXor();
  while (check(TokenKind::Pipe)) {
    auto loc = current_.location;
    advance();
    auto right = parseBitwiseXor();
    left = std::make_unique<ast::BinaryExpr>(
        ast::BinaryOp::BitOr, std::move(left), std::move(right), loc);
  }
  return left;
}

ast::ExprPtr Parser::parseBitwiseXor() {
  auto left = parseBitwiseAnd();
  while (check(TokenKind::Caret)) {
    auto loc = current_.location;
    advance();
    auto right = parseBitwiseAnd();
    left = std::make_unique<ast::BinaryExpr>(
        ast::BinaryOp::BitXor, std::move(left), std::move(right), loc);
  }
  return left;
}

ast::ExprPtr Parser::parseBitwiseAnd() {
  auto left = parseShift();
  while (check(TokenKind::Ampersand)) {
    auto loc = current_.location;
    advance();
    auto right = parseShift();
    left = std::make_unique<ast::BinaryExpr>(
        ast::BinaryOp::BitAnd, std::move(left), std::move(right), loc);
  }
  return left;
}

ast::ExprPtr Parser::parseShift() {
  auto left = parseAdditive();
  while (check(TokenKind::ShiftLeft) || check(TokenKind::ShiftRight)) {
    auto loc = current_.location;
    auto op = current_.kind == TokenKind::ShiftLeft ? ast::BinaryOp::ShiftLeft
                                                    : ast::BinaryOp::ShiftRight;
    advance();
    auto right = parseAdditive();
    left = std::make_unique<ast::BinaryExpr>(op, std::move(left),
                                             std::move(right), loc);
  }
  return left;
}

ast::ExprPtr Parser::parseAdditive() {
  auto left = parseMultiplicative();
  while (check(TokenKind::Plus) || check(TokenKind::Minus)) {
    auto loc = current_.location;
    auto op = current_.kind == TokenKind::Plus ? ast::BinaryOp::Add
                                               : ast::BinaryOp::Sub;
    advance();
    auto right = parseMultiplicative();
    left = std::make_unique<ast::BinaryExpr>(op, std::move(left),
                                             std::move(right), loc);
  }
  return left;
}

ast::ExprPtr Parser::parseMultiplicative() {
  auto left = parseUnary();
  while (check(TokenKind::Star) || check(TokenKind::Slash) ||
         check(TokenKind::Percent)) {
    auto loc = current_.location;
    ast::BinaryOp op;
    switch (current_.kind) {
    case TokenKind::Star:
      op = ast::BinaryOp::Mul;
      break;
    case TokenKind::Slash:
      op = ast::BinaryOp::Div;
      break;
    case TokenKind::Percent:
      op = ast::BinaryOp::Mod;
      break;
    default:
      op = ast::BinaryOp::Mul;
      break;
    }
    advance();
    auto right = parseUnary();
    left = std::make_unique<ast::BinaryExpr>(op, std::move(left),
                                             std::move(right), loc);
  }
  return left;
}

ast::ExprPtr Parser::parseUnary() {
  auto loc = current_.location;

  // Negate
  if (match(TokenKind::Minus)) {
    auto operand = parseUnary();
    return std::make_unique<ast::UnaryExpr>(ast::UnaryOp::Negate,
                                            std::move(operand), loc);
  }

  // Logical not
  if (match(TokenKind::KwNot)) {
    auto operand = parseUnary();
    return std::make_unique<ast::UnaryExpr>(ast::UnaryOp::Not,
                                            std::move(operand), loc);
  }

  // Bitwise not
  if (match(TokenKind::Tilde)) {
    auto operand = parseUnary();
    return std::make_unique<ast::UnaryExpr>(ast::UnaryOp::BitwiseNot,
                                            std::move(operand), loc);
  }

  // ref expr
  if (match(TokenKind::KwRef)) {
    auto operand = parseUnary();
    return std::make_unique<ast::RefExpr>(std::move(operand), loc);
  }

  // mut ref expr
  if (check(TokenKind::KwMut) && lexer_.peekToken().kind == TokenKind::KwRef) {
    advance(); // mut
    advance(); // ref
    auto operand = parseUnary();
    return std::make_unique<ast::MutRefExpr>(std::move(operand), loc);
  }

  // move expr
  if (match(TokenKind::KwMove)) {
    auto operand = parseUnary();
    return std::make_unique<ast::MoveExpr>(std::move(operand), loc);
  }

  // await expr
  if (match(TokenKind::KwAwait)) {
    auto operand = parseUnary();
    return std::make_unique<ast::AwaitExpr>(std::move(operand), loc);
  }

  auto expr = parsePrimary();

  // Postfix operators
  while (true) {
    expr = parsePostfix(std::move(expr));

    // Check if we consumed anything — if no postfix operator matched, break
    if (!check(TokenKind::LParen) && !check(TokenKind::LBracket) &&
        !check(TokenKind::Dot) && !check(TokenKind::ColonColon) &&
        !check(TokenKind::Question) && !check(TokenKind::KwAs)) {
      break;
    }

    // Need to re-parse one more postfix
  }

  return expr;
}

ast::ExprPtr Parser::parsePostfix(ast::ExprPtr left) {
  // Function call
  if (check(TokenKind::LParen)) {
    return parseCallArguments(std::move(left));
  }

  // Index
  if (check(TokenKind::LBracket)) {
    return parseIndexExpression(std::move(left));
  }

  // Member access
  if (match(TokenKind::Dot)) {
    auto loc = current_.location;
    auto memberTok =
        expect(TokenKind::Identifier, "expected member name after '.'");
    std::string member(memberTok.text);

    // Check if this is a method call
    if (check(TokenKind::LParen)) {
      advance(); // consume '('
      ast::ExprList args;
      while (!check(TokenKind::RParen) && !check(TokenKind::Eof)) {
        args.push_back(parseExpression());
        if (!match(TokenKind::Comma))
          break;
      }
      expect(TokenKind::RParen, "expected ')' after method arguments");
      return std::make_unique<ast::MethodCallExpr>(
          std::move(left), std::move(member), std::move(args), loc);
    }

    return std::make_unique<ast::MemberAccessExpr>(std::move(left),
                                                   std::move(member), loc);
  }

  // Path continuation (::)
  if (check(TokenKind::ColonColon)) {
    // Convert to path expression
    advance();
    auto loc = current_.location;
    auto nextTok =
        expect(TokenKind::Identifier, "expected identifier after '::'");

    // Build path segments
    std::vector<std::string> segments;
    if (left->kind == ast::Expr::Kind::Ident) {
      segments.push_back(static_cast<ast::IdentExpr *>(left.get())->name);
    } else if (left->kind == ast::Expr::Kind::Path) {
      segments = static_cast<ast::PathExpr *>(left.get())->segments;
    }
    segments.push_back(std::string(nextTok.text));

    while (match(TokenKind::ColonColon)) {
      auto seg =
          expect(TokenKind::Identifier, "expected identifier after '::'");
      segments.push_back(std::string(seg.text));
    }

    auto pathExpr = std::make_unique<ast::PathExpr>(std::move(segments), loc);

    // Check for struct construction: Type { field: value }
    // or function call: Path::func(args)
    return std::move(pathExpr);
  }

  // Try operator (?)
  if (match(TokenKind::Question)) {
    return std::make_unique<ast::TryExpr>(std::move(left), left->location);
  }

  // Type cast (as)
  if (match(TokenKind::KwAs)) {
    auto type = parseType();
    return std::make_unique<ast::CastExpr>(std::move(left), std::move(type),
                                           left->location);
  }

  return left;
}

ast::ExprPtr Parser::parseCallArguments(ast::ExprPtr callee) {
  auto loc = current_.location;
  expect(TokenKind::LParen, "expected '('");

  ast::ExprList args;
  while (!check(TokenKind::RParen) && !check(TokenKind::Eof)) {
    args.push_back(parseExpression());
    if (!match(TokenKind::Comma))
      break;
  }

  expect(TokenKind::RParen, "expected ')' after arguments");
  return std::make_unique<ast::CallExpr>(std::move(callee), std::move(args),
                                         loc);
}

ast::ExprPtr Parser::parseIndexExpression(ast::ExprPtr object) {
  auto loc = current_.location;
  expect(TokenKind::LBracket, "expected '['");
  auto index = parseExpression();
  expect(TokenKind::RBracket, "expected ']'");
  return std::make_unique<ast::IndexExpr>(std::move(object), std::move(index),
                                          loc);
}

ast::ExprPtr Parser::parsePrimary() {
  auto loc = current_.location;

  // Integer literal
  if (check(TokenKind::IntLiteral)) {
    auto tok = advance();
    return std::make_unique<ast::IntLiteralExpr>(tok.intValue, loc);
  }

  // Float literal
  if (check(TokenKind::FloatLiteral)) {
    auto tok = advance();
    return std::make_unique<ast::FloatLiteralExpr>(tok.floatValue, loc);
  }

  // String literal
  if (check(TokenKind::StringLiteral)) {
    auto tok = advance();
    // Text is already stored without quotes by the lexer
    std::string value(tok.text);
    return std::make_unique<ast::StringLiteralExpr>(std::move(value), loc);
  }

  // Char literal
  if (check(TokenKind::CharLiteral)) {
    auto tok = advance();
    char32_t value = tok.text[1]; // simplified
    return std::make_unique<ast::CharLiteralExpr>(value, loc);
  }

  // Bool literal (true/false keywords)
  if (check(TokenKind::KwTrue)) {
    advance();
    return std::make_unique<ast::BoolLiteralExpr>(true, loc);
  }
  if (check(TokenKind::KwFalse)) {
    advance();
    return std::make_unique<ast::BoolLiteralExpr>(false, loc);
  }

  // Identifier or path
  if (check(TokenKind::Identifier)) {
    auto tok = advance();
    std::string name(tok.text);

    // Check for :: path
    if (check(TokenKind::ColonColon)) {
      std::vector<std::string> segments;
      segments.push_back(std::move(name));
      while (match(TokenKind::ColonColon)) {
        auto seg =
            expect(TokenKind::Identifier, "expected identifier after '::'");
        segments.push_back(std::string(seg.text));
      }
      return std::make_unique<ast::PathExpr>(std::move(segments), loc);
    }

    // Check for struct construction: TypeName { ... }
    if (check(TokenKind::LBrace)) {
      // Disambiguate: struct literal if next tokens look like "ident : expr"
      auto parserSaved = saveState();
      auto lexerSaved = lexer_.saveState();
      advance(); // consume '{'
      bool isStructLiteral = false;
      if (check(TokenKind::Identifier)) {
        advance(); // consume identifier
        if (check(TokenKind::Colon)) {
          isStructLiteral = true;
        }
      } else if (check(TokenKind::RBrace)) {
        // Empty struct literal: Point {}
        isStructLiteral = true;
      }
      // Restore state
      restoreState(parserSaved);
      lexer_.restoreState(lexerSaved);

      if (isStructLiteral) {
        advance(); // consume '{'
        std::vector<std::pair<std::string, ast::ExprPtr>> fields;
        while (!check(TokenKind::RBrace) && !check(TokenKind::Eof)) {
          auto fieldName = expect(TokenKind::Identifier, "expected field name");
          expect(TokenKind::Colon, "expected ':' after field name");
          auto value = parseExpression();
          fields.emplace_back(std::string(fieldName.text), std::move(value));
          if (!match(TokenKind::Comma))
            break;
        }
        expect(TokenKind::RBrace, "expected '}' after struct literal");
        return std::make_unique<ast::StructLiteralExpr>(std::move(name),
                                                        std::move(fields), loc);
      }
    }

    return std::make_unique<ast::IdentExpr>(std::move(name), loc);
  }

  // Parenthesized expression or tuple
  if (match(TokenKind::LParen)) {
    if (check(TokenKind::RParen)) {
      advance();
      // Empty tuple / void
      return std::make_unique<ast::TupleExpr>(ast::ExprList{}, loc);
    }

    auto first = parseExpression();

    if (check(TokenKind::Comma)) {
      // Tuple
      ast::ExprList elements;
      elements.push_back(std::move(first));
      while (match(TokenKind::Comma)) {
        if (check(TokenKind::RParen))
          break;
        elements.push_back(parseExpression());
      }
      expect(TokenKind::RParen, "expected ')' after tuple");
      return std::make_unique<ast::TupleExpr>(std::move(elements), loc);
    }

    expect(TokenKind::RParen, "expected ')'");
    return first; // Parenthesized expression
  }

  // Block expression
  if (check(TokenKind::LBrace)) {
    return parseBlockExpr();
  }

  // If expression
  if (check(TokenKind::KwIf)) {
    return parseIfExpr();
  }

  // Match expression
  if (check(TokenKind::KwMatch)) {
    return parseMatchExpr();
  }

  // Closure: |params| -> RetType { body } or || { body }
  if (check(TokenKind::Pipe)) {
    return parseClosureExpr();
  }

  // Wildcard
  if (match(TokenKind::Underscore)) {
    return std::make_unique<ast::IdentExpr>("_", loc);
  }

  diag_.emitError(loc, "expected expression, got '" +
                           std::string(current_.text) + "'");
  return nullptr;
}

ast::ExprPtr Parser::parseBlockExpr() {
  auto loc = current_.location;
  expect(TokenKind::LBrace, "expected '{'");

  ast::StmtList stmts;
  while (!check(TokenKind::RBrace) && !check(TokenKind::Eof)) {
    auto stmt = parseStatement();
    if (stmt) {
      stmts.push_back(std::move(stmt));
    }
  }

  expect(TokenKind::RBrace, "expected '}'");
  return std::make_unique<ast::BlockExpr>(std::move(stmts), nullptr, loc);
}

ast::ExprPtr Parser::parseIfExpr() {
  auto loc = current_.location;
  expect(TokenKind::KwIf, "expected 'if'");

  auto condition = parseExpression();
  auto thenBranch = parseBlockExpr();

  ast::ExprPtr elseBranch;
  if (match(TokenKind::KwElse)) {
    if (check(TokenKind::KwIf)) {
      elseBranch = parseIfExpr();
    } else {
      elseBranch = parseBlockExpr();
    }
  }

  return std::make_unique<ast::IfExpr>(
      std::move(condition), std::move(thenBranch), std::move(elseBranch), loc);
}

ast::ExprPtr Parser::parseMatchExpr() {
  auto loc = current_.location;
  expect(TokenKind::KwMatch, "expected 'match'");

  auto scrutinee = parseExpression();
  expect(TokenKind::LBrace, "expected '{' in match expression");

  std::vector<ast::MatchArm> arms;
  while (!check(TokenKind::RBrace) && !check(TokenKind::Eof)) {
    arms.push_back(parseMatchArm());
    match(TokenKind::Comma);
  }

  expect(TokenKind::RBrace, "expected '}' after match arms");
  return std::make_unique<ast::MatchExpr>(std::move(scrutinee), std::move(arms),
                                          loc);
}

ast::ExprPtr Parser::parseClosureExpr() {
  auto loc = current_.location;
  expect(TokenKind::Pipe, "expected '|' for closure");

  std::vector<ast::ClosureParam> params;
  while (!check(TokenKind::Pipe) && !check(TokenKind::Eof)) {
    ast::ClosureParam param;
    auto nameTok = expect(TokenKind::Identifier, "expected parameter name");
    param.name = std::string(nameTok.text);

    if (match(TokenKind::Colon)) {
      param.type = parseType();
    }
    params.push_back(std::move(param));
    if (!match(TokenKind::Comma))
      break;
  }
  expect(TokenKind::Pipe, "expected '|' after closure parameters");

  ast::TypeNodePtr returnType;
  if (match(TokenKind::Arrow)) {
    returnType = parseType();
  }

  auto body = parseBlockExpr();

  return std::make_unique<ast::ClosureExpr>(
      std::move(params), std::move(returnType), std::move(body), loc);
}

// ============================================================================
// Pattern parsing
// ============================================================================

ast::PatternPtr Parser::parsePattern() {
  auto loc = current_.location;

  // Wildcard: _
  if (match(TokenKind::Underscore)) {
    return std::make_unique<ast::WildcardPattern>(loc);
  }

  // Literal patterns
  if (check(TokenKind::IntLiteral)) {
    auto tok = advance();
    auto lit = std::make_unique<ast::IntLiteralExpr>(tok.intValue, loc);
    return std::make_unique<ast::LiteralPattern>(std::move(lit), loc);
  }

  if (check(TokenKind::StringLiteral)) {
    auto tok = advance();
    std::string value(tok.text.substr(1, tok.text.size() - 2));
    auto lit = std::make_unique<ast::StringLiteralExpr>(std::move(value), loc);
    return std::make_unique<ast::LiteralPattern>(std::move(lit), loc);
  }

  if (check(TokenKind::BoolLiteral)) {
    auto tok = advance();
    auto lit = std::make_unique<ast::BoolLiteralExpr>(tok.intValue != 0, loc);
    return std::make_unique<ast::LiteralPattern>(std::move(lit), loc);
  }

  // Tuple pattern
  if (match(TokenKind::LParen)) {
    std::vector<ast::PatternPtr> elements;
    while (!check(TokenKind::RParen) && !check(TokenKind::Eof)) {
      elements.push_back(parsePattern());
      if (!match(TokenKind::Comma))
        break;
    }
    expect(TokenKind::RParen, "expected ')' after tuple pattern");
    return std::make_unique<ast::TuplePattern>(std::move(elements), loc);
  }

  // Identifier or constructor pattern
  if (check(TokenKind::Identifier)) {
    auto tok = advance();
    std::string name(tok.text);

    // Check for :: (constructor pattern like Option::Some)
    if (check(TokenKind::ColonColon)) {
      std::vector<std::string> path;
      path.push_back(std::move(name));
      while (match(TokenKind::ColonColon)) {
        auto seg = expect(TokenKind::Identifier, "expected identifier");
        path.push_back(std::string(seg.text));
      }

      auto pattern =
          std::make_unique<ast::ConstructorPattern>(std::move(path), loc);

      // Check for tuple fields: Option::Some(value)
      if (match(TokenKind::LParen)) {
        while (!check(TokenKind::RParen) && !check(TokenKind::Eof)) {
          pattern->positionalFields.push_back(parsePattern());
          if (!match(TokenKind::Comma))
            break;
        }
        expect(TokenKind::RParen, "expected ')' after constructor pattern");
      }

      // Check for struct fields: Message::Move { x, y }
      if (match(TokenKind::LBrace)) {
        while (!check(TokenKind::RBrace) && !check(TokenKind::Eof)) {
          auto fieldTok = expect(TokenKind::Identifier, "expected field name");
          ast::ConstructorPattern::NamedField field;
          field.name = std::string(fieldTok.text);

          if (match(TokenKind::Colon)) {
            field.pattern = parsePattern();
          } else {
            // Shorthand: { x } means { x: x }
            field.pattern = std::make_unique<ast::IdentPattern>(
                std::string(fieldTok.text), fieldTok.location);
          }
          pattern->namedFields.push_back(std::move(field));
          if (!match(TokenKind::Comma))
            break;
        }
        expect(TokenKind::RBrace, "expected '}' after struct pattern");
      }

      return pattern;
    }

    // Simple identifier binding
    return std::make_unique<ast::IdentPattern>(std::move(name), loc);
  }

  diag_.emitError(loc, "expected pattern");
  return std::make_unique<ast::WildcardPattern>(loc);
}

// ============================================================================
// Type parsing
// ============================================================================

ast::TypeNodePtr Parser::parseType() {
  auto loc = current_.location;

  // ref Type or &Type
  if (check(TokenKind::KwRef)) {
    return parseRefType();
  }
  if (check(TokenKind::Ampersand)) {
    advance();
    // &mut Type
    if (check(TokenKind::KwMut)) {
      advance();
      auto inner = parseType();
      return std::make_unique<ast::MutRefType>(std::move(inner), loc);
    }
    // &Type
    auto inner = parseType();
    return std::make_unique<ast::ReferenceType>(std::move(inner), loc);
  }

  // mut ref Type
  if (check(TokenKind::KwMut)) {
    advance();
    if (check(TokenKind::KwRef)) {
      advance();
      auto inner = parseType();
      return std::make_unique<ast::MutRefType>(std::move(inner), loc);
    }
    diag_.emitError(loc, "expected 'ref' after 'mut' in type");
    return nullptr;
  }

  // Tuple type: (T1, T2, ...)
  if (check(TokenKind::LParen)) {
    return parseTupleType();
  }

  // Named or generic type
  return parseNamedOrGenericType();
}

ast::TypeNodePtr Parser::parseNamedOrGenericType() {
  auto loc = current_.location;

  // Collect path segments: std::collections::HashMap
  std::vector<std::string> path;

  // Handle Void keyword specially
  if (check(TokenKind::KwVoid)) {
    advance();
    path.push_back("Void");
    return std::make_unique<ast::NamedType>(std::move(path), loc);
  }

  // Handle Self keyword
  if (check(TokenKind::KwSelfType)) {
    advance();
    path.push_back("Self");
    return std::make_unique<ast::NamedType>(std::move(path), loc);
  }

  auto tok = expect(TokenKind::Identifier, "expected type name");
  path.push_back(std::string(tok.text));

  while (match(TokenKind::ColonColon)) {
    tok = expect(TokenKind::Identifier, "expected type name after '::'");
    path.push_back(std::string(tok.text));
  }

  // Check for generic type arguments: Type<T1, T2>
  if (check(TokenKind::Less)) {
    advance();
    std::vector<ast::TypeNodePtr> typeArgs;
    while (!check(TokenKind::Greater) && !check(TokenKind::Eof)) {
      typeArgs.push_back(parseType());
      if (!match(TokenKind::Comma))
        break;
    }
    expect(TokenKind::Greater, "expected '>' after type arguments");

    auto base = std::make_unique<ast::NamedType>(std::move(path), loc);
    return std::make_unique<ast::GenericType>(std::move(base),
                                              std::move(typeArgs), loc);
  }

  return std::make_unique<ast::NamedType>(std::move(path), loc);
}

ast::TypeNodePtr Parser::parseTupleType() {
  auto loc = current_.location;
  expect(TokenKind::LParen, "expected '('");

  std::vector<ast::TypeNodePtr> elements;
  while (!check(TokenKind::RParen) && !check(TokenKind::Eof)) {
    elements.push_back(parseType());
    if (!match(TokenKind::Comma))
      break;
  }
  expect(TokenKind::RParen, "expected ')' after tuple type");

  // Check for function type: (T1, T2) -> RetType
  if (match(TokenKind::Arrow)) {
    auto retType = parseType();
    return std::make_unique<ast::FunctionType>(std::move(elements),
                                               std::move(retType), loc);
  }

  return std::make_unique<ast::TupleType>(std::move(elements), loc);
}

ast::TypeNodePtr Parser::parseRefType() {
  auto loc = current_.location;
  expect(TokenKind::KwRef, "expected 'ref'");

  // Check for lifetime: ref 'a T
  std::optional<std::string> lifetime;
  if (check(TokenKind::Apostrophe)) {
    advance();
    auto lifetimeTok = expect(TokenKind::Identifier, "expected lifetime name");
    lifetime = std::string(lifetimeTok.text);
  }

  auto inner = parseType();
  auto refType = std::make_unique<ast::ReferenceType>(std::move(inner), loc);
  refType->lifetime = std::move(lifetime);
  return refType;
}

// ============================================================================
// State save/restore (for lookahead disambiguation)
// ============================================================================

Parser::ParserState Parser::saveState() const {
  return {current_, previous_, 0, 0, 0}; // lexer state is saved separately
}

void Parser::restoreState(const ParserState &state) {
  current_ = state.current;
  previous_ = state.previous;
}

// ============================================================================
// Error recovery
// ============================================================================

void Parser::synchronize() {
  while (!check(TokenKind::Eof)) {
    if (previous_.kind == TokenKind::Semicolon)
      return;

    switch (current_.kind) {
    case TokenKind::KwFunc:
    case TokenKind::KwLet:
    case TokenKind::KwConst:
    case TokenKind::KwStruct:
    case TokenKind::KwClass:
    case TokenKind::KwEnum:
    case TokenKind::KwTrait:
    case TokenKind::KwImpl:
    case TokenKind::KwReturn:
    case TokenKind::KwIf:
    case TokenKind::KwFor:
    case TokenKind::KwWhile:
    case TokenKind::KwLoop:
    case TokenKind::KwModule:
    case TokenKind::KwImport:
      return;
    default:
      advance();
    }
  }
}

} // namespace flux
