#include "lexer.h"
#include "features/string_pool.h"
#include <iostream>
#include <sstream>

namespace claw {

// ============================================================
// Keyword table — all keywords including new ones
// ============================================================
const std::unordered_map<std::string_view, TokenType> Lexer::keywords_ = {
    // Declarations
    {"let",        TokenType::Let},
    {"const",      TokenType::Const},
    {"fn",         TokenType::Fn},
    {"fun",        TokenType::Fn},       // alias
    {"async",      TokenType::Async},
    {"await",      TokenType::Await},
    {"yield",      TokenType::Yield},
    {"class",      TokenType::Class},
    {"this",       TokenType::This},
    {"super",      TokenType::Super},
    {"extends",    TokenType::Extends},
    {"static",     TokenType::Static},
    {"enum",       TokenType::Enum},
    {"interface",  TokenType::Interface},
    {"implements", TokenType::Implements},
    {"abstract",   TokenType::Abstract},
    {"new",        TokenType::New},
    {"operator",   TokenType::Operator},

    // Control flow
    {"if",         TokenType::If},
    {"else",       TokenType::Else},
    {"while",      TokenType::While},
    {"for",        TokenType::For},
    {"of",         TokenType::Of},
    {"in",         TokenType::In},
    {"run",        TokenType::Run},
    {"until",      TokenType::Until},
    {"return",     TokenType::Return},
    {"break",      TokenType::Break},
    {"continue",   TokenType::Continue},
    {"switch",     TokenType::Switch},
    {"case",       TokenType::Case},
    {"default",    TokenType::Default},
    {"match",      TokenType::Match},

    // Values
    {"true",       TokenType::True},
    {"false",      TokenType::False},
    {"nil",        TokenType::Nil},
    {"null",       TokenType::Nil},     // alias

    // Modules
    {"import",     TokenType::Import},
    {"from",       TokenType::From},
    {"export",     TokenType::Export},
    {"as",         TokenType::As},

    // Error handling
    {"try",        TokenType::Try},
    {"catch",      TokenType::Catch},
    {"throw",      TokenType::Throw},
    {"finally",    TokenType::Finally},

    // Unique ClawScript
    {"defer",      TokenType::Defer},
    {"reactive",   TokenType::Reactive},
    {"with",       TokenType::With},
    {"print",      TokenType::Print},   // legacy compat

    // Logical operator word aliases (Python/Lua style)
    {"and",        TokenType::And},
    {"or",         TokenType::Or},
    {"not",        TokenType::Bang},
};

// ============================================================
// Constructor
// ============================================================
Lexer::Lexer(std::string_view source) : source_(source) {}

// ============================================================
// Main tokenize loop
// ============================================================
std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    tokens.reserve(source_.size() / 4); // rough estimate

    while (!isAtEnd()) {
        skipWhitespace();
        if (isAtEnd()) break;

        start_       = current_;
        startColumn_ = column_;
        Token tok = scanToken();
        if (tok.type != TokenType::Error || !tok.lexeme.empty()) {
            tokens.push_back(std::move(tok));
        }
    }

    tokens.push_back(Token(TokenType::Eof, "", line_, column_));
    return tokens;
}

