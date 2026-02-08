#include "flux/Lexer/Lexer.h"

#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace flux {

// ============================================================================
// Token kind to string mapping
// ============================================================================

const char* Token::kindToString(TokenKind kind) {
    switch (kind) {
        case TokenKind::Eof:            return "EOF";
        case TokenKind::Invalid:        return "INVALID";
        case TokenKind::Newline:        return "NEWLINE";
        case TokenKind::IntLiteral:     return "INT_LITERAL";
        case TokenKind::FloatLiteral:   return "FLOAT_LITERAL";
        case TokenKind::StringLiteral:  return "STRING_LITERAL";
        case TokenKind::CharLiteral:    return "CHAR_LITERAL";
        case TokenKind::BoolLiteral:    return "BOOL_LITERAL";
        case TokenKind::Identifier:     return "IDENTIFIER";
        case TokenKind::KwModule:       return "module";
        case TokenKind::KwImport:       return "import";
        case TokenKind::KwFunc:         return "func";
        case TokenKind::KwLet:          return "let";
        case TokenKind::KwMut:          return "mut";
        case TokenKind::KwConst:        return "const";
        case TokenKind::KwStruct:       return "struct";
        case TokenKind::KwClass:        return "class";
        case TokenKind::KwEnum:         return "enum";
        case TokenKind::KwTrait:        return "trait";
        case TokenKind::KwImpl:         return "impl";
        case TokenKind::KwType:         return "type";
        case TokenKind::KwSelf:         return "self";
        case TokenKind::KwSelfType:     return "Self";
        case TokenKind::KwIf:           return "if";
        case TokenKind::KwElse:         return "else";
        case TokenKind::KwMatch:        return "match";
        case TokenKind::KwFor:          return "for";
        case TokenKind::KwWhile:        return "while";
        case TokenKind::KwLoop:         return "loop";
        case TokenKind::KwBreak:        return "break";
        case TokenKind::KwContinue:     return "continue";
        case TokenKind::KwReturn:       return "return";
        case TokenKind::KwIn:           return "in";
        case TokenKind::KwMove:         return "move";
        case TokenKind::KwRef:          return "ref";
        case TokenKind::KwDrop:         return "drop";
        case TokenKind::KwAsync:        return "async";
        case TokenKind::KwAwait:        return "await";
        case TokenKind::KwSpawn:        return "spawn";
        case TokenKind::KwUnsafe:       return "unsafe";
        case TokenKind::KwPub:          return "pub";
        case TokenKind::KwPublic:       return "public";
        case TokenKind::KwPrivate:      return "private";
        case TokenKind::KwTrue:         return "true";
        case TokenKind::KwFalse:        return "false";
        case TokenKind::KwAnd:          return "and";
        case TokenKind::KwOr:           return "or";
        case TokenKind::KwNot:          return "not";
        case TokenKind::KwAs:           return "as";
        case TokenKind::KwIs:           return "is";
        case TokenKind::KwWhere:        return "where";
        case TokenKind::KwUse:          return "use";
        case TokenKind::KwVoid:         return "Void";
        case TokenKind::KwPanic:        return "panic";
        case TokenKind::KwAssert:       return "assert";
        case TokenKind::KwDoc:          return "@doc";
        case TokenKind::KwDeprecated:   return "@deprecated";
        case TokenKind::KwTest:         return "@test";
        case TokenKind::LParen:         return "(";
        case TokenKind::RParen:         return ")";
        case TokenKind::LBracket:       return "[";
        case TokenKind::RBracket:       return "]";
        case TokenKind::LBrace:         return "{";
        case TokenKind::RBrace:         return "}";
        case TokenKind::Comma:          return ",";
        case TokenKind::Semicolon:      return ";";
        case TokenKind::Colon:          return ":";
        case TokenKind::ColonColon:     return "::";
        case TokenKind::Dot:            return ".";
        case TokenKind::DotDot:         return "..";
        case TokenKind::DotDotDot:      return "...";
        case TokenKind::Arrow:          return "->";
        case TokenKind::FatArrow:       return "=>";
        case TokenKind::At:             return "@";
        case TokenKind::Hash:           return "#";
        case TokenKind::HashBang:       return "#!";
        case TokenKind::Plus:           return "+";
        case TokenKind::Minus:          return "-";
        case TokenKind::Star:           return "*";
        case TokenKind::Slash:          return "/";
        case TokenKind::Percent:        return "%";
        case TokenKind::Equal:          return "=";
        case TokenKind::EqualEqual:     return "==";
        case TokenKind::BangEqual:      return "!=";
        case TokenKind::Less:           return "<";
        case TokenKind::LessEqual:      return "<=";
        case TokenKind::Greater:        return ">";
        case TokenKind::GreaterEqual:   return ">=";
        case TokenKind::Ampersand:      return "&";
        case TokenKind::Pipe:           return "|";
        case TokenKind::Caret:          return "^";
        case TokenKind::Tilde:          return "~";
        case TokenKind::ShiftLeft:      return "<<";
        case TokenKind::ShiftRight:     return ">>";
        case TokenKind::PlusEqual:      return "+=";
        case TokenKind::MinusEqual:     return "-=";
        case TokenKind::StarEqual:      return "*=";
        case TokenKind::SlashEqual:     return "/=";
        case TokenKind::PercentEqual:   return "%=";
        case TokenKind::AmpersandEqual: return "&=";
        case TokenKind::PipeEqual:      return "|=";
        case TokenKind::CaretEqual:     return "^=";
        case TokenKind::Question:       return "?";
        case TokenKind::Underscore:     return "_";
        case TokenKind::Apostrophe:     return "'";
    }
    return "UNKNOWN";
}

