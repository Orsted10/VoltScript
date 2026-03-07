#pragma once
#include "ast.h"
#include <vector>
#include <memory>
#include <string>
#include <optional>

namespace claw {

// ============================================================
// Forward declarations
// ============================================================
struct Stmt;
using StmtPtr = std::unique_ptr<Stmt>;

struct ExprStmt;
struct PrintStmt;
struct LetStmt;
struct BlockStmt;
struct IfStmt;
struct WhileStmt;
struct RunUntilStmt;
struct ForStmt;
struct FnStmt;
struct ReturnStmt;
struct BreakStmt;
struct ContinueStmt;
struct TryStmt;
struct ThrowStmt;
struct ImportStmt;
struct ClassStmt;
struct SwitchStmt;
struct ConstStmt;
struct EnumStmt;
struct InterfaceStmt;
struct ForOfStmt;
struct ForInStmt;
struct DeferStmt;
struct AsyncFnStmt;
struct WithStmt;
struct LabeledStmt;
struct MultiLetStmt;
struct ExportStmt;
struct DecoratorStmt;

// ============================================================
// Visitor interface
// ============================================================
class StmtVisitor {
public:
    virtual ~StmtVisitor() = default;
    virtual void visitExprStmt(ExprStmt* stmt) = 0;
    virtual void visitPrintStmt(PrintStmt* stmt) = 0;
    virtual void visitLetStmt(LetStmt* stmt) = 0;
    virtual void visitBlockStmt(BlockStmt* stmt) = 0;
    virtual void visitIfStmt(IfStmt* stmt) = 0;
    virtual void visitWhileStmt(WhileStmt* stmt) = 0;
    virtual void visitRunUntilStmt(RunUntilStmt* stmt) = 0;
    virtual void visitForStmt(ForStmt* stmt) = 0;
    virtual void visitFnStmt(FnStmt* stmt) = 0;
    virtual void visitReturnStmt(ReturnStmt* stmt) = 0;
    virtual void visitBreakStmt(BreakStmt* stmt) = 0;
    virtual void visitContinueStmt(ContinueStmt* stmt) = 0;
    virtual void visitTryStmt(TryStmt* stmt) = 0;
    virtual void visitThrowStmt(ThrowStmt* stmt) = 0;
    virtual void visitImportStmt(ImportStmt* stmt) = 0;
    virtual void visitClassStmt(ClassStmt* stmt) = 0;
    virtual void visitSwitchStmt(SwitchStmt* stmt) = 0;
    virtual void visitConstStmt(ConstStmt* stmt) = 0;
    virtual void visitEnumStmt(EnumStmt* stmt) = 0;
    virtual void visitInterfaceStmt(InterfaceStmt* stmt) = 0;
    virtual void visitForOfStmt(ForOfStmt* stmt) = 0;
    virtual void visitForInStmt(ForInStmt* stmt) = 0;
    virtual void visitDeferStmt(DeferStmt* stmt) = 0;
    virtual void visitAsyncFnStmt(AsyncFnStmt* stmt) = 0;
    virtual void visitWithStmt(WithStmt* stmt) = 0;
    virtual void visitLabeledStmt(LabeledStmt* stmt) = 0;
    virtual void visitMultiLetStmt(MultiLetStmt* stmt) = 0;
    virtual void visitExportStmt(ExportStmt* stmt) = 0;
    virtual void visitDecoratorStmt(DecoratorStmt* stmt) = 0;
};

// ============================================================
// Base statement node
// ============================================================
struct Stmt {
    Token token;
    explicit Stmt(Token tok) : token(tok) {}
    virtual ~Stmt() = default;
    virtual void accept(StmtVisitor& visitor) = 0;
};

// ============================================================
// Original statement nodes
// ============================================================

struct ExprStmt : Stmt {
    ExprPtr expr;
    ExprStmt(Token tok, ExprPtr e) : Stmt(tok), expr(std::move(e)) {}
    void accept(StmtVisitor& visitor) override;
};

struct PrintStmt : Stmt {
    ExprPtr expr;
    PrintStmt(Token tok, ExprPtr e) : Stmt(tok), expr(std::move(e)) {}
    void accept(StmtVisitor& visitor) override;
};

struct LetStmt : Stmt {
    std::string name;
    std::string typeName;
    ExprPtr initializer;
    bool isConst = false;
    bool isReactive = false;
    LetStmt(Token nameTok, ExprPtr init, std::string type = "", bool reactive = false)
        : Stmt(nameTok), name(std::string(nameTok.lexeme)),
          typeName(std::move(type)), initializer(std::move(init)), isReactive(reactive) {}
    void accept(StmtVisitor& visitor) override;
};

struct BlockStmt : Stmt {
    std::vector<StmtPtr> statements;
    explicit BlockStmt(Token tok, std::vector<StmtPtr> stmts)
        : Stmt(tok), statements(std::move(stmts)) {}
    void accept(StmtVisitor& visitor) override;
};

struct IfStmt : Stmt {
    ExprPtr condition;
    StmtPtr thenBranch;
    StmtPtr elseBranch;
    IfStmt(Token tok, ExprPtr cond, StmtPtr then, StmtPtr els)
        : Stmt(tok), condition(std::move(cond)),
          thenBranch(std::move(then)), elseBranch(std::move(els)) {}
    void accept(StmtVisitor& visitor) override;
};

struct WhileStmt : Stmt {
    ExprPtr condition;
    StmtPtr body;
    std::string label;
    WhileStmt(Token tok, ExprPtr cond, StmtPtr body, std::string lbl = "")
        : Stmt(tok), condition(std::move(cond)), body(std::move(body)), label(std::move(lbl)) {}
    void accept(StmtVisitor& visitor) override;
};

struct RunUntilStmt : Stmt {
    StmtPtr body;
    ExprPtr condition;
    RunUntilStmt(Token tok, StmtPtr body, ExprPtr cond)
        : Stmt(tok), body(std::move(body)), condition(std::move(cond)) {}
    void accept(StmtVisitor& visitor) override;
};

struct ForStmt : Stmt {
    StmtPtr initializer;
    ExprPtr condition;
    ExprPtr increment;
    StmtPtr body;
    std::string label;
    ForStmt(Token tok, StmtPtr init, ExprPtr cond, ExprPtr inc, StmtPtr body, std::string lbl = "")
        : Stmt(tok), initializer(std::move(init)), condition(std::move(cond)),
          increment(std::move(inc)), body(std::move(body)), label(std::move(lbl)) {}
    void accept(StmtVisitor& visitor) override;
};

// Function parameter with optional type and default
struct FnParam {
    std::string name;
    std::string typeName;
    ExprPtr defaultValue;
    bool isRest = false;
};

struct FnStmt : Stmt {
    std::string name;
    std::vector<FnParam> params;
    std::vector<StmtPtr> body;
    bool isGenerator = false;
    bool isAsync = false;
    std::string returnType;
    std::vector<std::string> decorators;

