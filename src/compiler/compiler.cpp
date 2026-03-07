#include "compiler.h"
#include "features/string_pool.h"
#include <iostream>
#include <cmath>
#include <cstdint>

namespace claw {

Compiler::Compiler() : currentLine_(0), scopeDepth_(0), enclosing_(nullptr) {}
Compiler::Compiler(Compiler* enclosing) : currentLine_(0), scopeDepth_(0), enclosing_(enclosing) {}

std::unique_ptr<Chunk> Compiler::compile(const std::vector<StmtPtr>& program) {
    chunk_ = std::make_unique<Chunk>();
    locals_.clear();
    upvalues_.clear();
    scopeDepth_ = 0;
    enclosing_ = nullptr;
    
    for (const auto& stmt : program) {
        currentLine_ = stmt->token.line;
        stmt->accept(*this);
    }
    
    emitOp(OpCode::Return);
    chunk_->setLoopCount(static_cast<int>(chunk_->countOpcode(OpCode::Loop)));
    return std::move(chunk_);
}

// ExprVisitor implementation

Value Compiler::visitLiteralExpr(LiteralExpr* expr) {
    Value v = expr->value;
    if (v == nilValue()) {
        emitOp(OpCode::Nil);
    } else if (v == boolValue(true)) {
        emitOp(OpCode::True);
    } else if (v == boolValue(false)) {
        emitOp(OpCode::False);
    } else {
        emitConstant(v);
    }
    return nilValue();
}

Value Compiler::visitVariableExpr(VariableExpr* expr) {
    std::string_view name = expr->token.lexeme;
    int arg = resolveLocal(name);
    
    if (arg != -1) {
        emitOp(OpCode::GetLocal);
        emitByte(static_cast<uint8_t>(arg));
    } else if ((arg = resolveUpvalue(name)) != -1) {
        emitOp(OpCode::GetUpvalue);
        emitByte(static_cast<uint8_t>(arg));
    } else {
        emitOp(OpCode::GetGlobal);
        auto sv = StringPool::intern(name);
        emitByte(makeConstant(stringValue(sv.data())));
    }
    return nilValue();
}

Value Compiler::visitBinaryExpr(BinaryExpr* expr) {
    expr->left->accept(*this);
    expr->right->accept(*this);
    
    switch (expr->op.type) {
        case TokenType::Plus:  emitOp(OpCode::Add); break;
        case TokenType::Minus: emitOp(OpCode::Subtract); break;
        case TokenType::Star:  emitOp(OpCode::Multiply); break;
        case TokenType::Slash: emitOp(OpCode::Divide); break;
        case TokenType::BitAnd:       emitOp(OpCode::BitAnd); break;
        case TokenType::BitOr:        emitOp(OpCode::BitOr); break;
        case TokenType::BitXor:       emitOp(OpCode::BitXor); break;
        case TokenType::ShiftLeft:    emitOp(OpCode::ShiftLeft); break;
        case TokenType::ShiftRight:   emitOp(OpCode::ShiftRight); break;
        case TokenType::Greater:      emitOp(OpCode::Greater); break;
        case TokenType::GreaterEqual: {
            emitOp(OpCode::Less);
            emitOp(OpCode::Not);
            break;
        }
        case TokenType::Less:         emitOp(OpCode::Less); break;
        case TokenType::LessEqual: {
            emitOp(OpCode::Greater);
            emitOp(OpCode::Not);
            break;
        }
        case TokenType::EqualEqual:   emitOp(OpCode::Equal); break;
        case TokenType::BangEqual: {
            emitOp(OpCode::Equal);
            emitOp(OpCode::Not);
            break;
        }
        default: return nilValue();
    }
    return nilValue();
}

Value Compiler::visitUnaryExpr(UnaryExpr* expr) {
    expr->right->accept(*this);
    
    switch (expr->op.type) {
        case TokenType::Minus: emitOp(OpCode::Negate); break;
        case TokenType::Bang:  emitOp(OpCode::Not); break;
        default: return nilValue();
    }
    return nilValue();
}

Value Compiler::visitLogicalExpr(LogicalExpr* expr) {
    // Basic logical op - could be optimized with short-circuiting jumps
    expr->left->accept(*this);
    expr->right->accept(*this);
    return nilValue();
}

Value Compiler::visitGroupingExpr(GroupingExpr* expr) {
    return expr->expression->accept(*this);
}

Value Compiler::visitCallExpr(CallExpr* expr) {
    expr->callee->accept(*this);
    uint8_t argCount = 0;
    for (const auto& argument : expr->arguments) {
        argument->accept(*this);
        argCount++;
    }
    emitOp(OpCode::Call);
    emitByte(argCount);
    return nilValue();
}

Value Compiler::visitAssignExpr(AssignExpr* expr) {
    expr->value->accept(*this);
    
    std::string_view name = expr->token.lexeme;
    int arg = resolveLocal(name);
    if (arg != -1) {
        emitOp(OpCode::SetLocal);
        emitByte(static_cast<uint8_t>(arg));
    } else if ((arg = resolveUpvalue(name)) != -1) {
        emitOp(OpCode::SetUpvalue);
        emitByte(static_cast<uint8_t>(arg));
    } else {
        emitOp(OpCode::SetGlobal);
        auto sv = StringPool::intern(name);
        emitByte(makeConstant(stringValue(sv.data())));
    }
    return nilValue();
}

Value Compiler::visitCompoundAssignExpr(CompoundAssignExpr* expr) {
    // Load current variable value
    int local = resolveLocal(expr->name);
    int upv = -1;
    if (local != -1) {
        emitOp(OpCode::GetLocal);
        emitByte(static_cast<uint8_t>(local));
    } else if ((upv = resolveUpvalue(expr->name)) != -1) {
        emitOp(OpCode::GetUpvalue);
        emitByte(static_cast<uint8_t>(upv));
    } else {
        emitOp(OpCode::GetGlobal);
        auto sv = StringPool::intern(expr->name);
        emitByte(makeConstant(stringValue(sv.data())));
    }
    // Operand
    expr->value->accept(*this);
    // Operation
    switch (expr->op.type) {
        case TokenType::PlusEqual:        emitOp(OpCode::Add); break;
        case TokenType::MinusEqual:       emitOp(OpCode::Subtract); break;
        case TokenType::StarEqual:        emitOp(OpCode::Multiply); break;
        case TokenType::SlashEqual:       emitOp(OpCode::Divide); break;
        case TokenType::BitAndEqual:      emitOp(OpCode::BitAnd); break;
        case TokenType::BitOrEqual:       emitOp(OpCode::BitOr); break;
        case TokenType::BitXorEqual:      emitOp(OpCode::BitXor); break;
        case TokenType::ShiftLeftEqual:   emitOp(OpCode::ShiftLeft); break;
        case TokenType::ShiftRightEqual:  emitOp(OpCode::ShiftRight); break;
        default: break;
    }
    // Store back
    if (local != -1) {
        emitOp(OpCode::SetLocal);
        emitByte(static_cast<uint8_t>(local));
    } else if (upv != -1) {
        emitOp(OpCode::SetUpvalue);
        emitByte(static_cast<uint8_t>(upv));
    } else {
        emitOp(OpCode::SetGlobal);
        auto sv = StringPool::intern(expr->name);
        emitByte(makeConstant(stringValue(sv.data())));
    }
    return nilValue();
}
Value Compiler::visitCompoundMemberAssignExpr(CompoundMemberAssignExpr* expr) {
    beginScope();
    std::string tmpObj = std::string("$tmp_o_") + std::to_string(reinterpret_cast<std::uintptr_t>(expr));
    addLocal(tmpObj);
    int objSlot = resolveLocal(tmpObj);
    emitOp(OpCode::Nil);
    expr->object->accept(*this);
    emitOp(OpCode::SetLocal);
    emitByte(static_cast<uint8_t>(objSlot));
    emitOp(OpCode::Pop);
    std::string tmpRhs = std::string("$tmp_rhs_") + std::to_string(reinterpret_cast<std::uintptr_t>(expr));
    addLocal(tmpRhs);
    int rhsSlot = resolveLocal(tmpRhs);
    emitOp(OpCode::Nil);
    expr->value->accept(*this);
    emitOp(OpCode::SetLocal);
    emitByte(static_cast<uint8_t>(rhsSlot));
    emitOp(OpCode::Pop);
    auto svName = StringPool::intern(expr->member);
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(objSlot));
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(rhsSlot));
    emitOp(OpCode::EnsurePropertyDefault);
    emitByte(makeConstant(stringValue(svName.data())));
    switch (expr->op.type) {
        case TokenType::PlusEqual:        emitByte(0); break;
        case TokenType::MinusEqual:       emitByte(1); break;
        case TokenType::StarEqual:        emitByte(2); break;
        case TokenType::SlashEqual:       emitByte(3); break;
        case TokenType::BitAndEqual:      emitByte(4); break;
        case TokenType::BitOrEqual:       emitByte(5); break;
        case TokenType::BitXorEqual:      emitByte(6); break;
        case TokenType::ShiftLeftEqual:   emitByte(7); break;
        case TokenType::ShiftRightEqual:  emitByte(8); break;
        default:                           emitByte(255); break;
    }
    emitOp(OpCode::Pop);
    emitOp(OpCode::Pop);
    std::string tmpRes = std::string("$tmp_r_") + std::to_string(reinterpret_cast<std::uintptr_t>(expr));
    addLocal(tmpRes);
    int resSlot = resolveLocal(tmpRes);
    emitOp(OpCode::Nil);
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(objSlot));
    emitOp(OpCode::GetProperty);
    emitByte(makeConstant(stringValue(svName.data())));
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(rhsSlot));
    switch (expr->op.type) {
        case TokenType::PlusEqual:        emitOp(OpCode::Add); break;
        case TokenType::MinusEqual:       emitOp(OpCode::Subtract); break;
        case TokenType::StarEqual:        emitOp(OpCode::Multiply); break;
        case TokenType::SlashEqual:       emitOp(OpCode::Divide); break;
        case TokenType::BitAndEqual:      emitOp(OpCode::BitAnd); break;
        case TokenType::BitOrEqual:       emitOp(OpCode::BitOr); break;
        case TokenType::BitXorEqual:      emitOp(OpCode::BitXor); break;
        case TokenType::ShiftLeftEqual:   emitOp(OpCode::ShiftLeft); break;
        case TokenType::ShiftRightEqual:  emitOp(OpCode::ShiftRight); break;
        default: break;
    }
    emitOp(OpCode::SetLocal);
    emitByte(static_cast<uint8_t>(resSlot));
    emitOp(OpCode::Pop);
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(objSlot));
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(resSlot));
    emitOp(OpCode::SetProperty);
    emitByte(makeConstant(stringValue(svName.data())));
    endScope();
    return nilValue();
}
Value Compiler::visitCompoundIndexAssignExpr(CompoundIndexAssignExpr* expr) {
    beginScope();
    // Allocate locals and stash values once
    std::string tmpObj = std::string("$tmp_o_") + std::to_string(reinterpret_cast<std::uintptr_t>(expr));
    addLocal(tmpObj);
    int objSlot = resolveLocal(tmpObj);
    emitOp(OpCode::Nil);
    expr->object->accept(*this);
    emitOp(OpCode::SetLocal);
    emitByte(static_cast<uint8_t>(objSlot));
    emitOp(OpCode::Pop);
    std::string tmpIdx = std::string("$tmp_i_") + std::to_string(reinterpret_cast<std::uintptr_t>(expr));
    addLocal(tmpIdx);
    int idxSlot = resolveLocal(tmpIdx);
    emitOp(OpCode::Nil);
    expr->index->accept(*this);
    emitOp(OpCode::SetLocal);
    emitByte(static_cast<uint8_t>(idxSlot));
    emitOp(OpCode::Pop);
    std::string tmpRhs = std::string("$tmp_rhs_") + std::to_string(reinterpret_cast<std::uintptr_t>(expr));
    addLocal(tmpRhs);
    int rhsSlot = resolveLocal(tmpRhs);
    emitOp(OpCode::Nil);
    expr->value->accept(*this);
    emitOp(OpCode::SetLocal);
    emitByte(static_cast<uint8_t>(rhsSlot));
    emitOp(OpCode::Pop);
    // Ensure default for missing hash keys
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(objSlot));
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(idxSlot));
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(rhsSlot));
    emitOp(OpCode::EnsureIndexDefault);
    switch (expr->op.type) {
        case TokenType::PlusEqual:        emitByte(0); break;
        case TokenType::MinusEqual:       emitByte(1); break;
        case TokenType::StarEqual:        emitByte(2); break;
        case TokenType::SlashEqual:       emitByte(3); break;
        case TokenType::BitAndEqual:      emitByte(4); break;
        case TokenType::BitOrEqual:       emitByte(5); break;
        case TokenType::BitXorEqual:      emitByte(6); break;
        case TokenType::ShiftLeftEqual:   emitByte(7); break;
        case TokenType::ShiftRightEqual:  emitByte(8); break;
        default:                           emitByte(255); break;
    }
    // Clear the temporary stack triplet [obj, idx, rhs] used by EnsureIndexDefault
    emitOp(OpCode::Pop);
    emitOp(OpCode::Pop);
    emitOp(OpCode::Pop);
    // Allocate result slot before computing to ensure it resides below eval stack
    std::string tmpRes = std::string("$tmp_r_") + std::to_string(reinterpret_cast<std::uintptr_t>(expr));
    addLocal(tmpRes);
    int resSlot = resolveLocal(tmpRes);
    emitOp(OpCode::Nil);
    // Load current value at index, apply operation with RHS
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(objSlot));
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(idxSlot));
    emitOp(OpCode::GetIndex);
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(rhsSlot));
    switch (expr->op.type) {
        case TokenType::PlusEqual:        emitOp(OpCode::Add); break;
        case TokenType::MinusEqual:       emitOp(OpCode::Subtract); break;
        case TokenType::StarEqual:        emitOp(OpCode::Multiply); break;
        case TokenType::SlashEqual:       emitOp(OpCode::Divide); break;
        case TokenType::BitAndEqual:      emitOp(OpCode::BitAnd); break;
        case TokenType::BitOrEqual:       emitOp(OpCode::BitOr); break;
        case TokenType::BitXorEqual:      emitOp(OpCode::BitXor); break;
        case TokenType::ShiftLeftEqual:   emitOp(OpCode::ShiftLeft); break;
        case TokenType::ShiftRightEqual:  emitOp(OpCode::ShiftRight); break;
        default: break;
    }
    emitOp(OpCode::SetLocal);
    emitByte(static_cast<uint8_t>(resSlot));
    emitOp(OpCode::Pop);
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(objSlot));
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(idxSlot));
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(resSlot));
    emitOp(OpCode::SetIndex);
    endScope();
    return nilValue();
}
Value Compiler::visitUpdateExpr(UpdateExpr* expr) {
    int slot = resolveLocal(expr->name);
    if (slot != -1) {
        emitOp(OpCode::GetLocal);
        emitByte(static_cast<uint8_t>(slot));
        emitOp(OpCode::Constant);
        emitByte(makeConstant(numberToValue(1.0)));
        if (expr->op.type == TokenType::PlusPlus) {
            emitOp(OpCode::Add);
        } else {
            emitOp(OpCode::Subtract);
        }
        emitOp(OpCode::SetLocal);
        emitByte(static_cast<uint8_t>(slot));
        return nilValue();
    }
    return nilValue();
}
Value Compiler::visitUpdateMemberExpr(UpdateMemberExpr* expr) { return nilValue(); }
Value Compiler::visitUpdateIndexExpr(UpdateIndexExpr* expr) { return nilValue(); }
Value Compiler::visitTernaryExpr(TernaryExpr* expr) {
    expr->condition->accept(*this);
    int elseJump = emitJump(OpCode::JumpIfFalse);
    emitOp(OpCode::Pop);
    expr->thenBranch->accept(*this);
    int endJump = emitJump(OpCode::Jump);
    patchJump(elseJump);
    emitOp(OpCode::Pop);
    expr->elseBranch->accept(*this);
    patchJump(endJump);
    return nilValue();
}
Value Compiler::visitArrayExpr(ArrayExpr* expr) { return nilValue(); }
Value Compiler::visitIndexExpr(IndexExpr* expr) {
    expr->object->accept(*this);
    expr->index->accept(*this);
    emitOp(OpCode::GetIndex);
    return nilValue();
}
Value Compiler::visitIndexAssignExpr(IndexAssignExpr* expr) {
    expr->object->accept(*this);
    expr->index->accept(*this);
    expr->value->accept(*this);
    emitOp(OpCode::SetIndex);
    return nilValue();
}
Value Compiler::visitHashMapExpr(HashMapExpr* expr) { return nilValue(); }
Value Compiler::visitMemberExpr(MemberExpr* expr) {
    expr->object->accept(*this);
    emitOp(OpCode::GetProperty);
    auto sv = StringPool::intern(expr->member);
    emitByte(makeConstant(stringValue(sv.data())));
    return nilValue();
}
Value Compiler::visitSetExpr(SetExpr* expr) {
    expr->object->accept(*this);
    expr->value->accept(*this);
    emitOp(OpCode::SetProperty);
    auto sv = StringPool::intern(expr->member);
    emitByte(makeConstant(stringValue(sv.data())));
    return nilValue();
}
Value Compiler::visitThisExpr(ThisExpr* expr) { return nilValue(); }
Value Compiler::visitSuperExpr(SuperExpr* expr) { return nilValue(); }
Value Compiler::visitFunctionExpr(FunctionExpr* expr) {
    Compiler functionCompiler(this);
    functionCompiler.chunk_ = std::make_unique<Chunk>();
    functionCompiler.beginScope();
    functionCompiler.addLocal("");

    for (const auto& param : expr->parameters) {
        functionCompiler.addLocal(param);
    }

    for (const auto& stmt : expr->body) {
        functionCompiler.currentLine_ = stmt->token.line;
        stmt->accept(functionCompiler);
    }

    functionCompiler.emitOp(OpCode::Nil);
    functionCompiler.emitOp(OpCode::Return);

    auto function = std::make_shared<VMFunction>();
    function->name = "<lambda>";
    function->arity = static_cast<int>(expr->parameters.size());
    function->upvalueCount = static_cast<int>(functionCompiler.upvalues_.size());
    function->chunk = std::move(functionCompiler.chunk_);

    emitOp(OpCode::Closure);
    emitByte(makeConstant(vmFunctionValue(function)));

    for (const auto& upvalue : functionCompiler.upvalues_) {
        emitByte(upvalue.isLocal ? 1 : 0);
        emitByte(upvalue.index);
    }

    return nilValue();
}

