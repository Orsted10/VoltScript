#pragma once
#include "ast.h"
#include "stmt.h"
#include "token.h"
#include <vector>
#include <string>
#include <functional>

namespace claw {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);

    // Parse program (list of statements)
    std::vector<StmtPtr> parseProgram();

    // Parse single expression (for REPL/testing)
    ExprPtr parseExpression();

    // Check for errors
    bool hadError() const { return hadError_; }
    const std::vector<std::string>& getErrors() const { return errors_; }

private:
    // --------------------------------------------------------
    // Statement parsing
    // --------------------------------------------------------
    StmtPtr statement();
    StmtPtr printStatement();
    StmtPtr letStatement();
    StmtPtr constStatement();
    StmtPtr fnStatement(bool isAsync = false);
    StmtPtr returnStatement();
    StmtPtr breakStatement();
    StmtPtr continueStatement();
    StmtPtr ifStatement();
    StmtPtr whileStatement();
    StmtPtr runUntilStatement();
    StmtPtr forStatement();
    StmtPtr blockStatement();
    StmtPtr tryStatement();
    StmtPtr throwStatement();
    StmtPtr importStatement();
    StmtPtr exportStatement();
    StmtPtr classStatement();
    StmtPtr switchStatement();
    StmtPtr matchStatement();
    StmtPtr enumStatement();
    StmtPtr interfaceStatement();
    StmtPtr deferStatement();
    StmtPtr withStatement();
    StmtPtr labeledOrExprStatement();
    StmtPtr expressionStatement();
    StmtPtr decoratorStatement();

    // --------------------------------------------------------
    // Class member parsing
    // --------------------------------------------------------
    ClassMember parseClassMember();
    std::vector<FnParam> parseFnParams();
    FnParam parseFnParam();

    // --------------------------------------------------------
    // Expression parsing (by precedence level)
    // --------------------------------------------------------
    ExprPtr expression();
    ExprPtr assignment();
    ExprPtr ternary();
    ExprPtr nullCoalesce();
    ExprPtr logicalOr();
    ExprPtr logicalAnd();
    ExprPtr bitwiseOr();
    ExprPtr bitwiseXor();
    ExprPtr bitwiseAnd();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr shift();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr power();
    ExprPtr unary();
    ExprPtr postfix();
    ExprPtr call();
    ExprPtr primary();

    // --------------------------------------------------------
    // Special expression parsers
    // --------------------------------------------------------
    ExprPtr arrayLiteral();
    ExprPtr hashMapOrBlock();
    ExprPtr functionExpression(bool isAsync = false, bool isGenerator = false);
    ExprPtr fstringExpression(Token tok);
    ExprPtr templateExpression(Token tok);
    ExprPtr matchExpression();
    ExprPtr finishCall(ExprPtr callee);
    ExprPtr finishIndexOrMember(ExprPtr object);
    ExprPtr finishOptionalChain(ExprPtr object);

    // --------------------------------------------------------
    // Token manipulation
    // --------------------------------------------------------
    Token advance();
    Token peek() const;
    Token peekNext() const;
    Token previous() const;
    bool check(TokenType type) const;
    bool checkNext(TokenType type) const;
    bool match(TokenType type);
    bool match(std::initializer_list<TokenType> types);
    Token consume(TokenType type, const std::string& message);
    bool isAtEnd() const;

    // --------------------------------------------------------
    // Error handling
    // --------------------------------------------------------
    void error(const std::string& message);
    void error(const Token& tok, const std::string& message);
    void synchronize();

    // --------------------------------------------------------
    // Helpers
    // --------------------------------------------------------
    bool isCompoundAssignOp(TokenType t) const;
    TokenType compoundOpToBinary(TokenType t) const;

    std::vector<Token> tokens_;
    size_t current_ = 0;
    bool hadError_ = false;
    std::vector<std::string> errors_;

    // Pending decorators
    std::vector<std::string> pendingDecorators_;
    std::vector<std::vector<ExprPtr>> pendingDecoratorArgs_;
};

} // namespace claw
