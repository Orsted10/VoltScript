#pragma once
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include "token.h"
#include "value.h"

namespace claw {

// ============================================================
// Forward declarations
// ============================================================
struct Expr;
using ExprPtr = std::unique_ptr<Expr>;

struct Stmt;
using StmtPtr = std::unique_ptr<Stmt>;

// All expression node types
struct LiteralExpr;
struct VariableExpr;
struct UnaryExpr;
struct BinaryExpr;
struct LogicalExpr;
struct GroupingExpr;
struct CallExpr;
struct AssignExpr;
struct CompoundAssignExpr;
struct CompoundMemberAssignExpr;
struct CompoundIndexAssignExpr;
struct UpdateExpr;
struct UpdateMemberExpr;
struct UpdateIndexExpr;
struct TernaryExpr;
struct ArrayExpr;
struct IndexExpr;
struct IndexAssignExpr;
struct HashMapExpr;
struct MemberExpr;
struct SetExpr;
struct ThisExpr;
struct SuperExpr;
struct FunctionExpr;
// New expression nodes
struct FStringExpr;
struct TemplateExpr;
struct SpreadExpr;
struct OptionalChainExpr;
struct NullCoalesceExpr;
struct PipeExpr;
struct AwaitExpr;
struct YieldExpr;
struct MatchExpr;
struct ComprehensionExpr;
struct DestructureArrayExpr;
struct DestructureObjectExpr;
struct TypeAnnotationExpr;
struct NewExpr;
struct MetaExpr;

// ============================================================
// Visitor interface
// ============================================================
class ExprVisitor {
public:
    virtual ~ExprVisitor() = default;

    // Original nodes
    virtual Value visitLiteralExpr(LiteralExpr* expr) = 0;
    virtual Value visitVariableExpr(VariableExpr* expr) = 0;
    virtual Value visitUnaryExpr(UnaryExpr* expr) = 0;
    virtual Value visitBinaryExpr(BinaryExpr* expr) = 0;
    virtual Value visitLogicalExpr(LogicalExpr* expr) = 0;
    virtual Value visitGroupingExpr(GroupingExpr* expr) = 0;
    virtual Value visitCallExpr(CallExpr* expr) = 0;
    virtual Value visitAssignExpr(AssignExpr* expr) = 0;
    virtual Value visitCompoundAssignExpr(CompoundAssignExpr* expr) = 0;
    virtual Value visitCompoundMemberAssignExpr(CompoundMemberAssignExpr* expr) = 0;
    virtual Value visitCompoundIndexAssignExpr(CompoundIndexAssignExpr* expr) = 0;
    virtual Value visitUpdateExpr(UpdateExpr* expr) = 0;
    virtual Value visitUpdateMemberExpr(UpdateMemberExpr* expr) = 0;
    virtual Value visitUpdateIndexExpr(UpdateIndexExpr* expr) = 0;
    virtual Value visitTernaryExpr(TernaryExpr* expr) = 0;
    virtual Value visitArrayExpr(ArrayExpr* expr) = 0;
    virtual Value visitIndexExpr(IndexExpr* expr) = 0;
    virtual Value visitIndexAssignExpr(IndexAssignExpr* expr) = 0;
    virtual Value visitHashMapExpr(HashMapExpr* expr) = 0;
    virtual Value visitMemberExpr(MemberExpr* expr) = 0;
    virtual Value visitSetExpr(SetExpr* expr) = 0;
    virtual Value visitThisExpr(ThisExpr* expr) = 0;
    virtual Value visitSuperExpr(SuperExpr* expr) = 0;
    virtual Value visitFunctionExpr(FunctionExpr* expr) = 0;