// StmtVisitor implementation

void Compiler::visitExprStmt(ExprStmt* stmt) {
    stmt->expr->accept(*this);
    emitOp(OpCode::Pop);
}

void Compiler::visitPrintStmt(PrintStmt* stmt) {
    stmt->expr->accept(*this);
    emitOp(OpCode::Print);
}

void Compiler::visitLetStmt(LetStmt* stmt) {
    std::string_view name = stmt->token.lexeme;
    if (scopeDepth_ > 0) {
        addLocal(name);
        int slot = resolveLocal(name);
        emitOp(OpCode::Nil);
        if (stmt->initializer) {
            stmt->initializer->accept(*this);
            emitOp(OpCode::SetLocal);
            emitByte(static_cast<uint8_t>(slot));
            emitOp(OpCode::Pop);
        }
    } else {
        if (stmt->initializer) {
            stmt->initializer->accept(*this);
        } else {
            emitOp(OpCode::Nil);
        }
        emitOp(OpCode::DefineGlobal);
        auto sv = StringPool::intern(name);
        emitByte(makeConstant(stringValue(sv.data())));
    }
}

void Compiler::visitBlockStmt(BlockStmt* stmt) {
    beginScope();
    for (const auto& s : stmt->statements) {
        s->accept(*this);
    }
    endScope();
}