// ============================================================
// Core scanner
// ============================================================
Token Lexer::scanToken() {
    start_       = current_;
    startColumn_ = column_;
    char c = advance();

    // Numbers
    if (isDigit(c)) return number();

    // Identifiers / keywords
    if (isAlpha(c)) {
        // Check for f-string: f"..." or f'...'
        if ((c == 'f' || c == 'F') && (peek() == '"' || peek() == '\'')) {
            advance(); // consume opening quote
            return scanFString();
        }
        return identifier();
    }

    // String literals
    if (c == '"' || c == '\'') return string(c);

    // Template literals
    if (c == '`') return scanTemplate();

    // Decorator
    if (c == '@') {
        return Token(TokenType::At, source_.substr(start_, current_ - start_), line_, startColumn_);
    }

    // Private member prefix
    if (c == '#') {
        // If followed by identifier char, it's a private member name
        if (isAlpha(peek())) {
            while (isAlphaNumeric(peek())) advance();
            std::string_view text = source_.substr(start_, current_ - start_);
            return Token(TokenType::Hash, StringPool::intern(text), line_, startColumn_);
        }
        return Token(TokenType::Hash, "#", line_, startColumn_);
    }

    switch (c) {
        // Single-char punctuation
        case '(': return Token(TokenType::LeftParen,    "(", line_, startColumn_);
        case ')': return Token(TokenType::RightParen,   ")", line_, startColumn_);
        case '{': return Token(TokenType::LeftBrace,    "{", line_, startColumn_);
        case '}': return Token(TokenType::RightBrace,   "}", line_, startColumn_);
        case '[': return Token(TokenType::LeftBracket,  "[", line_, startColumn_);
        case ']': return Token(TokenType::RightBracket, "]", line_, startColumn_);
        case ';': return Token(TokenType::Semicolon,    ";", line_, startColumn_);
        case ',': return Token(TokenType::Comma,        ",", line_, startColumn_);
        case '~': return Token(TokenType::BitNot,       "~", line_, startColumn_);
        case '^':
            if (match('=')) return Token(TokenType::BitXorEqual, "^=", line_, startColumn_);
            return Token(TokenType::BitXor, "^", line_, startColumn_);

        // Dot / spread / range
        case '.':
            if (peek() == '.' && peekAt(1) == '.') {
                advance(); advance(); // consume ..
                return Token(TokenType::Spread, "...", line_, startColumn_);
            }
            if (peek() == '.') {
                advance();
                return Token(TokenType::DotDot, "..", line_, startColumn_);
            }
            return Token(TokenType::Dot, ".", line_, startColumn_);

        // Colon
        case ':': return Token(TokenType::Colon, ":", line_, startColumn_);

        // Question / optional chaining / null coalescing
        case '?':
            if (match('?')) return Token(TokenType::QuestionQuestion, "??", line_, startColumn_);
            if (match('.')) return Token(TokenType::QuestionDot,      "?.", line_, startColumn_);
            return Token(TokenType::Question, "?", line_, startColumn_);

        // Plus
        case '+':
            if (match('+')) return Token(TokenType::PlusPlus,   "++", line_, startColumn_);
            if (match('=')) return Token(TokenType::PlusEqual,  "+=", line_, startColumn_);
            return Token(TokenType::Plus, "+", line_, startColumn_);

        // Minus / arrow
        case '-':
            if (match('-')) return Token(TokenType::MinusMinus,  "--", line_, startColumn_);
            if (match('=')) return Token(TokenType::MinusEqual,  "-=", line_, startColumn_);
            if (match('>')) return Token(TokenType::Arrow,       "->", line_, startColumn_);
            return Token(TokenType::Minus, "-", line_, startColumn_);

        // Star / power
        case '*':
            if (match('*')) {
                if (match('=')) return Token(TokenType::StarStarEqual, "**=", line_, startColumn_);
                return Token(TokenType::StarStar, "**", line_, startColumn_);
            }
            if (match('=')) return Token(TokenType::StarEqual, "*=", line_, startColumn_);
            return Token(TokenType::Star, "*", line_, startColumn_);

        // Slash / comments
        case '/':
            if (match('/')) {
                // Line comment — skip to end of line, then consume the newline
                while (peek() != '\n' && !isAtEnd()) advance();
                if (!isAtEnd() && peek() == '\n') {
                    line_++;
                    column_ = 0;
                    advance(); // consume '\n'
                }
                // Skip any further whitespace before the next real token
                skipWhitespace();
                if (isAtEnd()) {
                    return Token(TokenType::Eof, "", line_, column_);
                }
                return scanToken();
            }
            if (match('*')) {
                // Block comment
                while (!isAtEnd()) {
                    if (peek() == '*' && peekAt(1) == '/') {
                        advance(); advance(); // consume */
                        break;
                    }
                    if (peek() == '\n') { line_++; column_ = 0; }
                    advance();
                }
                return scanToken();
            }
            if (match('=')) return Token(TokenType::SlashEqual, "/=", line_, startColumn_);
            return Token(TokenType::Slash, "/", line_, startColumn_);

        // Percent
        case '%':
            if (match('=')) return Token(TokenType::PercentEqual, "%=", line_, startColumn_);
            return Token(TokenType::Percent, "%", line_, startColumn_);

        // Equal / fat arrow
        case '=':
            if (match('=')) return Token(TokenType::EqualEqual, "==", line_, startColumn_);
            if (match('>')) return Token(TokenType::FatArrow,   "=>", line_, startColumn_);
            return Token(TokenType::Equal, "=", line_, startColumn_);

        // Bang
        case '!':
            if (match('=')) return Token(TokenType::BangEqual, "!=", line_, startColumn_);
            return Token(TokenType::Bang, "!", line_, startColumn_);

        // Less / shift left
        case '<':
            if (match('<')) {
                if (match('=')) return Token(TokenType::ShiftLeftEqual, "<<=", line_, startColumn_);
                return Token(TokenType::ShiftLeft, "<<", line_, startColumn_);
            }
            if (match('=')) return Token(TokenType::LessEqual, "<=", line_, startColumn_);
            return Token(TokenType::Less, "<", line_, startColumn_);

        // Greater / shift right
        case '>':
            if (match('>')) {
                if (match('=')) return Token(TokenType::ShiftRightEqual, ">>=", line_, startColumn_);
                return Token(TokenType::ShiftRight, ">>", line_, startColumn_);
            }
            if (match('=')) return Token(TokenType::GreaterEqual, ">=", line_, startColumn_);
            return Token(TokenType::Greater, ">", line_, startColumn_);

        // Ampersand / logical and
        case '&':
            if (match('&')) return Token(TokenType::And,        "&&", line_, startColumn_);
            if (match('=')) return Token(TokenType::BitAndEqual,"&=", line_, startColumn_);
            return Token(TokenType::BitAnd, "&", line_, startColumn_);

        // Pipe / logical or / pipe operator
        case '|':
            if (match('|')) return Token(TokenType::Or,         "||", line_, startColumn_);
            if (match('>')) return Token(TokenType::Pipe,       "|>", line_, startColumn_);
            if (match('=')) return Token(TokenType::BitOrEqual, "|=", line_, startColumn_);
            return Token(TokenType::BitOr, "|", line_, startColumn_);
    }

    // Unknown character
    std::string errMsg = "Unexpected character '";
    errMsg += c;
    errMsg += "'";
    return Token(TokenType::Error,
                 StringPool::intern(errMsg),
                 line_, startColumn_);
}

