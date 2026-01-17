#pragma once
#include "token.h"
#include <string_view>
#include <vector>

namespace volt {

class Lexer {
public:
    explicit Lexer(std::string_view source);
    
    std::vector<Token> tokenize();
    
private:
    Token scanToken();
    Token number();
    
    char advance();
    char peek() const;
    bool isAtEnd() const;
    bool isDigit(char c) const;
    
    std::string_view source_;
    size_t current_ = 0;
    size_t start_ = 0;
    int line_ = 1;
};

} // namespace volt