void Compiler::visitIfStmt(IfStmt* stmt) {
    stmt->condition->accept(*this);
    int thenJump = emitJump(OpCode::JumpIfFalse);
    emitOp(OpCode::Pop); // Pop condition
    
    stmt->thenBranch->accept(*this);
    
    int elseJump = emitJump(OpCode::Jump);
    
    patchJump(thenJump);
    emitOp(OpCode::Pop); // Pop condition
    
    if (stmt->elseBranch) {
        stmt->elseBranch->accept(*this);
    }
    patchJump(elseJump);
}

void Compiler::visitWhileStmt(WhileStmt* stmt) {
    int loopStart = static_cast<int>(chunk_->size());
    stmt->condition->accept(*this);
    
    int exitJump = emitJump(OpCode::JumpIfFalse);
    emitOp(OpCode::Pop);
    
    stmt->body->accept(*this);
    emitLoop(loopStart);
    
    patchJump(exitJump);
    emitOp(OpCode::Pop);
}
void Compiler::visitRunUntilStmt(RunUntilStmt*) {}
void Compiler::visitForStmt(ForStmt* stmt) {
    beginScope();
    if (stmt->initializer) {
        stmt->initializer->accept(*this);
    }
    int loopStart = static_cast<int>(chunk_->size());
    int exitJump = -1;
    if (stmt->condition) {
        stmt->condition->accept(*this);
        exitJump = emitJump(OpCode::JumpIfFalse);
        emitOp(OpCode::Pop);
    }
    stmt->body->accept(*this);
    if (stmt->increment) {
        stmt->increment->accept(*this);
        emitOp(OpCode::Pop);
    }
    emitLoop(loopStart);
    if (exitJump != -1) {
        patchJump(exitJump);
        emitOp(OpCode::Pop);
    }
    endScope();
}
void Compiler::visitFnStmt(FnStmt* stmt) {
    Compiler functionCompiler(this);
    functionCompiler.chunk_ = std::make_unique<Chunk>();
    functionCompiler.beginScope();
    functionCompiler.addLocal(stmt->name);

    for (const auto& param : stmt->params) {
        functionCompiler.addLocal(param.name);
    }

    for (const auto& bodyStmt : stmt->body) {
        functionCompiler.currentLine_ = bodyStmt->token.line;
        bodyStmt->accept(functionCompiler);
    }

    functionCompiler.emitOp(OpCode::Nil);
    functionCompiler.emitOp(OpCode::Return);

    auto function = std::make_shared<VMFunction>();
    function->name = stmt->name;
    function->arity = static_cast<int>(stmt->params.size());
    function->upvalueCount = static_cast<int>(functionCompiler.upvalues_.size());
    function->chunk = std::move(functionCompiler.chunk_);

    emitOp(OpCode::Closure);
    emitByte(makeConstant(vmFunctionValue(function)));

    for (const auto& upvalue : functionCompiler.upvalues_) {
        emitByte(upvalue.isLocal ? 1 : 0);
        emitByte(upvalue.index);
    }

    std::string_view name = stmt->token.lexeme;
    if (scopeDepth_ > 0) {
        addLocal(name);
    } else {
        emitOp(OpCode::DefineGlobal);
        auto sv = StringPool::intern(name);
        emitByte(makeConstant(stringValue(sv.data())));
    }
}
void Compiler::visitReturnStmt(ReturnStmt* stmt) {
    if (!stmt->values.empty()) {
        stmt->values[0]->accept(*this);
    } else {
        emitOp(OpCode::Nil);
    }
    emitOp(OpCode::Return);
}
void Compiler::visitBreakStmt(BreakStmt*) {}
void Compiler::visitContinueStmt(ContinueStmt*) {}
void Compiler::visitTryStmt(TryStmt*) {}
void Compiler::visitThrowStmt(ThrowStmt*) {}
void Compiler::visitImportStmt(ImportStmt*) {}
void Compiler::visitClassStmt(ClassStmt*) {}
void Compiler::visitSwitchStmt(SwitchStmt*) {}