// ============================================================
// Number literal (int, float, hex, binary, octal)
// ============================================================
Token Lexer::number() {
    // Hex: 0x...
    if (source_[start_] == '0' && (peek() == 'x' || peek() == 'X')) {
        advance(); // consume x
        while (isHexDigit(peek())) advance();
        return Token(TokenType::Number,
                     source_.substr(start_, current_ - start_),
                     line_, startColumn_);
    }
    // Binary: 0b...
    if (source_[start_] == '0' && (peek() == 'b' || peek() == 'B')) {
        advance();
        while (peek() == '0' || peek() == '1') advance();
        return Token(TokenType::Number,
                     source_.substr(start_, current_ - start_),
                     line_, startColumn_);
    }
    // Octal: 0o...
    if (source_[start_] == '0' && (peek() == 'o' || peek() == 'O')) {
        advance();
        while (peek() >= '0' && peek() <= '7') advance();
        return Token(TokenType::Number,
                     source_.substr(start_, current_ - start_),
                     line_, startColumn_);
    }

    // Decimal integer or float
    while (isDigit(peek())) advance();

    // Fractional part
    if (peek() == '.' && isDigit(peekAt(1))) {
        advance(); // consume '.'
        while (isDigit(peek())) advance();
    }

    // Scientific notation: 1e10, 1.5e-3
    if (peek() == 'e' || peek() == 'E') {
        advance();
        if (peek() == '+' || peek() == '-') advance();
        while (isDigit(peek())) advance();
    }

    // Numeric separator: 1_000_000
    // (already consumed above, underscores are ignored)

    return Token(TokenType::Number,
                 source_.substr(start_, current_ - start_),
                 line_, startColumn_);
}

