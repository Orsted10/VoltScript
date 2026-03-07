// parser_core.cpp — constructor, entry points, token helpers, error handling
#include "parser.h"
#include "features/string_pool.h"
#include "interpreter/value.h"
#include <sstream>

namespace claw {

Parser::Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

std::vector<StmtPtr> Parser::parseProgram() {
    std::vector<StmtPtr> stmts;
    while (!isAtEnd()) {
        try {
            auto s = statement();
            if (s) stmts.push_back(std::move(s));
        } catch (...) {
            synchronize();
        }
    }
    return stmts;
}

ExprPtr Parser::parseExpression() { return expression(); }

// ---- Token helpers ----
Token Parser::advance() {
    if (!isAtEnd()) current_++;
    return previous();
}

Token Parser::peek() const { return tokens_[current_]; }

Token Parser::peekNext() const {
    if (current_ + 1 >= tokens_.size()) return tokens_.back();
    return tokens_[current_ + 1];
}

Token Parser::previous() const { return tokens_[current_ - 1]; }

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::checkNext(TokenType type) const {
    if (current_ + 1 >= tokens_.size()) return false;
    return tokens_[current_ + 1].type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) { advance(); return true; }
    return false;
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (auto t : types) { if (check(t)) { advance(); return true; } }
    return false;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    error(peek(), message);
    return peek();
}

bool Parser::isAtEnd() const { return peek().type == TokenType::Eof; }

void Parser::error(const std::string& message) { error(peek(), message); }

void Parser::error(const Token& tok, const std::string& message) {
    hadError_ = true;
    std::ostringstream oss;
    oss << "[line " << tok.line << ":" << tok.column << "] Error";
    if (tok.type == TokenType::Eof) oss << " at end";
    else oss << " at '" << tok.lexeme << "'";
    oss << ": " << message;
    errors_.push_back(oss.str());
    throw std::runtime_error(oss.str());
}

void Parser::synchronize() {
    advance();
    while (!isAtEnd()) {
        if (previous().type == TokenType::Semicolon) return;
        switch (peek().type) {
            case TokenType::Class: case TokenType::Fn: case TokenType::Let:
            case TokenType::Const: case TokenType::For: case TokenType::If:
            case TokenType::While: case TokenType::Return: case TokenType::Import:
            case TokenType::Export: case TokenType::Enum: case TokenType::Interface:
                return;
            default: break;
        }
        advance();
    }
}

bool Parser::isCompoundAssignOp(TokenType t) const {
    switch (t) {
        case TokenType::PlusEqual: case TokenType::MinusEqual:
        case TokenType::StarEqual: case TokenType::SlashEqual:
        case TokenType::PercentEqual: case TokenType::StarStarEqual:
        case TokenType::BitAndEqual: case TokenType::BitOrEqual:
        case TokenType::BitXorEqual: case TokenType::ShiftLeftEqual:
        case TokenType::ShiftRightEqual: return true;
        default: return false;
    }
}

TokenType Parser::compoundOpToBinary(TokenType t) const {
    switch (t) {
        case TokenType::PlusEqual:       return TokenType::Plus;
        case TokenType::MinusEqual:      return TokenType::Minus;
        case TokenType::StarEqual:       return TokenType::Star;
        case TokenType::SlashEqual:      return TokenType::Slash;
        case TokenType::PercentEqual:    return TokenType::Percent;
        case TokenType::StarStarEqual:   return TokenType::StarStar;
        case TokenType::BitAndEqual:     return TokenType::BitAnd;
        case TokenType::BitOrEqual:      return TokenType::BitOr;
        case TokenType::BitXorEqual:     return TokenType::BitXor;
        case TokenType::ShiftLeftEqual:  return TokenType::ShiftLeft;
        case TokenType::ShiftRightEqual: return TokenType::ShiftRight;
        default: return t;
    }
}

} // namespace claw