// ---- New ExprVisitor stubs ----
Value Compiler::visitFStringExpr(FStringExpr* expr) {
    // Concatenate all segments at runtime
    bool first = true;
    for (auto& seg : expr->segments) {
        if (seg.isExpr) {
            seg.expr->accept(*this);
            emitOp(OpCode::ToString);
        } else {
            auto sv = StringPool::intern(seg.text);
            emitConstant(stringValue(sv.data()));
        }
        if (!first) emitOp(OpCode::Add);
        first = false;
    }
    if (first) emitConstant(stringValue(StringPool::intern("").data()));
    return nilValue();
}
Value Compiler::visitTemplateExpr(TemplateExpr* expr) {
    bool first = true;
    for (auto& seg : expr->segments) {
        if (seg.isExpr) {
            seg.expr->accept(*this);
            emitOp(OpCode::ToString);
        } else {
            auto sv = StringPool::intern(seg.text);
            emitConstant(stringValue(sv.data()));
        }
        if (!first) emitOp(OpCode::Add);
        first = false;
    }
    if (first) emitConstant(stringValue(StringPool::intern("").data()));
    return nilValue();
}
Value Compiler::visitSpreadExpr(SpreadExpr* expr) {
    expr->expr->accept(*this);
    emitOp(OpCode::Spread);
    return nilValue();
}
Value Compiler::visitOptionalChainExpr(OptionalChainExpr* expr) {
    expr->object->accept(*this);
    // Emit nil-check: if nil, short-circuit to nil
    int nilJump = emitJump(OpCode::JumpIfNil);
    switch (expr->kind) {
        case OptionalChainExpr::Kind::Member: {
            auto sv = StringPool::intern(expr->member);
            emitOp(OpCode::GetProperty);
            emitByte(makeConstant(stringValue(sv.data())));
            break;
        }
        case OptionalChainExpr::Kind::Index:
            expr->index->accept(*this);
            emitOp(OpCode::GetIndex);
            break;
        case OptionalChainExpr::Kind::Call:
            for (auto& arg : expr->args) arg->accept(*this);
            emitOp(OpCode::Call);
            emitByte(static_cast<uint8_t>(expr->args.size()));
            break;
    }
    int endJump = emitJump(OpCode::Jump);
    patchJump(nilJump);
    emitOp(OpCode::Pop);
    emitOp(OpCode::Nil);
    patchJump(endJump);
    return nilValue();
}
Value Compiler::visitNullCoalesceExpr(NullCoalesceExpr* expr) {
    expr->left->accept(*this);
    int notNilJump = emitJump(OpCode::JumpIfNotNil);
    emitOp(OpCode::Pop);
    expr->right->accept(*this);
    patchJump(notNilJump);
    return nilValue();
}
Value Compiler::visitPipeExpr(PipeExpr* expr) {
    expr->right->accept(*this); // callee
    expr->left->accept(*this);  // single argument
    emitOp(OpCode::Call);
    emitByte(1);
    return nilValue();
}
Value Compiler::visitAwaitExpr(AwaitExpr* expr) {
    expr->expr->accept(*this);
    emitOp(OpCode::Await);
    return nilValue();
}
Value Compiler::visitYieldExpr(YieldExpr* expr) {
    if (expr->expr) expr->expr->accept(*this);
    else emitOp(OpCode::Nil);
    emitOp(OpCode::Yield);
    return nilValue();
}
Value Compiler::visitMatchExpr(MatchExpr* expr) {
    expr->subject->accept(*this);
    // Simple linear match: for each arm emit compare + jump
    std::vector<int> endJumps;
    for (auto& arm : expr->arms) {
        if (arm.isDefault) {
            emitOp(OpCode::Pop); // pop subject
            if (arm.bodyExpr) arm.bodyExpr->accept(*this);
            else emitOp(OpCode::Nil);
        } else {
            for (auto& pat : arm.patterns) {
                emitOp(OpCode::Dup);
                pat->accept(*this);
                emitOp(OpCode::Equal);
                int matchJump = emitJump(OpCode::JumpIfTrue);
                emitOp(OpCode::Pop); // pop Equal result
                // try next pattern
                int skipJump = emitJump(OpCode::Jump);
                patchJump(matchJump);
                emitOp(OpCode::Pop); // pop Equal result
                emitOp(OpCode::Pop); // pop subject dup
                if (arm.bodyExpr) arm.bodyExpr->accept(*this);
                else emitOp(OpCode::Nil);
                endJumps.push_back(emitJump(OpCode::Jump));
                patchJump(skipJump);
            }
        }
    }
    // fallthrough: no match → nil
    emitOp(OpCode::Pop);
    emitOp(OpCode::Nil);
    for (int j : endJumps) patchJump(j);
    return nilValue();
}
Value Compiler::visitComprehensionExpr(ComprehensionExpr* expr) {
    // Emit as: let $arr = []; for varName in iterable { if cond { $arr.push(body) } }
    emitOp(OpCode::NewArray);
    // Store array in a temp local
    std::string tmpArr = std::string("$comp_") + std::to_string(reinterpret_cast<std::uintptr_t>(expr));
    addLocal(tmpArr);
    int arrSlot = resolveLocal(tmpArr);
    emitOp(OpCode::SetLocal);
    emitByte(static_cast<uint8_t>(arrSlot));
    emitOp(OpCode::Pop);
    // Iterate
    expr->iterable->accept(*this);
    emitOp(OpCode::GetIter);
    std::string tmpIter = std::string("$iter_") + std::to_string(reinterpret_cast<std::uintptr_t>(expr));
    addLocal(tmpIter);
    int iterSlot = resolveLocal(tmpIter);
    emitOp(OpCode::SetLocal);
    emitByte(static_cast<uint8_t>(iterSlot));
    emitOp(OpCode::Pop);
    int loopStart = static_cast<int>(chunk_->size());
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(iterSlot));
    emitOp(OpCode::IterNext);
    int exitJump = emitJump(OpCode::JumpIfFalse);
    emitOp(OpCode::Pop);
    addLocal(expr->varName);
    int varSlot = resolveLocal(expr->varName);
    emitOp(OpCode::SetLocal);
    emitByte(static_cast<uint8_t>(varSlot));
    emitOp(OpCode::Pop);
    if (expr->condition) {
        expr->condition->accept(*this);
        int skipJump = emitJump(OpCode::JumpIfFalse);
        emitOp(OpCode::Pop);
        emitOp(OpCode::GetLocal);
        emitByte(static_cast<uint8_t>(arrSlot));
        expr->body->accept(*this);
        emitOp(OpCode::ArrayPush);
        emitOp(OpCode::Pop);
        int afterJump = emitJump(OpCode::Jump);
        patchJump(skipJump);
        emitOp(OpCode::Pop);
        patchJump(afterJump);
    } else {
        emitOp(OpCode::GetLocal);
        emitByte(static_cast<uint8_t>(arrSlot));
        expr->body->accept(*this);
        emitOp(OpCode::ArrayPush);
        emitOp(OpCode::Pop);
    }
    emitLoop(loopStart);
    patchJump(exitJump);
    emitOp(OpCode::Pop);
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(arrSlot));
    return nilValue();
}
Value Compiler::visitDestructureArrayExpr(DestructureArrayExpr* expr) {
    expr->value->accept(*this);
    int idx = 0;
    for (auto& elem : expr->elements) {
        if (elem.isRest) {
            emitOp(OpCode::ArraySlice);
            emitByte(static_cast<uint8_t>(idx));
        } else {
            emitOp(OpCode::Dup);
            emitConstant(numberToValue(static_cast<double>(idx)));
            emitOp(OpCode::GetIndex);
        }
        if (scopeDepth_ > 0) {
            addLocal(elem.name);
        } else {
            emitOp(OpCode::DefineGlobal);
            auto sv = StringPool::intern(elem.name);
            emitByte(makeConstant(stringValue(sv.data())));
        }
        idx++;
    }
    emitOp(OpCode::Pop); // pop original array
    return nilValue();
}
Value Compiler::visitDestructureObjectExpr(DestructureObjectExpr* expr) {
    expr->value->accept(*this);
    for (auto& prop : expr->properties) {
        if (prop.isRest) continue; // TODO: rest object
        emitOp(OpCode::Dup);
        auto sv = StringPool::intern(prop.key);
        emitOp(OpCode::GetProperty);
        emitByte(makeConstant(stringValue(sv.data())));
        std::string bindName = prop.alias.empty() ? prop.key : prop.alias;
        if (scopeDepth_ > 0) {
            addLocal(bindName);
        } else {
            emitOp(OpCode::DefineGlobal);
            auto bsv = StringPool::intern(bindName);
            emitByte(makeConstant(stringValue(bsv.data())));
        }
    }
    emitOp(OpCode::Pop); // pop original object
    return nilValue();
}
Value Compiler::visitTypeAnnotationExpr(TypeAnnotationExpr* expr) {
    // Type annotations are erased at runtime
    return expr->expr->accept(*this);
}
Value Compiler::visitNewExpr(NewExpr* expr) {
    expr->callee->accept(*this);
    for (auto& arg : expr->arguments) arg->accept(*this);
    emitOp(OpCode::Construct);
    emitByte(static_cast<uint8_t>(expr->arguments.size()));
    return nilValue();
}
Value Compiler::visitMetaExpr(MetaExpr* expr) {
    expr->object->accept(*this);
    if (expr->metatable) {
        expr->metatable->accept(*this);
        emitOp(OpCode::SetMeta);
    } else {
        emitOp(OpCode::GetMeta);
    }
    return nilValue();
}