// ============================================================================
// Keyword lookup table
// ============================================================================

static const std::unordered_map<std::string_view, TokenKind> keywords = {
    {"module",   TokenKind::KwModule},
    {"import",   TokenKind::KwImport},
    {"func",     TokenKind::KwFunc},
    {"let",      TokenKind::KwLet},
    {"mut",      TokenKind::KwMut},
    {"const",    TokenKind::KwConst},
    {"struct",   TokenKind::KwStruct},
    {"class",    TokenKind::KwClass},
    {"enum",     TokenKind::KwEnum},
    {"trait",    TokenKind::KwTrait},
    {"impl",     TokenKind::KwImpl},
    {"type",     TokenKind::KwType},
    {"self",     TokenKind::KwSelf},
    {"Self",     TokenKind::KwSelfType},
    {"if",       TokenKind::KwIf},
    {"else",     TokenKind::KwElse},
    {"match",    TokenKind::KwMatch},
    {"for",      TokenKind::KwFor},
    {"while",    TokenKind::KwWhile},
    {"loop",     TokenKind::KwLoop},
    {"break",    TokenKind::KwBreak},
    {"continue", TokenKind::KwContinue},
    {"return",   TokenKind::KwReturn},
    {"in",       TokenKind::KwIn},
    {"move",     TokenKind::KwMove},
    {"ref",      TokenKind::KwRef},
    {"drop",     TokenKind::KwDrop},
    {"async",    TokenKind::KwAsync},
    {"await",    TokenKind::KwAwait},
    {"spawn",    TokenKind::KwSpawn},
    {"unsafe",   TokenKind::KwUnsafe},
    {"pub",      TokenKind::KwPub},
    {"public",   TokenKind::KwPublic},
    {"private",  TokenKind::KwPrivate},
    {"true",     TokenKind::KwTrue},
    {"false",    TokenKind::KwFalse},
    {"and",      TokenKind::KwAnd},
    {"or",       TokenKind::KwOr},
    {"not",      TokenKind::KwNot},
    {"as",       TokenKind::KwAs},
    {"is",       TokenKind::KwIs},
    {"where",    TokenKind::KwWhere},
    {"use",      TokenKind::KwUse},
    {"Void",     TokenKind::KwVoid},
    {"panic",    TokenKind::KwPanic},
    {"assert",   TokenKind::KwAssert},
};

