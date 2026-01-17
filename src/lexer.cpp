#include "lexer.h"
#include <iostream>

namespace volt {

Lexer::Lexer(std::string_view source) : source_(source) {}

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;
    
    while (!isAtEnd()) {
        start_ = current_;
        tokens.push_back(scanToken());
    }
    
    tokens.push_back(Token(TokenType::Eof, "", line_));
    return tokens;
}

Token Lexer::scanToken() {
    // Skip whitespace
    while (!isAtEnd() && (peek() == ' ' || peek() == '\t' || peek() == '\n')) {
        if (peek() == '\n') line_++;
        advance();
    }
    
    if (isAtEnd()) return Token(TokenType::Eof, "", line_);
    
    start_ = current_;
    char c = advance();
    
    if (isDigit(c)) return number();
    
    switch (c) {
        case '+': return Token(TokenType::Plus, "+", line_);
        case '-': return Token(TokenType::Minus, "-", line_);
        case '*': return Token(TokenType::Star, "*", line_);
        case '/': return Token(TokenType::Slash, "/", line_);
    }
    
    return Token(TokenType::Error, source_.substr(start_, 1), line_);
}

Token Lexer::number() {
    while (isDigit(peek())) advance();
    
    if (peek() == '.' && isDigit(source_[current_ + 1])) {
        advance(); // '.'
        while (isDigit(peek())) advance();
    }
    
    return Token(TokenType::Number, 
                 source_.substr(start_, current_ - start_), 
                 line_);
}

char Lexer::advance() {
    return source_[current_++];
}

char Lexer::peek() const {
    if (isAtEnd()) return '\0';
    return source_[current_];
}

bool Lexer::isAtEnd() const {
    return current_ >= source_.length();
}

bool Lexer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

} // namespace volt
