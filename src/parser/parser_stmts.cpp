// parser_stmts.cpp — all statement parsing methods
#include "parser.h"
#include "features/string_pool.h"
#include "interpreter/value.h"

namespace claw {

StmtPtr Parser::statement() {
    if (check(TokenType::At)) return decoratorStatement();
    if (check(TokenType::Async) && checkNext(TokenType::Fn)) { advance(); return fnStatement(true); }
    if (match(TokenType::Print))     return printStatement();
    if (match(TokenType::Let))       return letStatement();
    if (match(TokenType::Const))     return constStatement();
    if (match(TokenType::Reactive)) {
        consume(TokenType::Let, "Expected 'let' after 'reactive'");
        Token n = consume(TokenType::Identifier, "Expected variable name");
        ExprPtr init; if (match(TokenType::Equal)) init = expression();
        consume(TokenType::Semicolon, "Expected ';'");
        return std::make_unique<LetStmt>(n, std::move(init), "", true);
    }
    if (match(TokenType::Fn))        return fnStatement(false);
    if (match(TokenType::Return))    return returnStatement();
    if (match(TokenType::Break))     return breakStatement();
    if (match(TokenType::Continue))  return continueStatement();
    if (match(TokenType::If))        return ifStatement();
    if (match(TokenType::While))     return whileStatement();
    if (match(TokenType::Run))       return runUntilStatement();
    if (match(TokenType::For))       return forStatement();
    if (match(TokenType::LeftBrace)) return blockStatement();
    if (match(TokenType::Try))       return tryStatement();
    if (match(TokenType::Throw))     return throwStatement();
    if (match(TokenType::Import))    return importStatement();
    if (match(TokenType::Export))    return exportStatement();
    if (match(TokenType::Class))     return classStatement();
    if (match(TokenType::Switch))    return switchStatement();
    if (match(TokenType::Match))     return matchStatement();
    if (match(TokenType::Enum))      return enumStatement();
    if (match(TokenType::Interface)) return interfaceStatement();
    if (match(TokenType::Defer))     return deferStatement();
    if (match(TokenType::With))      return withStatement();
    return labeledOrExprStatement();
}

StmtPtr Parser::printStatement() {
    Token tok = previous();
    ExprPtr v = expression();
    consume(TokenType::Semicolon, "Expected ';'");
    return std::make_unique<PrintStmt>(tok, std::move(v));
}

StmtPtr Parser::letStatement() {
    Token letTok = previous();
    // Array destructuring
    if (check(TokenType::LeftBracket)) {
        Token br = advance();
        std::vector<DestructureArrayExpr::Element> elems;
        while (!check(TokenType::RightBracket) && !isAtEnd()) {
            DestructureArrayExpr::Element e;
            if (match(TokenType::Spread)) {
                e.isRest = true;
                e.name = std::string(consume(TokenType::Identifier, "Expected name").lexeme);
            } else if (check(TokenType::Identifier)) {
                e.name = std::string(advance().lexeme);
                if (match(TokenType::Equal)) e.defaultVal = expression();
            } else { e.name = "_"; advance(); }
            elems.push_back(std::move(e));
            if (!check(TokenType::RightBracket)) consume(TokenType::Comma, "Expected ','");
        }
        consume(TokenType::RightBracket, "Expected ']'");
        consume(TokenType::Equal, "Expected '='");
        ExprPtr val = expression();
        consume(TokenType::Semicolon, "Expected ';'");
        return std::make_unique<ExprStmt>(letTok,
            std::make_unique<DestructureArrayExpr>(br, std::move(elems), std::move(val)));
    }
    // Object destructuring
    if (check(TokenType::LeftBrace)) {
        Token br = advance();
        std::vector<DestructureObjectExpr::Property> props;
        while (!check(TokenType::RightBrace) && !isAtEnd()) {
            DestructureObjectExpr::Property p;
            if (match(TokenType::Spread)) {
                p.isRest = true;
                p.key = std::string(consume(TokenType::Identifier, "Expected name").lexeme);
                p.alias = p.key;
            } else {
                p.key = std::string(consume(TokenType::Identifier, "Expected name").lexeme);
                p.alias = p.key;
                if (match(TokenType::Colon)) p.alias = std::string(consume(TokenType::Identifier, "Expected alias").lexeme);
                if (match(TokenType::Equal)) p.defaultVal = expression();
            }
            props.push_back(std::move(p));
            if (!check(TokenType::RightBrace)) consume(TokenType::Comma, "Expected ','");
        }
        consume(TokenType::RightBrace, "Expected '}'");
        consume(TokenType::Equal, "Expected '='");
        ExprPtr val = expression();
        consume(TokenType::Semicolon, "Expected ';'");
        return std::make_unique<ExprStmt>(letTok,
            std::make_unique<DestructureObjectExpr>(br, std::move(props), std::move(val)));
    }
    Token first = consume(TokenType::Identifier, "Expected variable name");
    // Multi-let
    if (check(TokenType::Comma)) {
        std::vector<std::string> names, types;
        names.push_back(std::string(first.lexeme)); types.push_back("");
        while (match(TokenType::Comma)) {
            names.push_back(std::string(consume(TokenType::Identifier, "Expected name").lexeme));
            types.push_back("");
        }
        consume(TokenType::Equal, "Expected '='");
        ExprPtr init = expression();
        consume(TokenType::Semicolon, "Expected ';'");
        return std::make_unique<MultiLetStmt>(letTok, std::move(names), std::move(types), std::move(init));
    }
    std::string typeName;
    if (match(TokenType::Colon)) {
        typeName = std::string(consume(TokenType::Identifier, "Expected type name").lexeme);
        if (match(TokenType::Less)) {
            typeName += "<";
            typeName += std::string(consume(TokenType::Identifier, "Expected type param").lexeme);
            consume(TokenType::Greater, "Expected '>'");
            typeName += ">";
        }
    }
    ExprPtr init;
    if (match(TokenType::Equal)) init = expression();
    consume(TokenType::Semicolon, "Expected ';'");
    return std::make_unique<LetStmt>(first, std::move(init), std::move(typeName));
}

StmtPtr Parser::constStatement() {
    Token constTok = previous();
    Token nameTok = consume(TokenType::Identifier, "Expected constant name");
    std::string typeName;
    if (match(TokenType::Colon)) typeName = std::string(consume(TokenType::Identifier, "Expected type").lexeme);
    consume(TokenType::Equal, "Expected '='");
    ExprPtr init = expression();
    consume(TokenType::Semicolon, "Expected ';'");
    return std::make_unique<ConstStmt>(nameTok, std::move(init), std::move(typeName));
}

StmtPtr Parser::fnStatement(bool isAsync) {
    Token fnTok = previous();
    bool isGen = false;
    if (match(TokenType::Star)) isGen = true;
    Token nameTok = consume(TokenType::Identifier, "Expected function name");
    consume(TokenType::LeftParen, "Expected '('");
    auto params = parseFnParams();
    consume(TokenType::RightParen, "Expected ')'");
    std::string retType;
    if (match(TokenType::Arrow)) retType = std::string(consume(TokenType::Identifier, "Expected return type").lexeme);
    consume(TokenType::LeftBrace, "Expected '{'");
    std::vector<StmtPtr> body;
    while (!check(TokenType::RightBrace) && !isAtEnd()) body.push_back(statement());
    consume(TokenType::RightBrace, "Expected '}'");
    auto fn = std::make_unique<FnStmt>(nameTok, std::move(params), std::move(body), isGen, isAsync, std::move(retType));
    fn->decorators = pendingDecorators_;
    pendingDecorators_.clear(); pendingDecoratorArgs_.clear();
    if (isAsync) return std::make_unique<AsyncFnStmt>(fnTok, std::move(fn));
    return fn;
}

std::vector<FnParam> Parser::parseFnParams() {
    std::vector<FnParam> params;
    while (!check(TokenType::RightParen) && !isAtEnd()) {
        params.push_back(parseFnParam());
        if (!check(TokenType::RightParen)) consume(TokenType::Comma, "Expected ','");
    }
    return params;
}

FnParam Parser::parseFnParam() {
    FnParam p;
    if (match(TokenType::Spread)) p.isRest = true;
    p.name = std::string(consume(TokenType::Identifier, "Expected parameter name").lexeme);
    if (match(TokenType::Colon)) {
        p.typeName = std::string(consume(TokenType::Identifier, "Expected type").lexeme);
        if (match(TokenType::Less)) {
            p.typeName += "<";
            p.typeName += std::string(consume(TokenType::Identifier, "Expected type param").lexeme);
            consume(TokenType::Greater, "Expected '>'");
            p.typeName += ">";
        }
    }
    if (match(TokenType::Equal)) p.defaultValue = expression();
    return p;
}

StmtPtr Parser::returnStatement() {
    Token tok = previous();
    std::vector<ExprPtr> vals;
    if (!check(TokenType::Semicolon) && !isAtEnd()) {
        vals.push_back(expression());
        while (match(TokenType::Comma)) vals.push_back(expression());
    }
    consume(TokenType::Semicolon, "Expected ';'");
    return std::make_unique<ReturnStmt>(tok, std::move(vals));
}

StmtPtr Parser::breakStatement() {
    Token tok = previous();
    std::string lbl;
    if (check(TokenType::Identifier)) lbl = std::string(advance().lexeme);
    consume(TokenType::Semicolon, "Expected ';'");
    return std::make_unique<BreakStmt>(tok, std::move(lbl));
}

StmtPtr Parser::continueStatement() {
    Token tok = previous();
    std::string lbl;
    if (check(TokenType::Identifier)) lbl = std::string(advance().lexeme);
    consume(TokenType::Semicolon, "Expected ';'");
    return std::make_unique<ContinueStmt>(tok, std::move(lbl));
}

StmtPtr Parser::ifStatement() {
    Token tok = previous();
    consume(TokenType::LeftParen, "Expected '('");
    ExprPtr cond = expression();
    consume(TokenType::RightParen, "Expected ')'");
    StmtPtr then = statement();
    StmtPtr els;
    if (match(TokenType::Else)) els = statement();
    return std::make_unique<IfStmt>(tok, std::move(cond), std::move(then), std::move(els));
}

StmtPtr Parser::whileStatement() {
    Token tok = previous();
    consume(TokenType::LeftParen, "Expected '('");
    ExprPtr cond = expression();
    consume(TokenType::RightParen, "Expected ')'");
    return std::make_unique<WhileStmt>(tok, std::move(cond), statement());
}

StmtPtr Parser::runUntilStatement() {
    Token tok = previous();
    consume(TokenType::LeftBrace, "Expected '{'");
    std::vector<StmtPtr> bodyStmts;
    while (!check(TokenType::RightBrace) && !isAtEnd()) bodyStmts.push_back(statement());
    consume(TokenType::RightBrace, "Expected '}'");
    consume(TokenType::Until, "Expected 'until'");
    consume(TokenType::LeftParen, "Expected '('");
    ExprPtr cond = expression();
    consume(TokenType::RightParen, "Expected ')'");
    consume(TokenType::Semicolon, "Expected ';'");
    return std::make_unique<RunUntilStmt>(tok,
        std::make_unique<BlockStmt>(tok, std::move(bodyStmts)), std::move(cond));
}

StmtPtr Parser::forStatement() {
    Token tok = previous();
    consume(TokenType::LeftParen, "Expected '('");
    if (check(TokenType::Let) || check(TokenType::Const)) {
        bool isConst = check(TokenType::Const); advance();
        if (check(TokenType::Identifier)) {
            Token varTok = advance();
            std::string varName = std::string(varTok.lexeme);
            if (match(TokenType::Of)) {
                ExprPtr iter = expression(); consume(TokenType::RightParen, "Expected ')'");
                return std::make_unique<ForOfStmt>(tok, varName, isConst, std::move(iter), statement());
            }
            if (match(TokenType::In)) {
                ExprPtr obj = expression(); consume(TokenType::RightParen, "Expected ')'");
                return std::make_unique<ForInStmt>(tok, varName, isConst, std::move(obj), statement());
            }
            std::string typeName;
            if (match(TokenType::Colon)) typeName = std::string(consume(TokenType::Identifier, "Expected type").lexeme);
            ExprPtr initExpr; if (match(TokenType::Equal)) initExpr = expression();
            auto init = std::make_unique<LetStmt>(varTok, std::move(initExpr), std::move(typeName));
            consume(TokenType::Semicolon, "Expected ';'");
            ExprPtr cond; if (!check(TokenType::Semicolon)) cond = expression();
            consume(TokenType::Semicolon, "Expected ';'");
            ExprPtr inc; if (!check(TokenType::RightParen)) inc = expression();
            consume(TokenType::RightParen, "Expected ')'");
            return std::make_unique<ForStmt>(tok, std::move(init), std::move(cond), std::move(inc), statement());
        }
    }
    StmtPtr init;
    if (!check(TokenType::Semicolon)) {
        if (match(TokenType::Let)) init = letStatement(); else init = expressionStatement();
    } else consume(TokenType::Semicolon, "Expected ';'");
    ExprPtr cond; if (!check(TokenType::Semicolon)) cond = expression();
    consume(TokenType::Semicolon, "Expected ';'");
    ExprPtr inc; if (!check(TokenType::RightParen)) inc = expression();
    consume(TokenType::RightParen, "Expected ')'");
    return std::make_unique<ForStmt>(tok, std::move(init), std::move(cond), std::move(inc), statement());
}

StmtPtr Parser::blockStatement() {
    Token tok = previous();
    std::vector<StmtPtr> stmts;
    while (!check(TokenType::RightBrace) && !isAtEnd()) stmts.push_back(statement());
    consume(TokenType::RightBrace, "Expected '}'");
    return std::make_unique<BlockStmt>(tok, std::move(stmts));
}

StmtPtr Parser::tryStatement() {
    Token tok = previous();
    consume(TokenType::LeftBrace, "Expected '{'");
    std::vector<StmtPtr> tryStmts;
    while (!check(TokenType::RightBrace) && !isAtEnd()) tryStmts.push_back(statement());
    consume(TokenType::RightBrace, "Expected '}'");
    auto tryBody = std::make_unique<BlockStmt>(tok, std::move(tryStmts));
    std::string exVar, exType; StmtPtr catchBody;
    if (match(TokenType::Catch)) {
        consume(TokenType::LeftParen, "Expected '('");
        exVar = std::string(consume(TokenType::Identifier, "Expected exception variable").lexeme);
        if (match(TokenType::Colon)) exType = std::string(consume(TokenType::Identifier, "Expected type").lexeme);
        consume(TokenType::RightParen, "Expected ')'");
        consume(TokenType::LeftBrace, "Expected '{'");
        std::vector<StmtPtr> cs;
        while (!check(TokenType::RightBrace) && !isAtEnd()) cs.push_back(statement());
        consume(TokenType::RightBrace, "Expected '}'");
        catchBody = std::make_unique<BlockStmt>(tok, std::move(cs));
    }
    StmtPtr finallyBody;
    if (match(TokenType::Finally)) {
        consume(TokenType::LeftBrace, "Expected '{'");
        std::vector<StmtPtr> fs;
        while (!check(TokenType::RightBrace) && !isAtEnd()) fs.push_back(statement());
        consume(TokenType::RightBrace, "Expected '}'");
        finallyBody = std::make_unique<BlockStmt>(tok, std::move(fs));
    }
    return std::make_unique<TryStmt>(tok, std::move(tryBody), exVar, exType,
                                      std::move(catchBody), std::move(finallyBody));
}

StmtPtr Parser::throwStatement() {
    Token tok = previous(); ExprPtr expr = expression();
    consume(TokenType::Semicolon, "Expected ';'");
    return std::make_unique<ThrowStmt>(tok, std::move(expr));
}

StmtPtr Parser::importStatement() {
    Token tok = previous();
    std::vector<std::string> imports, aliases;
    bool importAll = false; std::string allAlias;
    if (match(TokenType::Star)) {
        importAll = true;
        consume(TokenType::As, "Expected 'as'");
        allAlias = std::string(consume(TokenType::Identifier, "Expected alias").lexeme);
    } else {
        consume(TokenType::LeftBrace, "Expected '{'");
        while (!check(TokenType::RightBrace) && !isAtEnd()) {
            std::string name = std::string(consume(TokenType::Identifier, "Expected name").lexeme);
            std::string alias = name;
            if (match(TokenType::As)) alias = std::string(consume(TokenType::Identifier, "Expected alias").lexeme);
            imports.push_back(name); aliases.push_back(alias);
            if (!check(TokenType::RightBrace)) consume(TokenType::Comma, "Expected ','");
        }
        consume(TokenType::RightBrace, "Expected '}'");
    }
    consume(TokenType::From, "Expected 'from'");
    std::string path = std::string(consume(TokenType::String, "Expected module path").stringValue);
    consume(TokenType::Semicolon, "Expected ';'");
    auto stmt = std::make_unique<ImportStmt>(tok, std::move(imports), std::move(aliases), std::move(path));
    stmt->importAll = importAll; stmt->allAlias = allAlias;
    return stmt;
}

StmtPtr Parser::exportStatement() {
    Token tok = previous();
    std::vector<std::string> names, aliases;
    bool isDefault = false; ExprPtr defaultExpr;
    if (match(TokenType::Default)) {
        isDefault = true; defaultExpr = expression();
        consume(TokenType::Semicolon, "Expected ';'");
    } else {
        consume(TokenType::LeftBrace, "Expected '{'");
        while (!check(TokenType::RightBrace) && !isAtEnd()) {
            std::string name = std::string(consume(TokenType::Identifier, "Expected name").lexeme);
            std::string alias = name;
            if (match(TokenType::As)) alias = std::string(consume(TokenType::Identifier, "Expected alias").lexeme);
            names.push_back(name); aliases.push_back(alias);
            if (!check(TokenType::RightBrace)) consume(TokenType::Comma, "Expected ','");
        }
        consume(TokenType::RightBrace, "Expected '}'");
        consume(TokenType::Semicolon, "Expected ';'");
    }
    auto stmt = std::make_unique<ExportStmt>(tok, std::move(names), std::move(aliases));
    stmt->isDefault = isDefault; stmt->defaultExpr = std::move(defaultExpr);
    return stmt;
}

StmtPtr Parser::classStatement() {
    Token tok = previous();
    Token nameTok = consume(TokenType::Identifier, "Expected class name");
    ExprPtr superclass; if (match(TokenType::Extends)) superclass = primary();
    std::vector<std::string> interfaces;
    if (match(TokenType::Implements)) {
        do { interfaces.push_back(std::string(consume(TokenType::Identifier, "Expected interface name").lexeme)); }
        while (match(TokenType::Comma));
    }
    consume(TokenType::LeftBrace, "Expected '{'");
    std::vector<ClassMember> members;
    while (!check(TokenType::RightBrace) && !isAtEnd()) members.push_back(parseClassMember());
    consume(TokenType::RightBrace, "Expected '}'");
    auto stmt = std::make_unique<ClassStmt>(nameTok, std::move(superclass), std::move(members), std::move(interfaces));
    stmt->decorators = pendingDecorators_; pendingDecorators_.clear();
    return stmt;
}

ClassMember Parser::parseClassMember() {
    ClassMember m;
    if (match(TokenType::Static)) m.isStatic = true;
    // Getter
    if (check(TokenType::Identifier) && peek().lexeme == "get" && checkNext(TokenType::Identifier)) {
        advance(); m.kind = ClassMember::Kind::Getter;
        m.name = std::string(consume(TokenType::Identifier, "Expected property name").lexeme);
        consume(TokenType::LeftParen, "Expected '('"); consume(TokenType::RightParen, "Expected ')'");
        std::string retType; if (match(TokenType::Arrow)) retType = std::string(consume(TokenType::Identifier, "Expected type").lexeme);
        consume(TokenType::LeftBrace, "Expected '{'");
        std::vector<StmtPtr> body;
        while (!check(TokenType::RightBrace) && !isAtEnd()) body.push_back(statement());
        consume(TokenType::RightBrace, "Expected '}'");
        Token nt(TokenType::Identifier, StringPool::intern(m.name), peek().line);
        m.method = std::make_unique<FnStmt>(nt, std::vector<FnParam>{}, std::move(body), false, false, retType);
        return m;
    }
    // Setter
    if (check(TokenType::Identifier) && peek().lexeme == "set" && checkNext(TokenType::Identifier)) {
        advance(); m.kind = ClassMember::Kind::Setter;
        m.name = std::string(consume(TokenType::Identifier, "Expected property name").lexeme);
        consume(TokenType::LeftParen, "Expected '('");
        auto params = parseFnParams(); consume(TokenType::RightParen, "Expected ')'");
        consume(TokenType::LeftBrace, "Expected '{'");
        std::vector<StmtPtr> body;
        while (!check(TokenType::RightBrace) && !isAtEnd()) body.push_back(statement());
        consume(TokenType::RightBrace, "Expected '}'");
        Token nt(TokenType::Identifier, StringPool::intern(m.name), peek().line);
        m.method = std::make_unique<FnStmt>(nt, std::move(params), std::move(body));
        return m;
    }
    // Operator overload
    if (match(TokenType::Operator)) {
        m.kind = ClassMember::Kind::OperatorOverload;
        Token opTok = advance(); m.opSymbol = std::string(opTok.lexeme);
        m.name = "__op_" + m.opSymbol;
        consume(TokenType::LeftParen, "Expected '('");
        auto params = parseFnParams(); consume(TokenType::RightParen, "Expected ')'");
        consume(TokenType::LeftBrace, "Expected '{'");
        std::vector<StmtPtr> body;
        while (!check(TokenType::RightBrace) && !isAtEnd()) body.push_back(statement());
        consume(TokenType::RightBrace, "Expected '}'");
        Token nt(TokenType::Identifier, StringPool::intern(m.name), peek().line);
        m.method = std::make_unique<FnStmt>(nt, std::move(params), std::move(body));
        return m;
    }
    // Private member
    if (check(TokenType::Hash)) {
        Token ht = advance(); m.isPrivate = true;
        std::string raw = std::string(ht.lexeme);
        m.name = raw.size() > 1 ? raw.substr(1) : raw;
    }
    // Method with optional 'fn' keyword: fn methodName(params) { body }
    if (match(TokenType::Fn)) {
        bool isGen = match(TokenType::Star);
        bool isAsync = false; // already consumed above if async
        Token nt = consume(TokenType::Identifier, "Expected method name");
        if (m.name.empty()) m.name = std::string(nt.lexeme);
        if (m.name == "init" || m.name == "constructor") m.kind = ClassMember::Kind::Constructor;
        else m.kind = m.isStatic ? ClassMember::Kind::StaticMethod : ClassMember::Kind::Method;
        consume(TokenType::LeftParen, "Expected '('");
        auto params = parseFnParams();
        consume(TokenType::RightParen, "Expected ')'");
        std::string retType;
        if (match(TokenType::Arrow)) retType = std::string(consume(TokenType::Identifier, "Expected type").lexeme);
        consume(TokenType::LeftBrace, "Expected '{'");
        std::vector<StmtPtr> body;
        while (!check(TokenType::RightBrace) && !isAtEnd()) body.push_back(statement());
        consume(TokenType::RightBrace, "Expected '}'");
        m.method = std::make_unique<FnStmt>(nt, std::move(params), std::move(body), isGen, isAsync, retType);
        return m;
    }
    // Regular method or field (no 'fn' keyword)
    if (check(TokenType::Identifier)) {
        Token nt = advance();
        if (m.name.empty()) m.name = std::string(nt.lexeme);
        if (m.name == "init" || m.name == "constructor") m.kind = ClassMember::Kind::Constructor;
        else m.kind = m.isStatic ? ClassMember::Kind::StaticMethod : ClassMember::Kind::Method;
        if (check(TokenType::LeftParen)) {
            consume(TokenType::LeftParen, "Expected '('");
            auto params = parseFnParams(); consume(TokenType::RightParen, "Expected ')'");
            std::string retType; if (match(TokenType::Arrow)) retType = std::string(consume(TokenType::Identifier, "Expected type").lexeme);
            consume(TokenType::LeftBrace, "Expected '{'");
            std::vector<StmtPtr> body;
            while (!check(TokenType::RightBrace) && !isAtEnd()) body.push_back(statement());
            consume(TokenType::RightBrace, "Expected '}'");
            m.method = std::make_unique<FnStmt>(nt, std::move(params), std::move(body), false, false, retType);
        } else {
            m.kind = m.isStatic ? ClassMember::Kind::StaticField : ClassMember::Kind::Field;
            if (match(TokenType::Colon)) m.typeName = std::string(consume(TokenType::Identifier, "Expected type").lexeme);
            if (match(TokenType::Equal)) m.fieldInit = expression();
            consume(TokenType::Semicolon, "Expected ';'");
        }
    }
    return m;
}

StmtPtr Parser::switchStatement() {
    Token tok = previous();
    consume(TokenType::LeftParen, "Expected '('");
    ExprPtr expr = expression();
    consume(TokenType::RightParen, "Expected ')'");
    consume(TokenType::LeftBrace, "Expected '{'");
    std::vector<SwitchStmt::Case> cases;
    while (!check(TokenType::RightBrace) && !isAtEnd()) {
        SwitchStmt::Case c;
        if (match(TokenType::Default)) { c.isDefault = true; consume(TokenType::Colon, "Expected ':'"); }
        else { consume(TokenType::Case, "Expected 'case'"); c.isDefault = false; c.match = expression(); consume(TokenType::Colon, "Expected ':'"); }
        while (!check(TokenType::Case) && !check(TokenType::Default) && !check(TokenType::RightBrace) && !isAtEnd())
            c.body.push_back(statement());
        cases.push_back(std::move(c));
    }
    consume(TokenType::RightBrace, "Expected '}'");
    return std::make_unique<SwitchStmt>(tok, std::move(expr), std::move(cases));
}

StmtPtr Parser::matchStatement() {
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
        if (check(TokenType::LeftBrace)) {
            advance();
            while (!check(TokenType::RightBrace) && !isAtEnd()) arm.body.push_back(statement());
            consume(TokenType::RightBrace, "Expected '}'");
        } else {
            arm.bodyExpr = expression();
            consume(TokenType::Semicolon, "Expected ';'");
        }
        arms.push_back(std::move(arm));
    }
    consume(TokenType::RightBrace, "Expected '}'");
    return std::make_unique<ExprStmt>(tok,
        std::make_unique<MatchExpr>(tok, std::move(subject), std::move(arms)));
}

} // namespace claw