// ============================================================================
// Lexer implementation
// ============================================================================

Lexer::Lexer(std::string_view source, std::string_view filename,
             DiagnosticEngine& diag)
    : source_(source), filename_(filename), diag_(diag) {}

Lexer::State Lexer::saveState() const {
    return {current_, tokenStart_, line_, column_,
            tokenLine_, tokenColumn_, hasPeeked_, peekedToken_};
}

void Lexer::restoreState(const State& s) {
    current_ = s.current;
    tokenStart_ = s.tokenStart;
    line_ = s.line;
    column_ = s.column;
    tokenLine_ = s.tokenLine;
    tokenColumn_ = s.tokenColumn;
    hasPeeked_ = s.hasPeeked;
    peekedToken_ = s.peekedToken;
}

bool Lexer::isAtEnd() const {
    return current_ >= source_.size();
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source_[current_];
}

char Lexer::peekNext() const {
    if (current_ + 1 >= source_.size()) return '\0';
    return source_[current_ + 1];
}

char Lexer::advance() {
    char c = source_[current_++];
    if (c == '\n') {
        ++line_;
        column_ = 1;
    } else {
        ++column_;
    }
    return c;
}

bool Lexer::match(char expected) {
    if (isAtEnd() || source_[current_] != expected) return false;
    advance();
    return true;
}

void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                advance();
                break;
            case '/':
                if (peekNext() == '/') {
                    skipLineComment();
                } else if (peekNext() == '*') {
                    if (!skipBlockComment()) {
                        return; // Error already emitted
                    }
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

void Lexer::skipLineComment() {
    // Skip the //
    advance();
    advance();
    while (!isAtEnd() && peek() != '\n') {
        advance();
    }
}

bool Lexer::skipBlockComment() {
    // Skip the /*
    advance();
    advance();
    int depth = 1;

    while (!isAtEnd() && depth > 0) {
        if (peek() == '/' && peekNext() == '*') {
            advance();
            advance();
            ++depth;
        } else if (peek() == '*' && peekNext() == '/') {
            advance();
            advance();
            --depth;
        } else {
            advance();
        }
    }

    if (depth > 0) {
        SourceLocation loc;
        loc.filename = filename_;
        loc.line = line_;
        loc.column = column_;
        diag_.emitError(loc, "unterminated block comment");
        return false;
    }
    return true;
}

Token Lexer::makeToken(TokenKind kind) const {
    Token token;
    token.kind = kind;
    token.text = source_.substr(tokenStart_, current_ - tokenStart_);
    token.location.filename = filename_;
    token.location.line = tokenLine_;
    token.location.column = tokenColumn_;
    token.location.offset = tokenStart_;
    token.intValue = 0;
    return token;
}

Token Lexer::makeToken(TokenKind kind, std::string_view text) const {
    Token token;
    token.kind = kind;
    token.text = text;
    token.location.filename = filename_;
    token.location.line = tokenLine_;
    token.location.column = tokenColumn_;
    token.location.offset = tokenStart_;
    token.intValue = 0;
    return token;
}

Token Lexer::errorToken(const std::string& message) {
    SourceLocation loc;
    loc.filename = filename_;
    loc.line = tokenLine_;
    loc.column = tokenColumn_;
    loc.offset = tokenStart_;
    diag_.emitError(loc, message);

    Token token;
    token.kind = TokenKind::Invalid;
    token.text = source_.substr(tokenStart_, current_ - tokenStart_);
    token.location = loc;
    token.intValue = 0;
    return token;
}

TokenKind Lexer::identifierKind(std::string_view text) const {
    auto it = keywords.find(text);
    if (it != keywords.end()) {
        return it->second;
    }
    return TokenKind::Identifier;
}

Token Lexer::lexIdentifierOrKeyword() {
    while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) {
        advance();
    }

    std::string_view text = source_.substr(tokenStart_, current_ - tokenStart_);
    TokenKind kind = identifierKind(text);

    return makeToken(kind, text);
}