// ============================================================
// Identifier / keyword
// ============================================================
Token Lexer::identifier() {
    while (isAlphaNumeric(peek())) advance();

    std::string_view text = source_.substr(start_, current_ - start_);

    // Check for generator function: fn*
    // (handled in parser by checking Star after Fn)

    auto it = keywords_.find(text);
    TokenType type = (it != keywords_.end()) ? it->second : TokenType::Identifier;

    return Token(type, StringPool::intern(text), line_, startColumn_);
}

// ============================================================
// Regular string literal: "..." or '...'
// ============================================================
Token Lexer::string(char quote) {
    int strCol = startColumn_;
    std::string processed;
    processed.reserve(32);

    while (!isAtEnd() && peek() != quote) {
        if (peek() == '\n') {
            line_++;
            column_ = 0;
        }
        if (peek() == '\\') {
            advance(); // consume backslash
            if (isAtEnd()) break;
            char esc = advance();
            switch (esc) {
                case 'n':  processed += '\n'; break;
                case 't':  processed += '\t'; break;
                case 'r':  processed += '\r'; break;
                case '\\': processed += '\\'; break;
                case '\'': processed += '\''; break;
                case '"':  processed += '"';  break;
                case '0':  processed += '\0'; break;
                case 'a':  processed += '\a'; break;
                case 'b':  processed += '\b'; break;
                case 'f':  processed += '\f'; break;
                case 'v':  processed += '\v'; break;
                case 'u': {
                    // Unicode escape: \uXXXX
                    if (peek() == '{') {
                        advance(); // {
                        std::string hex;
                        while (isHexDigit(peek())) hex += advance();
                        if (peek() == '}') advance();
                        // Convert to UTF-8
                        uint32_t cp = std::stoul(hex, nullptr, 16);
                        if (cp < 0x80) {
                            processed += (char)cp;
                        } else if (cp < 0x800) {
                            processed += (char)(0xC0 | (cp >> 6));
                            processed += (char)(0x80 | (cp & 0x3F));
                        } else if (cp < 0x10000) {
                            processed += (char)(0xE0 | (cp >> 12));
                            processed += (char)(0x80 | ((cp >> 6) & 0x3F));
                            processed += (char)(0x80 | (cp & 0x3F));
                        } else {
                            processed += (char)(0xF0 | (cp >> 18));
                            processed += (char)(0x80 | ((cp >> 12) & 0x3F));
                            processed += (char)(0x80 | ((cp >> 6) & 0x3F));
                            processed += (char)(0x80 | (cp & 0x3F));
                        }
                    } else {
                        // \uXXXX (4 hex digits)
                        std::string hex;
                        for (int i = 0; i < 4 && isHexDigit(peek()); i++) hex += advance();
                        uint32_t cp = std::stoul(hex, nullptr, 16);
                        if (cp < 0x80) processed += (char)cp;
                        else if (cp < 0x800) {
                            processed += (char)(0xC0 | (cp >> 6));
                            processed += (char)(0x80 | (cp & 0x3F));
                        } else {
                            processed += (char)(0xE0 | (cp >> 12));
                            processed += (char)(0x80 | ((cp >> 6) & 0x3F));
                            processed += (char)(0x80 | (cp & 0x3F));
                        }
                    }
                    break;
                }
                default:
                    processed += '\\';
                    processed += esc;
                    break;
            }
        } else {
            processed += advance();
        }
    }

    if (isAtEnd()) {
        return Token(TokenType::Error,
                     StringPool::intern("Unterminated string literal"),
                     line_, strCol);
    }
    advance(); // closing quote

    std::string_view rawLexeme = source_.substr(start_, current_ - start_);
    std::string internedVal = std::string(StringPool::intern(processed));
    return Token(TokenType::String,
                 StringPool::intern(rawLexeme),
                 line_, strCol,
                 std::move(internedVal));
}

