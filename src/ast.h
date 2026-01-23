#pragma once
#include <memory>
#include <string>
#include <vector>
#include "token.h"

namespace volt {

// Forward declarations
struct Expr;
using ExprPtr = std::unique_ptr<Expr>;

// Base expression node
struct Expr {
    virtual ~Expr() = default;
};

// Literal: 42, 3.14, "hello", true, false, nil
struct LiteralExpr : Expr {
    enum class Type { Number, String, Bool, Nil };
    
    Type type;
    double numberValue;
    std::string stringValue;
    bool boolValue;
    
    explicit LiteralExpr(double value)
        : type(Type::Number), numberValue(value), boolValue(false) {}
    
    explicit LiteralExpr(const std::string& value)
        : type(Type::String), numberValue(0.0), stringValue(value), boolValue(false) {}
    
    explicit LiteralExpr(bool value)
        : type(Type::Bool), numberValue(0.0), boolValue(value) {}
    
    static ExprPtr nil() {
        auto expr = std::make_unique<LiteralExpr>(0.0);
        expr->type = Type::Nil;
        return expr;
    }
};

// Variable: x, myVar
struct VariableExpr : Expr {
    std::string name;
    explicit VariableExpr(std::string n) : name(std::move(n)) {}
};

// Unary: -x, !flag
struct UnaryExpr : Expr {
    Token op;
    ExprPtr right;
    UnaryExpr(Token o, ExprPtr r)
        : op(o), right(std::move(r)) {}
};

// Binary: 1 + 2, x * y, a == b
struct BinaryExpr : Expr {
    ExprPtr left;
    Token op;
    ExprPtr right;
    BinaryExpr(ExprPtr l, Token o, ExprPtr r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
};

// Logical: a && b, x || y
struct LogicalExpr : Expr {
    ExprPtr left;
    Token op;
    ExprPtr right;
    LogicalExpr(ExprPtr l, Token o, ExprPtr r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
};

// Grouping: (expr)
struct GroupingExpr : Expr {
    ExprPtr expr;
    explicit GroupingExpr(ExprPtr e) : expr(std::move(e)) {}
};

// Call: foo(a, b, c)
struct CallExpr : Expr {
    ExprPtr callee;
    std::vector<ExprPtr> arguments;
    CallExpr(ExprPtr c, std::vector<ExprPtr> args)
        : callee(std::move(c)), arguments(std::move(args)) {}
};

// Assignment: x = 10
struct AssignExpr : Expr {
    std::string name;
    ExprPtr value;
    AssignExpr(std::string n, ExprPtr v)
        : name(std::move(n)), value(std::move(v)) {}
};

// Compound Assignment: x += 10, x -= 5, etc.
struct CompoundAssignExpr : Expr {
    std::string name;
    Token op;
    ExprPtr value;
    CompoundAssignExpr(std::string n, Token o, ExprPtr v)
        : name(std::move(n)), op(o), value(std::move(v)) {}
};

// Update Expression: ++x, x++, --x, x--
struct UpdateExpr : Expr {
    std::string name;
    Token op;
    bool prefix; // true for ++x, false for x++
    UpdateExpr(std::string n, Token o, bool pre)
        : name(std::move(n)), op(o), prefix(pre) {}
};

// Ternary: condition ? thenExpr : elseExpr
struct TernaryExpr : Expr {
    ExprPtr condition;
    ExprPtr thenBranch;
    ExprPtr elseBranch;
    TernaryExpr(ExprPtr cond, ExprPtr then_, ExprPtr else_)
        : condition(std::move(cond)),
          thenBranch(std::move(then_)),
          elseBranch(std::move(else_)) {}
};

// ========================================
// ARRAY EXPRESSIONS - NEW!
// ========================================

// Array Literal: [1, 2, 3, "hello"]
struct ArrayExpr : Expr {
    std::vector<ExprPtr> elements;
    explicit ArrayExpr(std::vector<ExprPtr> elems)
        : elements(std::move(elems)) {}
};

// Array Index Access: arr[0], matrix[i][j]
struct IndexExpr : Expr {
    ExprPtr object;  // The array being indexed
    ExprPtr index;   // The index expression
    
    IndexExpr(ExprPtr obj, ExprPtr idx)
        : object(std::move(obj)), index(std::move(idx)) {}
};

// Array Index Assignment: arr[0] = 42
struct IndexAssignExpr : Expr {
    ExprPtr object;  // The array being indexed
    ExprPtr index;   // The index expression
    ExprPtr value;   // The value to assign
    
    IndexAssignExpr(ExprPtr obj, ExprPtr idx, ExprPtr val)
        : object(std::move(obj)), index(std::move(idx)), value(std::move(val)) {}
};

// Member Access: array.length, array.push
struct MemberExpr : Expr {
    ExprPtr object;      // The object (array, etc.)
    std::string member;  // The member name (length, push, etc.)
    
    MemberExpr(ExprPtr obj, std::string mem)
        : object(std::move(obj)), member(std::move(mem)) {}
};

// AST Pretty Printer
std::string printAST(Expr* expr);

} // namespace volt