Token Lexer::lexNumber() {
    bool isFloat = false;

    // Check for hex, octal, binary prefixes
    if (peek() == '0' && !isAtEnd()) {
        char next = peekNext();
        if (next == 'x' || next == 'X') {
            // Hexadecimal
            advance(); // 0
            advance(); // x
            if (!isAtEnd() && !std::isxdigit(peek())) {
                return errorToken("expected hexadecimal digit after '0x'");
            }
            while (!isAtEnd() && (std::isxdigit(peek()) || peek() == '_')) {
                advance();
            }
            Token token = makeToken(TokenKind::IntLiteral);
            // Parse hex value (skip 0x prefix and underscores)
            std::string clean;
            for (size_t i = 2; i < token.text.size(); ++i) {
                if (token.text[i] != '_') clean += token.text[i];
            }
            token.intValue = std::stoll(clean, nullptr, 16);
            return token;
        } else if (next == 'b' || next == 'B') {
            // Binary
            advance(); // 0
            advance(); // b
            if (!isAtEnd() && peek() != '0' && peek() != '1') {
                return errorToken("expected binary digit after '0b'");
            }
            while (!isAtEnd() && (peek() == '0' || peek() == '1' || peek() == '_')) {
                advance();
            }
            Token token = makeToken(TokenKind::IntLiteral);
            std::string clean;
            for (size_t i = 2; i < token.text.size(); ++i) {
                if (token.text[i] != '_') clean += token.text[i];
            }
            token.intValue = std::stoll(clean, nullptr, 2);
            return token;
        } else if (next == 'o' || next == 'O') {
            // Octal
            advance(); // 0
            advance(); // o
            while (!isAtEnd() && ((peek() >= '0' && peek() <= '7') || peek() == '_')) {
                advance();
            }
            Token token = makeToken(TokenKind::IntLiteral);
            std::string clean;
            for (size_t i = 2; i < token.text.size(); ++i) {
                if (token.text[i] != '_') clean += token.text[i];
            }
            token.intValue = std::stoll(clean, nullptr, 8);
            return token;
        }
    }

    // Decimal integer or float
    while (!isAtEnd() && (std::isdigit(peek()) || peek() == '_')) {
        advance();
    }

    // Check for decimal point (but not ..)
    if (peek() == '.' && peekNext() != '.') {
        isFloat = true;
        advance(); // consume '.'
        while (!isAtEnd() && (std::isdigit(peek()) || peek() == '_')) {
            advance();
        }
    }

    // Check for exponent
    if (peek() == 'e' || peek() == 'E') {
        isFloat = true;
        advance();
        if (peek() == '+' || peek() == '-') {
            advance();
        }
        if (!std::isdigit(peek())) {
            return errorToken("expected digit in exponent");
        }
        while (!isAtEnd() && (std::isdigit(peek()) || peek() == '_')) {
            advance();
        }
    }

    if (isFloat) {
        Token token = makeToken(TokenKind::FloatLiteral);
        std::string clean;
        for (char c : token.text) {
            if (c != '_') clean += c;
        }
        token.floatValue = std::stod(clean);
        return token;
    } else {
        Token token = makeToken(TokenKind::IntLiteral);
        std::string clean;
        for (char c : token.text) {
            if (c != '_') clean += c;
        }
        token.intValue = std::stoll(clean);
        return token;
    }
}