    // New nodes
    virtual Value visitFStringExpr(FStringExpr* expr) = 0;
    virtual Value visitTemplateExpr(TemplateExpr* expr) = 0;
    virtual Value visitSpreadExpr(SpreadExpr* expr) = 0;
    virtual Value visitOptionalChainExpr(OptionalChainExpr* expr) = 0;
    virtual Value visitNullCoalesceExpr(NullCoalesceExpr* expr) = 0;
    virtual Value visitPipeExpr(PipeExpr* expr) = 0;
    virtual Value visitAwaitExpr(AwaitExpr* expr) = 0;
    virtual Value visitYieldExpr(YieldExpr* expr) = 0;
    virtual Value visitMatchExpr(MatchExpr* expr) = 0;
    virtual Value visitComprehensionExpr(ComprehensionExpr* expr) = 0;
    virtual Value visitDestructureArrayExpr(DestructureArrayExpr* expr) = 0;
    virtual Value visitDestructureObjectExpr(DestructureObjectExpr* expr) = 0;
    virtual Value visitTypeAnnotationExpr(TypeAnnotationExpr* expr) = 0;
    virtual Value visitNewExpr(NewExpr* expr) = 0;
    virtual Value visitMetaExpr(MetaExpr* expr) = 0;
};

// ============================================================
// Base expression node
// ============================================================
struct Expr {
    Token token;
    explicit Expr(Token tok) : token(tok) {}
    virtual ~Expr() = default;
    virtual Value accept(ExprVisitor& visitor) = 0;
};

// ============================================================
// Original expression nodes
// ============================================================

struct LiteralExpr : Expr {
    Value value;
    LiteralExpr(Token tok, Value val) : Expr(tok), value(val) {}
    Value accept(ExprVisitor& visitor) override;
};

struct VariableExpr : Expr {
    std::string name;
    VariableExpr(Token tok) : Expr(tok), name(std::string(tok.lexeme)) {}
    Value accept(ExprVisitor& visitor) override;
};

struct UnaryExpr : Expr {
    Token op;
    ExprPtr right;
    UnaryExpr(Token op, ExprPtr right)
        : Expr(op), op(op), right(std::move(right)) {}
    Value accept(ExprVisitor& visitor) override;
};

struct BinaryExpr : Expr {
    ExprPtr left;
    Token op;
    ExprPtr right;
    BinaryExpr(ExprPtr left, Token op, ExprPtr right)
        : Expr(op), left(std::move(left)), op(op), right(std::move(right)) {}
    Value accept(ExprVisitor& visitor) override;
};

struct LogicalExpr : Expr {
    ExprPtr left;
    Token op;
    ExprPtr right;
    LogicalExpr(ExprPtr left, Token op, ExprPtr right)
        : Expr(op), left(std::move(left)), op(op), right(std::move(right)) {}
    Value accept(ExprVisitor& visitor) override;
};

struct GroupingExpr : Expr {
    ExprPtr expression;
    GroupingExpr(Token tok, ExprPtr expr)
        : Expr(tok), expression(std::move(expr)) {}
    Value accept(ExprVisitor& visitor) override;
};

struct CallExpr : Expr {
    ExprPtr callee;
    std::vector<ExprPtr> arguments;
    bool hasSpread = false; // any spread arg?
    CallExpr(Token paren, ExprPtr callee, std::vector<ExprPtr> args)
        : Expr(paren), callee(std::move(callee)), arguments(std::move(args)) {}
    Value accept(ExprVisitor& visitor) override;
};

struct AssignExpr : Expr {
    std::string name;
    ExprPtr value;
    AssignExpr(Token name, ExprPtr value)
        : Expr(name), name(std::string(name.lexeme)), value(std::move(value)) {}
    Value accept(ExprVisitor& visitor) override;
};

struct CompoundAssignExpr : Expr {
    std::string name;
    Token op;
    ExprPtr value;
    CompoundAssignExpr(Token name, Token op, ExprPtr value)
        : Expr(name), name(std::string(name.lexeme)), op(op), value(std::move(value)) {}
    Value accept(ExprVisitor& visitor) override;
};

struct CompoundMemberAssignExpr : Expr {
    ExprPtr object;
    std::string member;
    Token op;
    ExprPtr value;
    CompoundMemberAssignExpr(Token tok, ExprPtr obj, std::string mem, Token op, ExprPtr val)
        : Expr(tok), object(std::move(obj)), member(std::move(mem)), op(op), value(std::move(val)) {}
    Value accept(ExprVisitor& visitor) override;
};

struct CompoundIndexAssignExpr : Expr {
    ExprPtr object;
    ExprPtr index;
    Token op;
    ExprPtr value;
    CompoundIndexAssignExpr(Token tok, ExprPtr obj, ExprPtr idx, Token op, ExprPtr val)
        : Expr(tok), object(std::move(obj)), index(std::move(idx)), op(op), value(std::move(val)) {}
    Value accept(ExprVisitor& visitor) override;
};

struct UpdateExpr : Expr {
    Token op;
    std::string name;
    bool prefix;
    UpdateExpr(Token op, std::string name, bool prefix)
        : Expr(op), op(op), name(std::move(name)), prefix(prefix) {}
    Value accept(ExprVisitor& visitor) override;
};

struct UpdateMemberExpr : Expr {
    Token op;
    ExprPtr object;
    std::string member;
    bool prefix;
    UpdateMemberExpr(Token op, ExprPtr obj, std::string mem, bool prefix)
        : Expr(op), op(op), object(std::move(obj)), member(std::move(mem)), prefix(prefix) {}
    Value accept(ExprVisitor& visitor) override;
};

struct UpdateIndexExpr : Expr {
    Token op;
    ExprPtr object;
    ExprPtr index;
    bool prefix;
    UpdateIndexExpr(Token op, ExprPtr obj, ExprPtr idx, bool prefix)
        : Expr(op), op(op), object(std::move(obj)), index(std::move(idx)), prefix(prefix) {}
    Value accept(ExprVisitor& visitor) override;
};

struct TernaryExpr : Expr {
    ExprPtr condition;
    ExprPtr thenBranch;
    ExprPtr elseBranch;
    TernaryExpr(Token tok, ExprPtr cond, ExprPtr then, ExprPtr els)
        : Expr(tok), condition(std::move(cond)),
          thenBranch(std::move(then)), elseBranch(std::move(els)) {}
    Value accept(ExprVisitor& visitor) override;
};

struct ArrayExpr : Expr {
    std::vector<ExprPtr> elements;
    ArrayExpr(Token bracket, std::vector<ExprPtr> elems)
        : Expr(bracket), elements(std::move(elems)) {}
    Value accept(ExprVisitor& visitor) override;
};

struct IndexExpr : Expr {
    ExprPtr object;
    ExprPtr index;
    IndexExpr(Token bracket, ExprPtr obj, ExprPtr idx)
        : Expr(bracket), object(std::move(obj)), index(std::move(idx)) {}
    Value accept(ExprVisitor& visitor) override;
};

struct IndexAssignExpr : Expr {
    ExprPtr object;
    ExprPtr index;
    ExprPtr value;
    IndexAssignExpr(Token bracket, ExprPtr obj, ExprPtr idx, ExprPtr val)
        : Expr(bracket), object(std::move(obj)),
          index(std::move(idx)), value(std::move(val)) {}
    Value accept(ExprVisitor& visitor) override;
};

struct HashMapExpr : Expr {
    std::vector<std::pair<ExprPtr, ExprPtr>> keyValuePairs;
    HashMapExpr(Token brace, std::vector<std::pair<ExprPtr, ExprPtr>> pairs)
        : Expr(brace), keyValuePairs(std::move(pairs)) {}
    Value accept(ExprVisitor& visitor) override;
};

struct MemberExpr : Expr {
    ExprPtr object;
    std::string member;
    MemberExpr(Token name, ExprPtr obj, std::string mem)
        : Expr(name), object(std::move(obj)), member(std::move(mem)) {}
    Value accept(ExprVisitor& visitor) override;
};

struct SetExpr : Expr {
    ExprPtr object;
    std::string member;
    ExprPtr value;
    SetExpr(Token name, ExprPtr obj, std::string mem, ExprPtr val)
        : Expr(name), object(std::move(obj)),
          member(std::move(mem)), value(std::move(val)) {}
    Value accept(ExprVisitor& visitor) override;
};

struct ThisExpr : Expr {
    explicit ThisExpr(Token keyword) : Expr(keyword) {}
    Value accept(ExprVisitor& visitor) override;
};

struct SuperExpr : Expr {
    std::string method;
    SuperExpr(Token keyword, std::string m)
        : Expr(keyword), method(std::move(m)) {}
    Value accept(ExprVisitor& visitor) override;
};

// ============================================================
// New expression nodes — Phase 1
// ============================================================

// F-string: f"Hello {name}, you are {age}!"
// Segments alternate: literal text, expression, literal text, ...
struct FStringExpr : Expr {
    struct Segment {
        bool isExpr;
        std::string text;       // for literal segments
        ExprPtr expr;           // for expression segments
    };
    std::vector<Segment> segments;
    FStringExpr(Token tok, std::vector<Segment> segs)
        : Expr(tok), segments(std::move(segs)) {}
    Value accept(ExprVisitor& visitor) override;
};

// Template literal: `Hello ${name}!`
struct TemplateExpr : Expr {
    struct Segment {
        bool isExpr;
        std::string text;
        ExprPtr expr;
    };
    std::vector<Segment> segments;
    TemplateExpr(Token tok, std::vector<Segment> segs)
        : Expr(tok), segments(std::move(segs)) {}
    Value accept(ExprVisitor& visitor) override;
};

// Spread: ...expr
struct SpreadExpr : Expr {
    ExprPtr expr;
    explicit SpreadExpr(Token tok, ExprPtr e)
        : Expr(tok), expr(std::move(e)) {}
    Value accept(ExprVisitor& visitor) override;
};

// Optional chaining: obj?.prop or obj?.[idx] or obj?.method()
struct OptionalChainExpr : Expr {
    ExprPtr object;
    std::string member;     // for ?.prop
    ExprPtr index;          // for ?.[idx]
    std::vector<ExprPtr> args; // for ?.method()
    enum class Kind { Member, Index, Call } kind;
    OptionalChainExpr(Token tok, ExprPtr obj, std::string mem)
        : Expr(tok), object(std::move(obj)), member(std::move(mem)),
          kind(Kind::Member) {}
    OptionalChainExpr(Token tok, ExprPtr obj, ExprPtr idx)
        : Expr(tok), object(std::move(obj)), index(std::move(idx)),
          kind(Kind::Index) {}
    Value accept(ExprVisitor& visitor) override;
};

// Null coalescing: left ?? right
struct NullCoalesceExpr : Expr {
    ExprPtr left;
    ExprPtr right;
    NullCoalesceExpr(Token tok, ExprPtr l, ExprPtr r)
        : Expr(tok), left(std::move(l)), right(std::move(r)) {}
    Value accept(ExprVisitor& visitor) override;
};

// Pipe operator: expr |> fn
struct PipeExpr : Expr {
    ExprPtr left;
    ExprPtr right; // must be callable
    PipeExpr(Token tok, ExprPtr l, ExprPtr r)
        : Expr(tok), left(std::move(l)), right(std::move(r)) {}
    Value accept(ExprVisitor& visitor) override;
};

// Await: await expr
struct AwaitExpr : Expr {
    ExprPtr expr;
    explicit AwaitExpr(Token tok, ExprPtr e)
        : Expr(tok), expr(std::move(e)) {}
    Value accept(ExprVisitor& visitor) override;
};

// Yield: yield [expr]
struct YieldExpr : Expr {
    ExprPtr expr; // may be null for bare yield
    explicit YieldExpr(Token tok, ExprPtr e = nullptr)
        : Expr(tok), expr(std::move(e)) {}
    Value accept(ExprVisitor& visitor) override;
};

// Match expression arm
struct MatchArm {
    bool isDefault = false;
    std::vector<ExprPtr> patterns;  // multiple patterns: case 1 | 2 | 3
    ExprPtr guard;                  // optional: case n if n > 10
    std::vector<StmtPtr> body;      // body statements
    ExprPtr bodyExpr;               // OR single expression body
};

// Match expression: match val { case 1: ... case _: ... }
struct MatchExpr : Expr {
    ExprPtr subject;
    std::vector<MatchArm> arms;
    MatchExpr(Token tok, ExprPtr subj, std::vector<MatchArm> a)
        : Expr(tok), subject(std::move(subj)), arms(std::move(a)) {}
    Value accept(ExprVisitor& visitor) override;
};

// List comprehension: [expr for name in iterable if condition]
struct ComprehensionExpr : Expr {
    ExprPtr body;           // expression to evaluate
    std::string varName;    // loop variable
    ExprPtr iterable;       // what to iterate over
    ExprPtr condition;      // optional filter (if ...)
    bool isMap = false;     // {k: v for ...} style
    ExprPtr keyExpr;        // for map comprehensions
    ComprehensionExpr(Token tok, ExprPtr body, std::string var,
                      ExprPtr iter, ExprPtr cond = nullptr)
        : Expr(tok), body(std::move(body)), varName(std::move(var)),
          iterable(std::move(iter)), condition(std::move(cond)) {}
    Value accept(ExprVisitor& visitor) override;
};

// Array destructuring: let [a, b, ...rest] = expr
struct DestructureArrayExpr : Expr {
    struct Element {
        std::string name;
        bool isRest = false;
        ExprPtr defaultVal;
    };
    std::vector<Element> elements;
    ExprPtr value;
    DestructureArrayExpr(Token tok, std::vector<Element> elems, ExprPtr val)
        : Expr(tok), elements(std::move(elems)), value(std::move(val)) {}
    Value accept(ExprVisitor& visitor) override;
};

// Object destructuring: let {x, y: renamed, z = default} = expr
struct DestructureObjectExpr : Expr {
    struct Property {
        std::string key;
        std::string alias;      // renamed binding (empty = same as key)
        ExprPtr defaultVal;
        bool isRest = false;    // ...rest
    };
    std::vector<Property> properties;
    ExprPtr value;
    DestructureObjectExpr(Token tok, std::vector<Property> props, ExprPtr val)
        : Expr(tok), properties(std::move(props)), value(std::move(val)) {}
    Value accept(ExprVisitor& visitor) override;
};

// Type annotation: expr: TypeName (used in params, let, etc.)
struct TypeAnnotationExpr : Expr {
    ExprPtr expr;
    std::string typeName;
    bool isOptional = false; // type?
    TypeAnnotationExpr(Token tok, ExprPtr e, std::string type, bool opt = false)
        : Expr(tok), expr(std::move(e)), typeName(std::move(type)), isOptional(opt) {}
    Value accept(ExprVisitor& visitor) override;
};

// new ClassName(args)
struct NewExpr : Expr {
    ExprPtr callee;
    std::vector<ExprPtr> arguments;
    NewExpr(Token tok, ExprPtr callee, std::vector<ExprPtr> args)
        : Expr(tok), callee(std::move(callee)), arguments(std::move(args)) {}
    Value accept(ExprVisitor& visitor) override;
};

// Metatable access: __meta(obj) or setMeta(obj, table)
struct MetaExpr : Expr {
    ExprPtr object;
    ExprPtr metatable; // null for get, non-null for set
    MetaExpr(Token tok, ExprPtr obj, ExprPtr meta = nullptr)
        : Expr(tok), object(std::move(obj)), metatable(std::move(meta)) {}
    Value accept(ExprVisitor& visitor) override;
};

// Function expression (anonymous): fn(params) { body } or fn*(params) { body }
struct FunctionExpr : Expr {
    std::vector<std::string> parameters;
    std::vector<std::string> paramTypes;    // optional type annotations
    std::vector<ExprPtr> paramDefaults;     // optional default values
    bool hasRest = false;                   // last param is ...rest
    std::vector<StmtPtr> body;
    bool isGenerator = false;               // fn* generator
    bool isAsync = false;                   // async fn
    std::string returnType;                 // optional return type annotation

    FunctionExpr(Token keyword,
                 std::vector<std::string> params,
                 std::vector<StmtPtr> b,
                 bool gen = false,
                 bool async = false)
        : Expr(keyword),
          parameters(std::move(params)),
          body(std::move(b)),
          isGenerator(gen),
          isAsync(async) {}
    Value accept(ExprVisitor& visitor) override;
};

// ============================================================
// AST Pretty Printer
// ============================================================
std::string printAST(Expr* expr);

} // namespace claw
