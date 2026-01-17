#pragma once
#include <string>
#include <string_view>

namespace volt {

enum class TokenType {
    // Literals
    Number,
    
    // Operators
    Plus, Minus, Star, Slash,
    
    // Special
    Eof, Error
};

struct Token {
    TokenType type;
    std::string_view lexeme;
    int line;
    
    Token(TokenType t, std::string_view lex, int ln)
        : type(t), lexeme(lex), line(ln) {}
};

const char* tokenName(TokenType type);

} // namespace volt
