// parser_exprs.cpp — all expression parsing methods
#include "parser.h"
#include "features/string_pool.h"
#include "interpreter/value.h"
#include "lexer/lexer.h"
#include <sstream>

namespace claw {

// ============================================================
// Expression precedence chain (lowest → highest):
//   expression → assignment → ternary → nullCoalesce →
//   logicalOr → logicalAnd → bitwiseOr → bitwiseXor →
//   bitwiseAnd → equality → comparison → shift →
//   term → factor → power → unary → postfix → call → primary
// ============================================================

ExprPtr Parser::expression() { return assignment(); }

ExprPtr Parser::assignment() {
    ExprPtr left = ternary();

    // Pipe operator: expr |> fn  (right-associative chain)
    if (check(TokenType::Pipe)) {
        while (match(TokenType::Pipe)) {
            Token tok = previous();
            ExprPtr right = ternary();
            left = std::make_unique<PipeExpr>(tok, std::move(left), std::move(right));
        }
        return left;
    }

    // Simple assignment: name = expr
    if (match(TokenType::Equal)) {
        Token eq = previous();
        ExprPtr val = assignment();
        if (auto* var = dynamic_cast<VariableExpr*>(left.get())) {
            return std::make_unique<AssignExpr>(
                Token(TokenType::Identifier, StringPool::intern(var->name), eq.line, eq.column),
                std::move(val));
        }
        if (auto* mem = dynamic_cast<MemberExpr*>(left.get())) {
            return std::make_unique<SetExpr>(eq, std::move(mem->object), mem->member, std::move(val));
        }
        if (auto* idx = dynamic_cast<IndexExpr*>(left.get())) {
            return std::make_unique<IndexAssignExpr>(eq, std::move(idx->object), std::move(idx->index), std::move(val));
        }
        error(eq, "Invalid assignment target");
    }

    // Compound assignment: name op= expr
    if (isCompoundAssignOp(peek().type)) {
        Token opTok = advance();
        ExprPtr val = assignment();
        TokenType binOp = compoundOpToBinary(opTok.type);
        Token binTok(binOp, opTok.lexeme, opTok.line, opTok.column);

        if (auto* var = dynamic_cast<VariableExpr*>(left.get())) {
            Token nameTok(TokenType::Identifier, StringPool::intern(var->name), opTok.line, opTok.column);
            return std::make_unique<CompoundAssignExpr>(nameTok, binTok, std::move(val));
        }
        if (auto* mem = dynamic_cast<MemberExpr*>(left.get())) {
            return std::make_unique<CompoundMemberAssignExpr>(opTok, std::move(mem->object), mem->member, binTok, std::move(val));
        }
        if (auto* idx = dynamic_cast<IndexExpr*>(left.get())) {
            return std::make_unique<CompoundIndexAssignExpr>(opTok, std::move(idx->object), std::move(idx->index), binTok, std::move(val));
        }
        error(opTok, "Invalid compound assignment target");
    }

    return left;
}

ExprPtr Parser::ternary() {
    ExprPtr cond = nullCoalesce();
    if (match(TokenType::Question)) {
        Token tok = previous();
        ExprPtr then = expression();
        consume(TokenType::Colon, "Expected ':' in ternary");
        ExprPtr els = ternary();
        return std::make_unique<TernaryExpr>(tok, std::move(cond), std::move(then), std::move(els));
    }
    return cond;
}

ExprPtr Parser::nullCoalesce() {
    ExprPtr left = logicalOr();
    while (match(TokenType::QuestionQuestion)) {
        Token tok = previous();
        ExprPtr right = logicalOr();
        left = std::make_unique<NullCoalesceExpr>(tok, std::move(left), std::move(right));
    }
    return left;
}

ExprPtr Parser::logicalOr() {
    ExprPtr left = logicalAnd();
    while (match(TokenType::Or)) {
        Token op = previous();
        ExprPtr right = logicalAnd();
        left = std::make_unique<LogicalExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

ExprPtr Parser::logicalAnd() {
    ExprPtr left = bitwiseOr();
    while (match(TokenType::And)) {
        Token op = previous();
        ExprPtr right = bitwiseOr();
        left = std::make_unique<LogicalExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

ExprPtr Parser::bitwiseOr() {
    ExprPtr left = bitwiseXor();
    while (check(TokenType::BitOr)) {
        Token op = advance();
        ExprPtr right = bitwiseXor();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

ExprPtr Parser::bitwiseXor() {
    ExprPtr left = bitwiseAnd();
    while (match(TokenType::BitXor)) {
        Token op = previous();
        ExprPtr right = bitwiseAnd();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

ExprPtr Parser::bitwiseAnd() {
    ExprPtr left = equality();
    while (check(TokenType::BitAnd)) {
        Token op = advance();
        ExprPtr right = equality();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

ExprPtr Parser::equality() {
    ExprPtr left = comparison();
    while (match({TokenType::EqualEqual, TokenType::BangEqual})) {
        Token op = previous();
        ExprPtr right = comparison();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

ExprPtr Parser::comparison() {
    ExprPtr left = shift();
    while (match({TokenType::Less, TokenType::LessEqual, TokenType::Greater, TokenType::GreaterEqual})) {
        Token op = previous();
        ExprPtr right = shift();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

ExprPtr Parser::shift() {
    ExprPtr left = term();
    while (match({TokenType::ShiftLeft, TokenType::ShiftRight})) {
        Token op = previous();
        ExprPtr right = term();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

ExprPtr Parser::term() {
    ExprPtr left = factor();
    while (match({TokenType::Plus, TokenType::Minus})) {
        Token op = previous();
        ExprPtr right = factor();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

ExprPtr Parser::factor() {
    ExprPtr left = power();
    while (match({TokenType::Star, TokenType::Slash, TokenType::Percent})) {
        Token op = previous();
        ExprPtr right = power();
        left = std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

ExprPtr Parser::power() {
    ExprPtr left = unary();
    if (match(TokenType::StarStar)) {
        Token op = previous();
        ExprPtr right = power(); // right-associative
        return std::make_unique<BinaryExpr>(std::move(left), op, std::move(right));
    }
    return left;
}

ExprPtr Parser::unary() {
    // Prefix increment/decrement
    if (match({TokenType::PlusPlus, TokenType::MinusMinus})) {
        Token op = previous();
        ExprPtr operand = unary();
        if (auto* var = dynamic_cast<VariableExpr*>(operand.get())) {
            return std::make_unique<UpdateExpr>(op, var->name, true);
        }
        if (auto* mem = dynamic_cast<MemberExpr*>(operand.get())) {
            return std::make_unique<UpdateMemberExpr>(op, std::move(mem->object), mem->member, true);
        }
        if (auto* idx = dynamic_cast<IndexExpr*>(operand.get())) {
            return std::make_unique<UpdateIndexExpr>(op, std::move(idx->object), std::move(idx->index), true);
        }
        error(op, "Invalid increment/decrement target");
    }
    if (match({TokenType::Bang, TokenType::Minus, TokenType::BitNot})) {
        Token op = previous();
        ExprPtr right = unary();
        return std::make_unique<UnaryExpr>(op, std::move(right));
    }
    // await expr
    if (match(TokenType::Await)) {
        Token tok = previous();
        return std::make_unique<AwaitExpr>(tok, unary());
    }
    // yield [expr]
    if (match(TokenType::Yield)) {
        Token tok = previous();
        ExprPtr val;
        if (!check(TokenType::Semicolon) && !check(TokenType::RightParen) && !isAtEnd())
            val = expression();
        return std::make_unique<YieldExpr>(tok, std::move(val));
    }
    // Spread: ...expr
    if (match(TokenType::Spread)) {
        Token tok = previous();
        return std::make_unique<SpreadExpr>(tok, unary());
    }
    return postfix();
}

ExprPtr Parser::postfix() {
    ExprPtr expr = call();
    // Postfix increment/decrement
    while (match({TokenType::PlusPlus, TokenType::MinusMinus})) {
        Token op = previous();
        if (auto* var = dynamic_cast<VariableExpr*>(expr.get())) {
            expr = std::make_unique<UpdateExpr>(op, var->name, false);
        } else if (auto* mem = dynamic_cast<MemberExpr*>(expr.get())) {
            expr = std::make_unique<UpdateMemberExpr>(op, std::move(mem->object), mem->member, false);
        } else if (auto* idx = dynamic_cast<IndexExpr*>(expr.get())) {
            expr = std::make_unique<UpdateIndexExpr>(op, std::move(idx->object), std::move(idx->index), false);
        } else {
            error(op, "Invalid postfix target");
        }
    }
    return expr;
}

ExprPtr Parser::call() {
    ExprPtr expr = primary();
    while (true) {
        if (check(TokenType::LeftParen)) {
            expr = finishCall(std::move(expr));
        } else if (check(TokenType::Dot) || check(TokenType::LeftBracket)) {
            expr = finishIndexOrMember(std::move(expr));
        } else if (check(TokenType::QuestionDot)) {
            expr = finishOptionalChain(std::move(expr));
        } else {
            break;
        }
    }
    return expr;
}

ExprPtr Parser::finishCall(ExprPtr callee) {
    Token paren = advance(); // consume '('
    std::vector<ExprPtr> args;
    while (!check(TokenType::RightParen) && !isAtEnd()) {
        args.push_back(expression());
        if (!check(TokenType::RightParen)) consume(TokenType::Comma, "Expected ','");
    }
    consume(TokenType::RightParen, "Expected ')' after arguments");
    return std::make_unique<CallExpr>(paren, std::move(callee), std::move(args));
}

ExprPtr Parser::finishIndexOrMember(ExprPtr object) {
    if (match(TokenType::Dot)) {
        Token name = consume(TokenType::Identifier, "Expected property name after '.'");
        return std::make_unique<MemberExpr>(name, std::move(object), std::string(name.lexeme));
    }
    if (match(TokenType::LeftBracket)) {
        Token bracket = previous();
        ExprPtr index = expression();
        consume(TokenType::RightBracket, "Expected ']'");
        return std::make_unique<IndexExpr>(bracket, std::move(object), std::move(index));
    }
    return object;
}

ExprPtr Parser::finishOptionalChain(ExprPtr object) {
    Token tok = advance(); // consume '?.'
    if (check(TokenType::LeftBracket)) {
        advance();
        ExprPtr index = expression();
        consume(TokenType::RightBracket, "Expected ']'");
        return std::make_unique<OptionalChainExpr>(tok, std::move(object), std::move(index));
    }
    Token name = consume(TokenType::Identifier, "Expected property name after '?.'");
    return std::make_unique<OptionalChainExpr>(tok, std::move(object), std::string(name.lexeme));
}

ExprPtr Parser::primary() {
    // Literals
    if (match(TokenType::True))  return std::make_unique<LiteralExpr>(previous(), boolValue(true));
    if (match(TokenType::False)) return std::make_unique<LiteralExpr>(previous(), boolValue(false));
    if (match(TokenType::Nil))   return std::make_unique<LiteralExpr>(previous(), nilValue());

    if (match(TokenType::Number)) {
        Token tok = previous();
        std::string s(tok.lexeme);
        double val = 0.0;
        try {
            if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
                val = (double)std::stoull(s, nullptr, 16);
            else if (s.size() > 2 && s[0] == '0' && (s[1] == 'b' || s[1] == 'B'))
                val = (double)std::stoull(s.substr(2), nullptr, 2);
            else if (s.size() > 2 && s[0] == '0' && (s[1] == 'o' || s[1] == 'O'))
                val = (double)std::stoull(s.substr(2), nullptr, 8);
            else
                val = std::stod(s);
        } catch (...) { val = 0.0; }
        return std::make_unique<LiteralExpr>(tok, numberToValue(val));
    }

    if (match(TokenType::String)) {
        Token tok = previous();
        const char* interned = StringPool::intern(tok.stringValue).data();
        return std::make_unique<LiteralExpr>(tok, stringValue(interned));
    }

    // F-string: f"Hello {name}!"
    if (match(TokenType::FString)) {
        Token tok = previous();
        return fstringExpression(tok);
    }

    // Template literal: `Hello ${name}!`
    if (match(TokenType::Template)) {
        Token tok = previous();
        return templateExpression(tok);
    }

    // Grouping / tuple
    if (match(TokenType::LeftParen)) {
        Token tok = previous();
        if (check(TokenType::RightParen)) {
            advance();
            // Empty parens — unit value
            return std::make_unique<LiteralExpr>(tok, nilValue());
        }
        ExprPtr expr = expression();
        consume(TokenType::RightParen, "Expected ')'");
        return std::make_unique<GroupingExpr>(tok, std::move(expr));
    }

    // Array literal / comprehension
    if (match(TokenType::LeftBracket)) {
        return arrayLiteral();
    }

    // Hash map literal
    if (match(TokenType::LeftBrace)) {
        return hashMapOrBlock();
    }

    // Anonymous function: fn(params) { body } or fn*(params) { body }
    if (match(TokenType::Fn)) {
        bool isGen = false;
        if (match(TokenType::Star)) isGen = true;
        return functionExpression(false, isGen);
    }

    // Async anonymous function: async fn(params) { body }
    if (check(TokenType::Async) && checkNext(TokenType::Fn)) {
        advance(); advance();
        return functionExpression(true, false);
    }

    // Arrow function: (params) => expr  or  param => expr
    // (handled in assignment via fat arrow detection — see below)

    // new ClassName(args)
    if (match(TokenType::New)) {
        Token tok = previous();
        ExprPtr callee = primary();
        consume(TokenType::LeftParen, "Expected '('");
        std::vector<ExprPtr> args;
        while (!check(TokenType::RightParen) && !isAtEnd()) {
            args.push_back(expression());
            if (!check(TokenType::RightParen)) consume(TokenType::Comma, "Expected ','");
        }
        consume(TokenType::RightParen, "Expected ')'");
        return std::make_unique<NewExpr>(tok, std::move(callee), std::move(args));
    }

    // this
    if (match(TokenType::This)) return std::make_unique<ThisExpr>(previous());

    // super.method
    if (match(TokenType::Super)) {
        Token tok = previous();
        consume(TokenType::Dot, "Expected '.' after 'super'");
        Token method = consume(TokenType::Identifier, "Expected superclass method name");
        return std::make_unique<SuperExpr>(tok, std::string(method.lexeme));
    }

    // match expression
    if (match(TokenType::Match)) {
        return matchExpression();
    }

    // Identifier
    if (match(TokenType::Identifier)) {
        Token tok = previous();
        return std::make_unique<VariableExpr>(tok);
    }

    error("Expected expression");
    return nullptr;
}

// ============================================================
// Array literal: [elem, elem, ...] or comprehension [expr for x in iter]
// ============================================================
ExprPtr Parser::arrayLiteral() {
    Token bracket = previous();
    std::vector<ExprPtr> elements;

    if (check(TokenType::RightBracket)) {
        advance();
        return std::make_unique<ArrayExpr>(bracket, std::move(elements));
    }

    ExprPtr first = expression();

    // Comprehension: [expr for x in iter if cond]
    if (check(TokenType::For)) {
        advance(); // consume 'for'
        std::string varName = std::string(consume(TokenType::Identifier, "Expected variable name").lexeme);
        consume(TokenType::In, "Expected 'in'");
        ExprPtr iter = expression();
        ExprPtr cond;
        if (check(TokenType::If)) { advance(); cond = expression(); }
        consume(TokenType::RightBracket, "Expected ']'");
        return std::make_unique<ComprehensionExpr>(bracket, std::move(first), varName, std::move(iter), std::move(cond));
    }

    elements.push_back(std::move(first));
    while (match(TokenType::Comma)) {
        if (check(TokenType::RightBracket)) break;
        elements.push_back(expression());
    }
    consume(TokenType::RightBracket, "Expected ']'");
    return std::make_unique<ArrayExpr>(bracket, std::move(elements));
}

// ============================================================
// Hash map literal: { key: val, key: val }
// ============================================================
ExprPtr Parser::hashMapOrBlock() {
    Token brace = previous();
    std::vector<std::pair<ExprPtr, ExprPtr>> pairs;

    if (check(TokenType::RightBrace)) {
        advance();
        return std::make_unique<HashMapExpr>(brace, std::move(pairs));
    }

    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        ExprPtr key;
        // Computed key: [expr]: val
        if (match(TokenType::LeftBracket)) {
            key = expression();
            consume(TokenType::RightBracket, "Expected ']'");
        } else if (check(TokenType::String)) {
            Token tok = advance();
            const char* interned = StringPool::intern(tok.stringValue).data();
            key = std::make_unique<LiteralExpr>(tok, stringValue(interned));
        } else if (check(TokenType::Identifier)) {
            Token tok = advance();
            key = std::make_unique<LiteralExpr>(tok, stringValue(StringPool::intern(std::string(tok.lexeme)).data()));
        } else {
            key = expression();
        }
        consume(TokenType::Colon, "Expected ':' after map key");
        ExprPtr val = expression();
        pairs.push_back({std::move(key), std::move(val)});
        if (!check(TokenType::RightBrace)) consume(TokenType::Comma, "Expected ','");
    }
    consume(TokenType::RightBrace, "Expected '}'");
    return std::make_unique<HashMapExpr>(brace, std::move(pairs));
}

// ============================================================
// Function expression: fn(params) { body }
// ============================================================
ExprPtr Parser::functionExpression(bool isAsync, bool isGenerator) {
    Token tok = previous();
    consume(TokenType::LeftParen, "Expected '('");
    auto params = parseFnParams();
    consume(TokenType::RightParen, "Expected ')'");

    std::string retType;
    if (match(TokenType::Arrow)) retType = std::string(consume(TokenType::Identifier, "Expected return type").lexeme);

    // Arrow function shorthand: => expr
    if (match(TokenType::FatArrow)) {
        Token arrowTok = previous();
        ExprPtr body = expression();
        // Wrap in return statement
        std::vector<ExprPtr> retVals;
        retVals.push_back(std::move(body));
        std::vector<StmtPtr> bodyStmts;
        bodyStmts.push_back(std::make_unique<ReturnStmt>(arrowTok, std::move(retVals)));
        std::vector<std::string> paramNames;
        bool hasRest = false;
        for (auto& p : params) {
            paramNames.push_back(p.name);
            if (p.isRest) hasRest = true;
        }
        auto fe = std::make_unique<FunctionExpr>(tok, std::move(paramNames), std::move(bodyStmts), isGenerator, isAsync);
        fe->hasRest = hasRest;
        return fe;
    }

    consume(TokenType::LeftBrace, "Expected '{'");
    std::vector<StmtPtr> body;
    while (!check(TokenType::RightBrace) && !isAtEnd()) body.push_back(statement());
    consume(TokenType::RightBrace, "Expected '}'");

    std::vector<std::string> paramNames;
    bool hasRest = false;
    for (auto& p : params) {
        paramNames.push_back(p.name);
        if (p.isRest) hasRest = true;
    }
    auto fe = std::make_unique<FunctionExpr>(tok, std::move(paramNames), std::move(body), isGenerator, isAsync);
    fe->hasRest = hasRest;
    return fe;
}

// ============================================================
// F-string interpolation parser
// f"Hello {name}, you are {age} years old!"
// ============================================================
ExprPtr Parser::fstringExpression(Token tok) {
    // tok.stringValue contains the raw content between quotes
    const std::string& raw = tok.stringValue;
    std::vector<FStringExpr::Segment> segments;

    size_t i = 0;
    std::string text;
    while (i < raw.size()) {
        if (raw[i] == '{' && i + 1 < raw.size() && raw[i+1] != '{') {
            // Flush text segment
            if (!text.empty()) {
                FStringExpr::Segment s; s.isExpr = false; s.text = text;
                segments.push_back(std::move(s)); text.clear();
            }
            // Find matching }
            size_t depth = 1; size_t j = i + 1;
            while (j < raw.size() && depth > 0) {
                if (raw[j] == '{') depth++;
                else if (raw[j] == '}') depth--;
                if (depth > 0) j++;
            }
            std::string exprSrc = raw.substr(i + 1, j - i - 1);
            // Parse the inner expression
            Lexer innerLex(exprSrc);
            auto innerToks = innerLex.tokenize();
            Parser innerParser(std::move(innerToks));
            ExprPtr innerExpr = innerParser.parseExpression();
            FStringExpr::Segment s; s.isExpr = true; s.expr = std::move(innerExpr);
            segments.push_back(std::move(s));
            i = j + 1;
        } else if (raw[i] == '{' && i + 1 < raw.size() && raw[i+1] == '{') {
            text += '{'; i += 2;
        } else if (raw[i] == '}' && i + 1 < raw.size() && raw[i+1] == '}') {
            text += '}'; i += 2;
        } else {
            text += raw[i++];
        }
    }
    if (!text.empty()) {
        FStringExpr::Segment s; s.isExpr = false; s.text = text;
        segments.push_back(std::move(s));
    }
    return std::make_unique<FStringExpr>(tok, std::move(segments));
}

// ============================================================
// Template literal: `Hello ${name}!`
// ============================================================
ExprPtr Parser::templateExpression(Token tok) {
    const std::string& raw = tok.stringValue;
    std::vector<TemplateExpr::Segment> segments;

    size_t i = 0;
    std::string text;
    while (i < raw.size()) {
        if (raw[i] == '$' && i + 1 < raw.size() && raw[i+1] == '{') {
            if (!text.empty()) {
                TemplateExpr::Segment s; s.isExpr = false; s.text = text;
                segments.push_back(std::move(s)); text.clear();
            }
            size_t depth = 1; size_t j = i + 2;
            while (j < raw.size() && depth > 0) {
                if (raw[j] == '{') depth++;
                else if (raw[j] == '}') depth--;
                if (depth > 0) j++;
            }
            std::string exprSrc = raw.substr(i + 2, j - i - 2);
            Lexer innerLex(exprSrc);
            auto innerToks = innerLex.tokenize();
            Parser innerParser(std::move(innerToks));
            ExprPtr innerExpr = innerParser.parseExpression();
            TemplateExpr::Segment s; s.isExpr = true; s.expr = std::move(innerExpr);
            segments.push_back(std::move(s));
            i = j + 1;
        } else {
            text += raw[i++];
        }
    }
    if (!text.empty()) {
        TemplateExpr::Segment s; s.isExpr = false; s.text = text;
        segments.push_back(std::move(s));
    }
    return std::make_unique<TemplateExpr>(tok, std::move(segments));
}

// ============================================================
// Match expression (used as expression, not statement)
// ============================================================
ExprPtr Parser::matchExpression() {
    Token tok = previous();
    consume(TokenType::LeftParen, "Expected '('");
    ExprPtr subject = expression();
    consume(TokenType::RightParen, "Expected ')'");
    consume(TokenType::LeftBrace, "Expected '{'");
    std::vector<MatchArm> arms;
    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        MatchArm arm;
        if (match(TokenType::Default)) {
            arm.isDefault = true;
        } else {
            consume(TokenType::Case, "Expected 'case'");
            arm.patterns.push_back(expression());
            while (match(TokenType::BitOr)) arm.patterns.push_back(expression());
            if (check(TokenType::Identifier) && peek().lexeme == "if") {
                advance(); arm.guard = expression();
            }
        }
        consume(TokenType::Colon, "Expected ':'");
        arm.bodyExpr = expression();
        arms.push_back(std::move(arm));
        if (!check(TokenType::RightBrace)) consume(TokenType::Comma, "Expected ','");
    }
    consume(TokenType::RightBrace, "Expected '}'");
    return std::make_unique<MatchExpr>(tok, std::move(subject), std::move(arms));
}

} // namespace claw