    FnStmt(Token nameTok, std::vector<FnParam> params, std::vector<StmtPtr> body,
           bool gen = false, bool async = false, std::string retType = "")
        : Stmt(nameTok), name(std::string(nameTok.lexeme)),
          params(std::move(params)), body(std::move(body)),
          isGenerator(gen), isAsync(async), returnType(std::move(retType)) {}
    void accept(StmtVisitor& visitor) override;
};

struct ReturnStmt : Stmt {
    std::vector<ExprPtr> values;
    ReturnStmt(Token tok, std::vector<ExprPtr> vals)
        : Stmt(tok), values(std::move(vals)) {}
    void accept(StmtVisitor& visitor) override;
};

struct BreakStmt : Stmt {
    std::string label;
    explicit BreakStmt(Token tok, std::string lbl = "")
        : Stmt(tok), label(std::move(lbl)) {}
    void accept(StmtVisitor& visitor) override;
};

struct ContinueStmt : Stmt {
    std::string label;
    explicit ContinueStmt(Token tok, std::string lbl = "")
        : Stmt(tok), label(std::move(lbl)) {}
    void accept(StmtVisitor& visitor) override;
};

struct TryStmt : Stmt {
    StmtPtr tryBody;
    std::string exceptionVar;
    std::string exceptionType;
    StmtPtr catchBody;
    StmtPtr finallyBody;
    TryStmt(Token tryTok, StmtPtr tryB, std::string exVar, std::string exType,
            StmtPtr catchB, StmtPtr finallyB = nullptr)
        : Stmt(tryTok), tryBody(std::move(tryB)), exceptionVar(std::move(exVar)),
          exceptionType(std::move(exType)), catchBody(std::move(catchB)),
          finallyBody(std::move(finallyB)) {}
    void accept(StmtVisitor& visitor) override;
};

struct ThrowStmt : Stmt {
    ExprPtr expression;
    ThrowStmt(Token throwTok, ExprPtr expr)
        : Stmt(throwTok), expression(std::move(expr)) {}
    void accept(StmtVisitor& visitor) override;
};

struct ImportStmt : Stmt {
    std::vector<std::string> imports;
    std::vector<std::string> aliases;
    std::string modulePath;
    bool importAll = false;
    std::string allAlias;
    ImportStmt(Token importTok, std::vector<std::string> imps,
               std::vector<std::string> als, std::string path)
        : Stmt(importTok), imports(std::move(imps)),
          aliases(std::move(als)), modulePath(std::move(path)) {}
    void accept(StmtVisitor& visitor) override;
};

// Class member — opSymbol is a string to avoid Token default-ctor issues
struct ClassMember {
    enum class Kind { Method, Field, StaticMethod, StaticField,
                      Getter, Setter, OperatorOverload, Constructor };
    Kind kind = Kind::Method;
    std::string name;
    bool isPrivate = false;
    bool isStatic = false;
    bool isAbstract = false;
    std::unique_ptr<FnStmt> method;
    ExprPtr fieldInit;
    std::string typeName;
    std::string opSymbol; // for operator overloads: "+", "-", "*", etc.
};

struct ClassStmt : Stmt {
    std::string name;
    ExprPtr superclass;
    std::vector<std::string> interfaces;
    std::vector<ClassMember> members;
    bool isAbstract = false;
    std::vector<std::string> decorators;

