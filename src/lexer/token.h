#pragma once
#include <string_view>
#include <string>

namespace claw {

enum class TokenType {
    // Literals
    Number, String, Identifier,
    FString,        // f"Hello {name}!"
    Template,       // `Hello ${name}!`

    // Keywords — Control Flow
    If, Else, While, For, Run, Until,
    Return, Break, Continue,
    Switch, Case, Default,
    Match,          // match expr { case ... }

    // Keywords — Declarations
    Let, Const,     // const = immutable
    Fn,             // fn name() {}
    Async,          // async fn
    Yield,          // yield value
    Class, This, Super, Extends,
    Static,         // static member
    Enum,           // enum Direction { ... }
    Interface,      // interface Drawable { ... }
    Implements,     // class Foo implements Bar
    Abstract,       // abstract class

    // Keywords — Values
    True, False, Nil,
    New,            // new ClassName()

    // Keywords — Modules
    Import, From, Export,
    As,             // import X as Y / with X as Y

    // Keywords — Error Handling
    Try, Catch, Throw, Finally,

    // Keywords — Unique ClawScript
    Defer,          // defer { cleanup(); }
    Reactive,       // reactive let x = 0
    Operator,       // operator+(other) {}
    With,           // with open(f) as h { }
    Of,             // for (let x of arr)
    In,             // for (let k in map)
    Await,          // await promise
    Print,          // print (legacy, keep for compat)

    // Operators — Arithmetic
    Plus, Minus, Star, Slash, Percent,
    StarStar,       // ** (power)

    // Operators — Comparison
    Equal, EqualEqual, Bang, BangEqual,
    Less, LessEqual, Greater, GreaterEqual,

    // Operators — Logical
    And, Or,

    // Operators — Compound Assignment
    PlusEqual, MinusEqual, StarEqual, SlashEqual, PercentEqual,
    StarStarEqual,  // **=

    // Operators — Increment/Decrement
    PlusPlus, MinusMinus,

    // Operators — Bitwise
    BitAnd, BitOr, BitXor, BitNot,
    ShiftLeft, ShiftRight,
    BitAndEqual, BitOrEqual, BitXorEqual,
    ShiftLeftEqual, ShiftRightEqual,

    // Operators — Special
    Question,       // ?
    Colon,          // :
    QuestionQuestion,   // ?? (null coalescing)
    QuestionDot,        // ?. (optional chaining)
    Arrow,              // -> (return type / lambda)
    FatArrow,           // => (lambda shorthand)
    Pipe,               // |> (pipe operator)
    Spread,             // ... (spread/rest)
    Hash,               // # (private member)
    At,                 // @ (decorator)

    // Punctuation
    LeftParen, RightParen,
    LeftBrace, RightBrace,
    LeftBracket, RightBracket,
    Semicolon, Comma,
    Dot,
    DotDot,         // .. (range)
    DotDotDot,      // ... (spread — alias for Spread)
    Backtick,       // ` (template literal start/end)

    // Special
    Eof, Error,
    Newline,        // significant newline (for ASI)
};

struct Token {
    TokenType type;
    std::string_view lexeme;
    int line;
    int column;
    std::string stringValue; // For processed string literals (with escape sequences)

    Token(TokenType t, std::string_view lex, int ln, int col = 1)
        : type(t), lexeme(lex), line(ln), column(col) {}

    // Constructor for string tokens with processed value
    Token(TokenType t, std::string_view lex, int ln, int col, std::string strVal)
        : type(t), lexeme(lex), line(ln), column(col), stringValue(std::move(strVal)) {}
};

const char* tokenName(TokenType type);

} // namespace claw