Token Lexer::lexString() {
    // Skip opening quote
    advance();
    uint32_t contentStart = current_;

    while (!isAtEnd() && peek() != '"') {
        if (peek() == '\\') {
            advance(); // skip backslash
            if (isAtEnd()) {
                return errorToken("unterminated string literal");
            }
            // Skip escaped character
            advance();
        } else if (peek() == '\n') {
            return errorToken("unterminated string literal (newline in string)");
        } else {
            advance();
        }
    }

    if (isAtEnd()) {
        return errorToken("unterminated string literal");
    }

    uint32_t contentEnd = current_;
    advance(); // closing quote

    // Store text as content without quotes
    auto token = makeToken(TokenKind::StringLiteral);
    token.text = source_.substr(contentStart, contentEnd - contentStart);
    return token;
}

Token Lexer::lexChar() {
    // Skip opening quote
    advance();

    if (isAtEnd()) {
        return errorToken("unterminated character literal");
    }

    if (peek() == '\\') {
        advance(); // backslash
        if (isAtEnd()) {
            return errorToken("unterminated character literal");
        }
        advance(); // escaped char
    } else {
        advance(); // the character
    }

    if (isAtEnd() || peek() != '\'') {
        return errorToken("unterminated character literal (expected closing ')");
    }

    advance(); // closing quote
    return makeToken(TokenKind::CharLiteral);
}

Token Lexer::lexAnnotation() {
    advance(); // skip @

    // Read the annotation name
    uint32_t nameStart = current_;
    while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) {
        advance();
    }

    std::string_view name = source_.substr(nameStart, current_ - nameStart);
    if (name == "doc") return makeToken(TokenKind::KwDoc);
    if (name == "deprecated") return makeToken(TokenKind::KwDeprecated);
    if (name == "test") return makeToken(TokenKind::KwTest);

    // Unknown annotation — treat as @ followed by identifier
    // Reset to just after @
    current_ = nameStart;
    line_ = tokenLine_;
    column_ = tokenColumn_ + 1;
    return makeToken(TokenKind::At);
}

