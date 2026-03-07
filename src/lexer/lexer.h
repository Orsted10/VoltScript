#pragma once
#include "token.h"
#include <string_view>
#include <string>
#include <vector>
#include <unordered_map>

namespace claw {

class Lexer {
public:
    explicit Lexer(std::string_view source);

    std::vector<Token> tokenize();

private:
    // Core scanners
    Token scanToken();
    Token number();
    Token identifier();
    Token string(char quote);
    Token fstring();
    Token templateLiteral();
    Token lineComment();
    Token blockComment();

    // Whitespace
    void skipWhitespace();

    // Character helpers
    char advance();
    char peek() const;
    char peekNext() const;
    char peekAt(int offset) const;
    bool match(char expected);
    bool match2(char c1, char c2);
    bool isAtEnd() const;
    bool isDigit(char c) const;
    bool isHexDigit(char c) const;
    bool isAlpha(char c) const;
    bool isAlphaNumeric(char c) const;

    // String escape processing
    std::string processEscapes(std::string_view raw) const;
    // F-string: extract interpolation segments
    // Returns a Token of type FString with stringValue = raw source between quotes
    Token scanFString();
    Token scanTemplate();

    std::string_view source_;
    size_t current_ = 0;
    size_t start_   = 0;
    int line_        = 1;
    int column_      = 1;
    int startColumn_ = 1;

    static const std::unordered_map<std::string_view, TokenType> keywords_;
};

} // namespace claw