// ---- New StmtVisitor stubs ----
void Compiler::visitConstStmt(ConstStmt* stmt) {
    // Treat const like let (enforcement is semantic, not bytecode-level)
    std::string_view name = stmt->token.lexeme;
    if (scopeDepth_ > 0) {
        addLocal(name);
        int slot = resolveLocal(name);
        emitOp(OpCode::Nil);
        if (stmt->initializer) {
            stmt->initializer->accept(*this);
            emitOp(OpCode::SetLocal);
            emitByte(static_cast<uint8_t>(slot));
            emitOp(OpCode::Pop);
        }
    } else {
        if (stmt->initializer) stmt->initializer->accept(*this);
        else emitOp(OpCode::Nil);
        emitOp(OpCode::DefineGlobal);
        auto sv = StringPool::intern(name);
        emitByte(makeConstant(stringValue(sv.data())));
    }
}
void Compiler::visitEnumStmt(EnumStmt* stmt) {
    // Emit enum as a hash-map constant
    emitOp(OpCode::NewHashMap);
    double idx = 0.0;
    for (auto& mem : stmt->members) {
        auto sv = StringPool::intern(mem.name);
        emitConstant(stringValue(sv.data()));
        if (mem.value) mem.value->accept(*this);
        else emitConstant(numberToValue(idx));
        emitOp(OpCode::HashMapSet);
        idx += 1.0;
    }
    emitOp(OpCode::DefineGlobal);
    auto sv = StringPool::intern(stmt->name);
    emitByte(makeConstant(stringValue(sv.data())));
}
void Compiler::visitInterfaceStmt(InterfaceStmt*) {
    // Interfaces are erased at runtime (structural typing)
}
void Compiler::visitForOfStmt(ForOfStmt* stmt) {
    beginScope();
    stmt->iterable->accept(*this);
    emitOp(OpCode::GetIter);
    std::string tmpIter = std::string("$foriter_") + std::to_string(reinterpret_cast<std::uintptr_t>(stmt));
    addLocal(tmpIter);
    int iterSlot = resolveLocal(tmpIter);
    emitOp(OpCode::SetLocal);
    emitByte(static_cast<uint8_t>(iterSlot));
    emitOp(OpCode::Pop);
    int loopStart = static_cast<int>(chunk_->size());
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(iterSlot));
    emitOp(OpCode::IterNext);
    int exitJump = emitJump(OpCode::JumpIfFalse);
    emitOp(OpCode::Pop);
    addLocal(stmt->varName);
    int varSlot = resolveLocal(stmt->varName);
    emitOp(OpCode::SetLocal);
    emitByte(static_cast<uint8_t>(varSlot));
    emitOp(OpCode::Pop);
    stmt->body->accept(*this);
    emitLoop(loopStart);
    patchJump(exitJump);
    emitOp(OpCode::Pop);
    endScope();
}
void Compiler::visitForInStmt(ForInStmt* stmt) {
    // for (key in obj) — iterate over keys
    beginScope();
    stmt->object->accept(*this);
    emitOp(OpCode::GetIter);
    std::string tmpIter = std::string("$finiter_") + std::to_string(reinterpret_cast<std::uintptr_t>(stmt));
    addLocal(tmpIter);
    int iterSlot = resolveLocal(tmpIter);
    emitOp(OpCode::SetLocal);
    emitByte(static_cast<uint8_t>(iterSlot));
    emitOp(OpCode::Pop);
    int loopStart = static_cast<int>(chunk_->size());
    emitOp(OpCode::GetLocal);
    emitByte(static_cast<uint8_t>(iterSlot));
    emitOp(OpCode::IterNext);
    int exitJump = emitJump(OpCode::JumpIfFalse);
    emitOp(OpCode::Pop);
    addLocal(stmt->varName);
    int varSlot = resolveLocal(stmt->varName);
    emitOp(OpCode::SetLocal);
    emitByte(static_cast<uint8_t>(varSlot));
    emitOp(OpCode::Pop);
    stmt->body->accept(*this);
    emitLoop(loopStart);
    patchJump(exitJump);
    emitOp(OpCode::Pop);
    endScope();
}
void Compiler::visitDeferStmt(DeferStmt* stmt) {
    // Defer: execute body at end of scope — emit as-is for now (no stack unwinding)
    stmt->body->accept(*this);
}
void Compiler::visitAsyncFnStmt(AsyncFnStmt* stmt) {
    // Async fn: compile the inner fn normally; async semantics handled at runtime
    if (stmt->fn) stmt->fn->accept(*this);
}
void Compiler::visitWithStmt(WithStmt* stmt) {
    beginScope();
    stmt->resource->accept(*this);
    addLocal(stmt->varName);
    int slot = resolveLocal(stmt->varName);
    emitOp(OpCode::SetLocal);
    emitByte(static_cast<uint8_t>(slot));
    emitOp(OpCode::Pop);
    stmt->body->accept(*this);
    endScope();
}
void Compiler::visitLabeledStmt(LabeledStmt* stmt) {
    stmt->body->accept(*this);
}
void Compiler::visitMultiLetStmt(MultiLetStmt* stmt) {
    // let [a, b] = expr  — compile initializer then destructure
    if (stmt->initializer) stmt->initializer->accept(*this);
    else emitOp(OpCode::Nil);
    for (size_t i = 0; i < stmt->names.size(); ++i) {
        emitOp(OpCode::Dup);
        emitConstant(numberToValue(static_cast<double>(i)));
        emitOp(OpCode::GetIndex);
        std::string_view name = StringPool::intern(stmt->names[i]);
        if (scopeDepth_ > 0) {
            addLocal(name);
        } else {
            emitOp(OpCode::DefineGlobal);
            emitByte(makeConstant(stringValue(name.data())));
        }
    }
    emitOp(OpCode::Pop); // pop original value
}
void Compiler::visitExportStmt(ExportStmt* stmt) {
    // Export: no-op at bytecode level (module system handles it)
    (void)stmt;
}
void Compiler::visitDecoratorStmt(DecoratorStmt* stmt) {
    // Compile the target first, then apply decorators in order
    if (stmt->target) stmt->target->accept(*this);
}