Token Lexer::nextToken() {
    // Return peeked token if available
    if (hasPeeked_) {
        hasPeeked_ = false;
        return peekedToken_;
    }

    skipWhitespace();

    tokenStart_ = current_;
    tokenLine_ = line_;
    tokenColumn_ = column_;

    if (isAtEnd()) {
        return makeToken(TokenKind::Eof);
    }

    char c = advance();

    // Identifiers and keywords
    if (std::isalpha(c) || c == '_') {
        // Check for single underscore as wildcard
        if (c == '_' && (isAtEnd() || (!std::isalnum(peek()) && peek() != '_'))) {
            return makeToken(TokenKind::Underscore);
        }
        return lexIdentifierOrKeyword();
    }

    // Numbers
    if (std::isdigit(c)) {
        // We already advanced one char, back up
        --current_;
        if (c == '\n') {
            --line_;
        } else {
            --column_;
        }
        return lexNumber();
    }

    // Strings
    if (c == '"') {
        --current_;
        --column_;
        return lexString();
    }

    // Characters or lifetime
    if (c == '\'') {
        // Check if this is a lifetime ('a, 'b, etc.) or char literal ('x')
        // Lifetime: ' followed by alpha then not '
        // Char: ' followed by char then '
        if (!isAtEnd() && std::isalpha(peek())) {
            // Could be lifetime or char — check further
            uint32_t savedCurrent = current_;
            uint32_t savedLine = line_;
            uint32_t savedColumn = column_;

            char first = advance();
            if (!isAtEnd() && peek() == '\'') {
                // It's a char literal like 'a'
                current_ = savedCurrent;
                line_ = savedLine;
                column_ = savedColumn;
                --current_; --column_; // back up past the first '
                return lexChar();
            } else {
                // It's a lifetime 'a
                // Back to after the letter
                // Token text is 'a (the apostrophe + identifier)
                while (!isAtEnd() && (std::isalnum(peek()) || peek() == '_')) {
                    advance();
                }
                return makeToken(TokenKind::Apostrophe);
            }
        }

        if (!isAtEnd() && peek() == '\\') {
            // Escaped char literal
            --current_; --column_;
            return lexChar();
        }

        return makeToken(TokenKind::Apostrophe);
    }

    // Annotations
    if (c == '@') {
        if (!isAtEnd() && std::isalpha(peek())) {
            --current_; --column_;
            return lexAnnotation();
        }
        return makeToken(TokenKind::At);
    }

    // Operators and punctuation
    switch (c) {
        case '(': return makeToken(TokenKind::LParen);
        case ')': return makeToken(TokenKind::RParen);
        case '[': return makeToken(TokenKind::LBracket);
        case ']': return makeToken(TokenKind::RBracket);
        case '{': return makeToken(TokenKind::LBrace);
        case '}': return makeToken(TokenKind::RBrace);
        case ',': return makeToken(TokenKind::Comma);
        case ';': return makeToken(TokenKind::Semicolon);
        case '~': return makeToken(TokenKind::Tilde);
        case '?': return makeToken(TokenKind::Question);

        case ':':
            if (match(':')) return makeToken(TokenKind::ColonColon);
            return makeToken(TokenKind::Colon);

        case '.':
            if (match('.')) {
                if (match('.')) return makeToken(TokenKind::DotDotDot);
                return makeToken(TokenKind::DotDot);
            }
            return makeToken(TokenKind::Dot);

        case '+':
            if (match('=')) return makeToken(TokenKind::PlusEqual);
            return makeToken(TokenKind::Plus);

        case '-':
            if (match('>')) return makeToken(TokenKind::Arrow);
            if (match('=')) return makeToken(TokenKind::MinusEqual);
            return makeToken(TokenKind::Minus);

        case '*':
            if (match('=')) return makeToken(TokenKind::StarEqual);
            return makeToken(TokenKind::Star);

        case '/':
            if (match('=')) return makeToken(TokenKind::SlashEqual);
            return makeToken(TokenKind::Slash);

        case '%':
            if (match('=')) return makeToken(TokenKind::PercentEqual);
            return makeToken(TokenKind::Percent);

        case '=':
            if (match('=')) return makeToken(TokenKind::EqualEqual);
            if (match('>')) return makeToken(TokenKind::FatArrow);
            return makeToken(TokenKind::Equal);

        case '!':
            if (match('=')) return makeToken(TokenKind::BangEqual);
            return errorToken("unexpected character '!'");

        case '<':
            if (match('=')) return makeToken(TokenKind::LessEqual);
            if (match('<')) return makeToken(TokenKind::ShiftLeft);
            return makeToken(TokenKind::Less);

        case '>':
            if (match('=')) return makeToken(TokenKind::GreaterEqual);
            if (match('>')) return makeToken(TokenKind::ShiftRight);
            return makeToken(TokenKind::Greater);

        case '&':
            if (match('=')) return makeToken(TokenKind::AmpersandEqual);
            return makeToken(TokenKind::Ampersand);

        case '|':
            if (match('=')) return makeToken(TokenKind::PipeEqual);
            return makeToken(TokenKind::Pipe);

        case '^':
            if (match('=')) return makeToken(TokenKind::CaretEqual);
            return makeToken(TokenKind::Caret);

        case '#':
            if (match('!')) return makeToken(TokenKind::HashBang);
            return makeToken(TokenKind::Hash);

        default:
            break;
    }

    return errorToken(std::string("unexpected character '") + c + "'");
}

Token Lexer::peekToken() {
    if (!hasPeeked_) {
        peekedToken_ = nextToken();
        hasPeeked_ = true;
    }
    return peekedToken_;
}

std::vector<Token> Lexer::lexAll() {
    std::vector<Token> tokens;
    while (true) {
        Token token = nextToken();
        tokens.push_back(token);
        if (token.kind == TokenKind::Eof) break;
    }
    return tokens;
}

} // namespace flux
