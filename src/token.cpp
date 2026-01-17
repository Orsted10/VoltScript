#include "token.h"

namespace volt {

const char* tokenName(TokenType type) {
    switch (type) {
        case TokenType::Number: return "NUMBER";
        case TokenType::Plus: return "PLUS";
        case TokenType::Minus: return "MINUS";
        case TokenType::Star: return "STAR";
        case TokenType::Slash: return "SLASH";
        case TokenType::Eof: return "EOF";
        case TokenType::Error: return "ERROR";
        default: return "UNKNOWN";
    }
}

} // namespace volt