    ClassStmt(Token nameTok, ExprPtr super, std::vector<ClassMember> mems,
              std::vector<std::string> ifaces = {}, bool abstract = false)
        : Stmt(nameTok), name(std::string(nameTok.lexeme)),
          superclass(std::move(super)), interfaces(std::move(ifaces)),
          members(std::move(mems)), isAbstract(abstract) {}
    void accept(StmtVisitor& visitor) override;
};

struct SwitchStmt : Stmt {
    struct Case {
        bool isDefault;
        ExprPtr match;
        std::vector<StmtPtr> body;
    };
    ExprPtr expression;
    std::vector<Case> cases;
    SwitchStmt(Token switchTok, ExprPtr expr, std::vector<Case> cs)
        : Stmt(switchTok), expression(std::move(expr)), cases(std::move(cs)) {}
    void accept(StmtVisitor& visitor) override;
};

// ============================================================
// New statement nodes
// ============================================================

struct ConstStmt : Stmt {
    std::string name;
    std::string typeName;
    ExprPtr initializer;
    ConstStmt(Token nameTok, ExprPtr init, std::string type = "")
        : Stmt(nameTok), name(std::string(nameTok.lexeme)),
          typeName(std::move(type)), initializer(std::move(init)) {}
    void accept(StmtVisitor& visitor) override;
};

struct EnumStmt : Stmt {
    struct EnumMember {
        std::string name;
        ExprPtr value;
    };
    std::string name;
    std::vector<EnumMember> members;
    EnumStmt(Token nameTok, std::vector<EnumMember> mems)
        : Stmt(nameTok), name(std::string(nameTok.lexeme)), members(std::move(mems)) {}
    void accept(StmtVisitor& visitor) override;
};

struct InterfaceStmt : Stmt {
    struct Method {
        std::string name;
        std::vector<FnParam> params;
        std::string returnType;
        bool isOptional = false;
    };
    std::string name;
    std::vector<std::string> extends;
    std::vector<Method> methods;
    InterfaceStmt(Token nameTok, std::vector<Method> meths, std::vector<std::string> ext = {})
        : Stmt(nameTok), name(std::string(nameTok.lexeme)),
          extends(std::move(ext)), methods(std::move(meths)) {}
    void accept(StmtVisitor& visitor) override;
};

struct ForOfStmt : Stmt {
    std::string varName;
    bool isConst = false;
    ExprPtr iterable;
    StmtPtr body;
    std::string label;
    ForOfStmt(Token tok, std::string var, bool isConst, ExprPtr iter, StmtPtr body, std::string lbl = "")
        : Stmt(tok), varName(std::move(var)), isConst(isConst),
          iterable(std::move(iter)), body(std::move(body)), label(std::move(lbl)) {}
    void accept(StmtVisitor& visitor) override;
};

struct ForInStmt : Stmt {
    std::string varName;
    bool isConst = false;
    ExprPtr object;
    StmtPtr body;
    std::string label;
    ForInStmt(Token tok, std::string var, bool isConst, ExprPtr obj, StmtPtr body, std::string lbl = "")
        : Stmt(tok), varName(std::move(var)), isConst(isConst),
          object(std::move(obj)), body(std::move(body)), label(std::move(lbl)) {}
    void accept(StmtVisitor& visitor) override;
};

struct DeferStmt : Stmt {
    StmtPtr body;
    explicit DeferStmt(Token tok, StmtPtr body) : Stmt(tok), body(std::move(body)) {}
    void accept(StmtVisitor& visitor) override;
};

struct AsyncFnStmt : Stmt {
    std::unique_ptr<FnStmt> fn;
    explicit AsyncFnStmt(Token tok, std::unique_ptr<FnStmt> f) : Stmt(tok), fn(std::move(f)) {}
    void accept(StmtVisitor& visitor) override;
};

struct WithStmt : Stmt {
    ExprPtr resource;
    std::string varName;
    StmtPtr body;
    WithStmt(Token tok, ExprPtr res, std::string var, StmtPtr body)
        : Stmt(tok), resource(std::move(res)), varName(std::move(var)), body(std::move(body)) {}
    void accept(StmtVisitor& visitor) override;
};

struct LabeledStmt : Stmt {
    std::string label;
    StmtPtr body;
    LabeledStmt(Token tok, std::string lbl, StmtPtr body)
        : Stmt(tok), label(std::move(lbl)), body(std::move(body)) {}
    void accept(StmtVisitor& visitor) override;
};

struct MultiLetStmt : Stmt {
    std::vector<std::string> names;
    std::vector<std::string> typeNames;
    ExprPtr initializer;
    MultiLetStmt(Token tok, std::vector<std::string> names,
                 std::vector<std::string> types, ExprPtr init)
        : Stmt(tok), names(std::move(names)), typeNames(std::move(types)),
          initializer(std::move(init)) {}
    void accept(StmtVisitor& visitor) override;
};

struct ExportStmt : Stmt {
    std::vector<std::string> names;
    std::vector<std::string> aliases;
    ExprPtr defaultExpr;
    bool isDefault = false;
    ExportStmt(Token tok, std::vector<std::string> names, std::vector<std::string> als)
        : Stmt(tok), names(std::move(names)), aliases(std::move(als)) {}
    void accept(StmtVisitor& visitor) override;
};

struct DecoratorStmt : Stmt {
    std::vector<std::string> decoratorNames;
    std::vector<std::vector<ExprPtr>> decoratorArgs;
    StmtPtr target;
    DecoratorStmt(Token tok, std::vector<std::string> names,
                  std::vector<std::vector<ExprPtr>> args, StmtPtr target)
        : Stmt(tok), decoratorNames(std::move(names)),
          decoratorArgs(std::move(args)), target(std::move(target)) {}
    void accept(StmtVisitor& visitor) override;
};

// FunctionExpr is defined in ast.h but referenced in stmt.h for FnStmt body
// (already included via ast.h)

} // namespace claw
