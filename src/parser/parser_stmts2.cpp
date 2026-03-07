// parser_stmts2.cpp — remaining statement parsers (enum, interface, defer, with, decorator, labeled)
#include "parser.h"
#include "features/string_pool.h"
#include "interpreter/value.h"

namespace claw {

StmtPtr Parser::enumStatement() {
    Token tok = previous();
    Token nameTok = consume(TokenType::Identifier, "Expected enum name");
    consume(TokenType::LeftBrace, "Expected '{'");
    std::vector<EnumStmt::EnumMember> members;
    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        EnumStmt::EnumMember m;
        m.name = std::string(consume(TokenType::Identifier, "Expected member name").lexeme);
        if (match(TokenType::Equal)) m.value = expression();
        members.push_back(std::move(m));
        if (!check(TokenType::RightBrace)) consume(TokenType::Comma, "Expected ','");
    }
    consume(TokenType::RightBrace, "Expected '}'");
    consume(TokenType::Semicolon, "Expected ';'");
    return std::make_unique<EnumStmt>(nameTok, std::move(members));
}

StmtPtr Parser::interfaceStatement() {
    Token tok = previous();
    Token nameTok = consume(TokenType::Identifier, "Expected interface name");
    std::vector<std::string> ext;
    if (match(TokenType::Extends)) {
        do { ext.push_back(std::string(consume(TokenType::Identifier, "Expected interface name").lexeme)); }
        while (match(TokenType::Comma));
    }
    consume(TokenType::LeftBrace, "Expected '{'");
    std::vector<InterfaceStmt::Method> methods;
    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        InterfaceStmt::Method m;
        m.name = std::string(consume(TokenType::Identifier, "Expected method name").lexeme);
        if (match(TokenType::Question)) m.isOptional = true;
        consume(TokenType::LeftParen, "Expected '('");
        m.params = parseFnParams();
        consume(TokenType::RightParen, "Expected ')'");
        if (match(TokenType::Arrow)) m.returnType = std::string(consume(TokenType::Identifier, "Expected return type").lexeme);
        consume(TokenType::Semicolon, "Expected ';'");
        methods.push_back(std::move(m));
    }
    consume(TokenType::RightBrace, "Expected '}'");
    return std::make_unique<InterfaceStmt>(nameTok, std::move(methods), std::move(ext));
}

StmtPtr Parser::deferStatement() {
    Token tok = previous();
    StmtPtr body;
    if (check(TokenType::LeftBrace)) {
        advance();
        std::vector<StmtPtr> stmts;
        while (!check(TokenType::RightBrace) && !isAtEnd()) stmts.push_back(statement());
        consume(TokenType::RightBrace, "Expected '}'");
        body = std::make_unique<BlockStmt>(tok, std::move(stmts));
    } else {
        body = expressionStatement();
    }
    return std::make_unique<DeferStmt>(tok, std::move(body));
}

StmtPtr Parser::withStatement() {
    Token tok = previous();
    consume(TokenType::LeftParen, "Expected '('");
    ExprPtr resource = expression();
    std::string varName;
    if (match(TokenType::As)) varName = std::string(consume(TokenType::Identifier, "Expected variable name").lexeme);
    consume(TokenType::RightParen, "Expected ')'");
    StmtPtr body = statement();
    return std::make_unique<WithStmt>(tok, std::move(resource), std::move(varName), std::move(body));
}

StmtPtr Parser::labeledOrExprStatement() {
    // Check for label: identifier followed by colon
    if (check(TokenType::Identifier) && current_ + 1 < tokens_.size() &&
        tokens_[current_ + 1].type == TokenType::Colon) {
        Token labelTok = advance();
        std::string label = std::string(labelTok.lexeme);
        advance(); // consume ':'
        StmtPtr body = statement();
        return std::make_unique<LabeledStmt>(labelTok, std::move(label), std::move(body));
    }
    return expressionStatement();
}

StmtPtr Parser::expressionStatement() {
    Token tok = peek();
    ExprPtr expr = expression();
    consume(TokenType::Semicolon, "Expected ';' after expression");
    return std::make_unique<ExprStmt>(tok, std::move(expr));
}

StmtPtr Parser::decoratorStatement() {
    Token tok = peek();
    // Collect all decorators
    while (check(TokenType::At)) {
        advance(); // consume @
        std::string name = std::string(consume(TokenType::Identifier, "Expected decorator name").lexeme);
        std::vector<ExprPtr> args;
        if (match(TokenType::LeftParen)) {
            while (!check(TokenType::RightParen) && !isAtEnd()) {
                args.push_back(expression());
                if (!check(TokenType::RightParen)) consume(TokenType::Comma, "Expected ','");
            }
            consume(TokenType::RightParen, "Expected ')'");
        }
        pendingDecorators_.push_back(std::move(name));
        pendingDecoratorArgs_.push_back(std::move(args));
    }
    // The next statement gets the decorators
    return statement();
}

} // namespace claw
