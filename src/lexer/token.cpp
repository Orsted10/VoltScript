#include "token.h"

namespace claw {

const char* tokenName(TokenType type) {
    switch (type) {
        // Literals
        case TokenType::Number:           return "Number";
        case TokenType::String:           return "String";
        case TokenType::Identifier:       return "Identifier";
        case TokenType::FString:          return "FString";
        case TokenType::Template:         return "Template";

        // Declarations
        case TokenType::Let:              return "let";
        case TokenType::Const:            return "const";
        case TokenType::Fn:               return "fn";
        case TokenType::Async:            return "async";
        case TokenType::Await:            return "await";
        case TokenType::Yield:            return "yield";
        case TokenType::Class:            return "class";
        case TokenType::This:             return "this";
        case TokenType::Super:            return "super";
        case TokenType::Extends:          return "extends";
        case TokenType::Static:           return "static";
        case TokenType::Enum:             return "enum";
        case TokenType::Interface:        return "interface";
        case TokenType::Implements:       return "implements";
        case TokenType::Abstract:         return "abstract";
        case TokenType::New:              return "new";
        case TokenType::Operator:         return "operator";

        // Control flow
        case TokenType::If:               return "if";
        case TokenType::Else:             return "else";
        case TokenType::While:            return "while";
        case TokenType::For:              return "for";
        case TokenType::Of:               return "of";
        case TokenType::In:               return "in";
        case TokenType::Run:              return "run";
        case TokenType::Until:            return "until";
        case TokenType::Return:           return "return";
        case TokenType::Break:            return "break";
        case TokenType::Continue:         return "continue";
        case TokenType::Switch:           return "switch";
        case TokenType::Case:             return "case";
        case TokenType::Default:          return "default";
        case TokenType::Match:            return "match";

        // Values
        case TokenType::True:             return "true";
        case TokenType::False:            return "false";
        case TokenType::Nil:              return "nil";

        // Modules
        case TokenType::Import:           return "import";
        case TokenType::From:             return "from";
        case TokenType::Export:           return "export";
        case TokenType::As:               return "as";

        // Error handling
        case TokenType::Try:              return "try";
        case TokenType::Catch:            return "catch";
        case TokenType::Throw:            return "throw";
        case TokenType::Finally:          return "finally";

        // Unique ClawScript
        case TokenType::Defer:            return "defer";
        case TokenType::Reactive:         return "reactive";
        case TokenType::With:             return "with";
        case TokenType::Print:            return "print";

        // Arithmetic
        case TokenType::Plus:             return "+";
        case TokenType::Minus:            return "-";
        case TokenType::Star:             return "*";
        case TokenType::Slash:            return "/";
        case TokenType::Percent:          return "%";
        case TokenType::StarStar:         return "**";

        // Comparison
        case TokenType::Equal:            return "=";
        case TokenType::EqualEqual:       return "==";
        case TokenType::Bang:             return "!";
        case TokenType::BangEqual:        return "!=";
        case TokenType::Less:             return "<";
        case TokenType::LessEqual:        return "<=";
        case TokenType::Greater:          return ">";
        case TokenType::GreaterEqual:     return ">=";

        // Logical
        case TokenType::And:              return "&&";
        case TokenType::Or:               return "||";

        // Compound assignment
        case TokenType::PlusEqual:        return "+=";
        case TokenType::MinusEqual:       return "-=";
        case TokenType::StarEqual:        return "*=";
        case TokenType::SlashEqual:       return "/=";
        case TokenType::PercentEqual:     return "%=";
        case TokenType::StarStarEqual:    return "**=";

        // Increment/Decrement
        case TokenType::PlusPlus:         return "++";
        case TokenType::MinusMinus:       return "--";

        // Bitwise
        case TokenType::BitAnd:           return "&";
        case TokenType::BitOr:            return "|";
        case TokenType::BitXor:           return "^";
        case TokenType::BitNot:           return "~";
        case TokenType::ShiftLeft:        return "<<";
        case TokenType::ShiftRight:       return ">>";
        case TokenType::BitAndEqual:      return "&=";
        case TokenType::BitOrEqual:       return "|=";
        case TokenType::BitXorEqual:      return "^=";
        case TokenType::ShiftLeftEqual:   return "<<=";
        case TokenType::ShiftRightEqual:  return ">>=";

        // Special operators
        case TokenType::Question:         return "?";
        case TokenType::Colon:            return ":";
        case TokenType::QuestionQuestion: return "??";
        case TokenType::QuestionDot:      return "?.";
        case TokenType::Arrow:            return "->";
        case TokenType::FatArrow:         return "=>";
        case TokenType::Pipe:             return "|>";
        case TokenType::Spread:           return "...";
        case TokenType::Hash:             return "#";
        case TokenType::At:               return "@";

        // Punctuation
        case TokenType::LeftParen:        return "(";
        case TokenType::RightParen:       return ")";
        case TokenType::LeftBrace:        return "{";
        case TokenType::RightBrace:       return "}";
        case TokenType::LeftBracket:      return "[";
        case TokenType::RightBracket:     return "]";
        case TokenType::Semicolon:        return ";";
        case TokenType::Comma:            return ",";
        case TokenType::Dot:              return ".";
        case TokenType::DotDot:           return "..";
        case TokenType::DotDotDot:        return "...";
        case TokenType::Backtick:         return "`";

        // Special
        case TokenType::Eof:              return "EOF";
        case TokenType::Error:            return "Error";
        case TokenType::Newline:          return "Newline";

        default:                          return "Unknown";
    }
}

} // namespace claw