// Private helpers

void Compiler::emitByte(uint8_t byte) {
    chunk_->write(byte, currentLine_);
}

void Compiler::emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

void Compiler::emitOp(OpCode op) {
    emitByte(static_cast<uint8_t>(op));
}

void Compiler::emitConstant(Value value) {
    emitOp(OpCode::Constant);
    emitByte(makeConstant(value));
}

uint8_t Compiler::makeConstant(Value value) {
    int index = chunk_->addConstant(value);
    if (index > 255) {
        error(Token(TokenType::Error, "", currentLine_), "Too many constants in one chunk.");
        return 0;
    }
    return static_cast<uint8_t>(index);
}

int Compiler::emitJump(OpCode instruction) {
    emitOp(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return static_cast<int>(chunk_->size() - 2);
}

void Compiler::patchJump(int offset) {
    // -2 to adjust for the bytecode for the jump offset itself.
    int jump = static_cast<int>(chunk_->size() - offset - 2);

    if (jump > UINT16_MAX) {
        error(Token(TokenType::Error, "", currentLine_), "Too much code to jump over.");
    }

    const_cast<std::vector<uint8_t>&>(chunk_->code())[offset] = (jump >> 8) & 0xff;
    const_cast<std::vector<uint8_t>&>(chunk_->code())[offset + 1] = jump & 0xff;
}

void Compiler::emitLoop(int loopStart) {
    emitOp(OpCode::Loop);

    int offset = static_cast<int>(chunk_->size() - loopStart + 2);
    if (offset > UINT16_MAX) error(Token(TokenType::Error, "", currentLine_), "Loop body too large.");

    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

void Compiler::beginScope() {
    scopeDepth_++;
}

void Compiler::endScope() {
    scopeDepth_--;
    
    while (!locals_.empty() && locals_.back().depth > scopeDepth_) {
        if (locals_.back().isCaptured) {
            emitOp(OpCode::CloseUpvalue);
        } else {
            emitOp(OpCode::Pop);
        }
        locals_.pop_back();
    }
}

void Compiler::addLocal(std::string_view name) {
    if (locals_.size() >= 256) {
        error(Token(TokenType::Error, "", currentLine_), "Too many local variables in function.");
        return;
    }

    name = StringPool::intern(name);

    // Check if variable already exists in same scope
    for (int i = static_cast<int>(locals_.size()) - 1; i >= 0; i--) {
        Local& local = locals_[i];
        if (local.depth != -1 && local.depth < scopeDepth_) break;
        
        if (local.name.data() == name.data()) {
            error(Token(TokenType::Error, "", currentLine_), "Already a variable with this name in this scope.");
        }
    }

    Local local;
    local.name = name;
    local.depth = scopeDepth_;
    local.isCaptured = false;
    locals_.push_back(local);
}

int Compiler::resolveLocal(std::string_view name) {
    name = StringPool::intern(name);
    for (int i = static_cast<int>(locals_.size()) - 1; i >= 0; i--) {
        if (locals_[i].name.data() == name.data()) {
            return i;
        }
    }
    return -1;
}

int Compiler::resolveUpvalue(std::string_view name) {
    if (!enclosing_) return -1;

    int local = enclosing_->resolveLocal(name);
    if (local != -1) {
        enclosing_->locals_[local].isCaptured = true;
        return addUpvalue(static_cast<uint8_t>(local), true);
    }

    int upvalue = enclosing_->resolveUpvalue(name);
    if (upvalue != -1) {
        return addUpvalue(static_cast<uint8_t>(upvalue), false);
    }

    return -1;
}

int Compiler::addUpvalue(uint8_t index, bool isLocal) {
    for (size_t i = 0; i < upvalues_.size(); i++) {
        if (upvalues_[i].index == index && upvalues_[i].isLocal == isLocal) {
            return static_cast<int>(i);
        }
    }

    if (upvalues_.size() >= 256) {
        error(Token(TokenType::Error, "", currentLine_), "Too many closure variables in function.");
        return 0;
    }

    upvalues_.push_back(Upvalue{index, isLocal});
    return static_cast<int>(upvalues_.size() - 1);
}

void Compiler::error(Token token, const std::string& message) {
    std::cerr << "[line " << token.line << "] Error: " << message << std::endl;
}

} // namespace claw