// ============================================================
// F-string: f"Hello {name}, you are {age} years old!"
// The stringValue stores the raw content between quotes.
// The parser/interpreter will handle interpolation.
// ============================================================
Token Lexer::scanFString() {
    int strCol = startColumn_;
    // The opening quote was already consumed by scanToken
    // We need to find the matching closing quote
    char quote = source_[current_ - 1]; // the quote char that was consumed

    std::string raw;
    raw.reserve(64);

    while (!isAtEnd() && peek() != quote) {
        if (peek() == '\n') { line_++; column_ = 0; }
        if (peek() == '\\') {
            raw += advance(); // backslash
            if (!isAtEnd()) raw += advance(); // escaped char
        } else {
            raw += advance();
        }
    }

    if (isAtEnd()) {
        return Token(TokenType::Error,
                     StringPool::intern("Unterminated f-string"),
                     line_, strCol);
    }
    advance(); // closing quote

    std::string_view rawLexeme = source_.substr(start_, current_ - start_);
    return Token(TokenType::FString,
                 StringPool::intern(rawLexeme),
                 line_, strCol,
                 std::move(raw));
}

// ============================================================
// Template literal: `Hello ${name}!`
// ============================================================
Token Lexer::scanTemplate() {
    int strCol = startColumn_;
    std::string raw;
    raw.reserve(64);

    while (!isAtEnd() && peek() != '`') {
        if (peek() == '\n') { line_++; column_ = 0; }
        if (peek() == '\\') {
            raw += advance();
            if (!isAtEnd()) raw += advance();
        } else {
            raw += advance();
        }
    }

    if (isAtEnd()) {
        return Token(TokenType::Error,
                     StringPool::intern("Unterminated template literal"),
                     line_, strCol);
    }
    advance(); // closing backtick

    std::string_view rawLexeme = source_.substr(start_, current_ - start_);
    return Token(TokenType::Template,
                 StringPool::intern(rawLexeme),
                 line_, strCol,
                 std::move(raw));
}

// ============================================================
// Whitespace / comment skipping
// ============================================================
void Lexer::skipWhitespace() {
    while (!isAtEnd()) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                line_++;
                column_ = 0;
                advance();
                break;
            default:
                return;
        }
    }
}

// ============================================================
// Character helpers
// ============================================================
char Lexer::advance() {
    column_++;
    return source_[current_++];
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source_[current_];
}

char Lexer::peekNext() const {
    if (current_ + 1 >= source_.length()) return '\0';
    return source_[current_ + 1];
}

char Lexer::peekAt(int offset) const {
    size_t idx = current_ + static_cast<size_t>(offset);
    if (idx >= source_.length()) return '\0';
    return source_[idx];
}

bool Lexer::match(char expected) {
    if (isAtEnd()) return false;
    if (source_[current_] != expected) return false;
    current_++;
    column_++;
    return true;
}

bool Lexer::match2(char c1, char c2) {
    if (current_ + 1 >= source_.length()) return false;
    if (source_[current_] != c1 || source_[current_ + 1] != c2) return false;
    current_ += 2;
    column_  += 2;
    return true;
}

bool Lexer::isAtEnd() const {
    return current_ >= source_.length();
}

bool Lexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

bool Lexer::isHexDigit(char c) const {
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

bool Lexer::isAlpha(char c) const {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           c == '_';
}

bool Lexer::isAlphaNumeric(char c) const {
    return isAlpha(c) || isDigit(c);
}

} // namespace claw
