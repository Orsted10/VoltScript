#include "interpreter.h"
#include "callable.h"
#include "stmt.h"
#include "ast.h"
#include "value.h"
#include "environment.h"
#include "features/array.h"
#include "features/hashmap.h"
#include "features/class.h"
#include "features/string_pool.h"
#include "interpreter/natives/native_math.h"
#include "interpreter/natives/native_gamemath.h"
#include "interpreter/natives/native_collections.h"
#include "interpreter/natives/native_regex.h"
#include "interpreter/natives/native_os.h"
#include "interpreter/natives/native_events.h"
#include "interpreter/natives/native_ecs.h"
#include "interpreter/natives/native_string.h"
#include "interpreter/natives/native_coroutine.h"
#include "interpreter/natives/native_ffi.h"
#include "interpreter/natives/native_http.h"
#include "interpreter/natives/native_thread.h"
#include "interpreter/natives/native_gfx.h"
#include "interpreter/natives/native_types.h"
#include "interpreter/natives/native_package.h"
#include "interpreter/natives/native_database.h"
#include "interpreter/natives/native_crypto.h"
#include "interpreter/natives/native_ai.h"
#include "interpreter/natives/native_websocket.h"
#include "interpreter/natives/native_audio.h"
#include "interpreter/natives/native_blockchain.h"
#include "interpreter/natives/native_vr.h"
#include "interpreter/natives/native_quantum.h"
#include "interpreter/natives/native_robotics.h"
#include "interpreter/natives/native_space.h"
#include "interpreter/natives/native_biotech.h"
#include "interpreter/natives/native_nanotech.h"
#include "interpreter/natives/native_climate.h"
#include "interpreter/natives/native_medical.h"
#include "interpreter/natives/native_physics.h"
#include "interpreter/natives/native_mathematics.h"
#include "interpreter/natives/native_optimization.h"
#include "interpreter/natives/native_cybersecurity.h"
#include "interpreter/natives/native_augmented_reality.h"
#include "interpreter/natives/native_blockchain_advanced.h"
#include "interpreter/natives/native_quantum_computing_advanced.h"
// Must undef Windows macros that clash with our identifiers before including coroutine.h
#ifdef _WIN32
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#endif
#include "features/coroutine.h"
#ifdef _WIN32
// Windows defines Yield as SwitchToThread — undefine so our code compiles
#  ifdef Yield
#    undef Yield
#  endif
#endif

#include "interpreter/natives/native_array.h"
#include "interpreter/natives/native_io.h"
#include "interpreter/natives/native_time.h"
#include "interpreter/natives/native_json.h"
#include "interpreter/natives/native_security.h"
#include "interpreter/gc_alloc.h"
#include <memory>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdio>
#include <algorithm>
#include <limits>
#include "observability/profiler.h"

namespace claw {

thread_local Interpreter* tl_current_interpreter = nullptr;


Interpreter::Interpreter()
    : environment_(std::make_shared<Environment>()),
      globals_(environment_) {
    // Set up all the built-in functions that come with VoltScript
    const char* benchMode = std::getenv("CLAW_BENCHMARK_MODE");
    if (!benchMode) benchMode = std::getenv("VOLT_BENCHMARK_MODE");
    if (benchMode && std::string(benchMode) == "1") {
        gcSetBenchmarkMode(true);
    } else {
        gcSetBenchmarkMode(false);
    }
    const char* envProf = std::getenv("CLAW_PROFILE");
    if (!envProf) envProf = std::getenv("VOLT_PROFILE");
    const char* envHz = std::getenv("CLAW_PROFILE_HZ");
    if (!envHz) envHz = std::getenv("VOLT_PROFILE_HZ");
    if (envProf && *envProf && !profilerEnabled()) {
        int hz = 100;
        if (envHz && *envHz) {
            try { hz = std::stoi(envHz); } catch (...) {}
        }
        profilerSetCurrentInterpreter(this);
        profilerStart(hz);
    }
    defineNatives();
}

Interpreter::~Interpreter() {
    // Destructor - no cleanup needed for now
}

void Interpreter::reset() {
    environment_ = std::make_shared<Environment>();
    globals_ = environment_;
    defineNatives();
}

// Register native functions (built into the language)
void Interpreter::defineNatives() {
    registerNativeTime(globals_);
    
    registerNativeArray(globals_, *this);
    
    // num(value) - convert to number
    globals_->define("num", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (isNumber(args[0])) return args[0];
            if (isString(args[0])) {
                try {
                    return numberToValue(std::stod(asString(args[0])));
                } catch (...) {
                    throw std::runtime_error("E2001: Cannot convert string to number: " + asString(args[0]));
                }
            }
            if (isBool(args[0])) {
                return numberToValue(asBool(args[0]) ? 1.0 : 0.0);
            }
            throw std::runtime_error("E2001: Cannot convert to number");
        },
        "num"
    ));
    
    registerNativeIO(globals_);
    
    // Register string native functions
    registerNativeString(globals_);
    
    // ==================== BASIC MATH STUFF (NEW FOR v0.7.2) ====================
    
    // Register math native functions
    registerNativeMath(globals_);
    registerNativeGameMath(globals_);
    registerNativeCollections(globals_);
    registerNativeRegex(globals_);
    registerNativeOS(globals_);
    registerNativeEvents(globals_);
    registerNativeECS(*globals_);
    registerNativeCoroutine(globals_);
    registerNativeFFI(globals_);
    registerNativeHTTP(globals_);
    registerNativeThread(globals_);
    registerNativeGFX(globals_);
    registerNativeTypes(globals_);
    registerNativePackage(globals_);
    registerNativeDatabase(globals_);
    registerNativeCrypto(globals_);
    registerNativeAI(globals_);
    registerNativeWebSocket(globals_);
    registerNativeAudio(globals_);
    registerNativeBlockchain(globals_);
    registerNativeVR(globals_);
    registerNativeQuantum(globals_);
    registerNativeRobotics(globals_);
    registerNativeSpace(globals_);
    registerNativeBiotech(globals_);
    registerNativeNanotech(globals_);
    registerNativeClimate(globals_);
    registerNativeMedical(globals_);
    registerNativePhysics(globals_);
    registerNativeMathematics(globals_);
    registerNativeOptimization(globals_);
    registerNativeCybersecurity(globals_);
    registerNativeAugmentedReality(globals_);
    registerNativeBlockchainAdvanced(globals_);
    registerNativeQuantumComputingAdvanced(globals_);
    
    // ==================== TIME AND DATE STUFF (NEW FOR v0.7.5) ====================

    
    // Time moved to native_time.cpp
    
    // ==================== JSON HANDLING (NEW FOR v0.7.5) ====================
    
    registerNativeJSON(globals_);
    registerNativeSecurity(globals_, *this);
    
    
    // type(val) - get type of value as string
    globals_->define("type", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            const Value& v = args[0];
            std::string t = "unknown";
            if (isNil(v)) t = "nil";
            else if (isBool(v)) t = "bool";
            else if (isNumber(v)) t = "number";
            else if (isString(v)) t = "string";
            else if (isCallable(v)) t = "function";
            else if (isArray(v)) t = "array";
            else if (isHashMap(v)) t = "hashmap";
            auto sv = StringPool::intern(t);
            return stringValue(sv.data());
        },
        "type"
    ));
    
    // keys(hashmap) - get all keys from a hash map
    globals_->define("keys", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isHashMap(args[0])) {
                throw std::runtime_error("keys() requires a hashmap argument");
            }
            
            auto map = asHashMap(args[0]);
            auto keysVec = map->getKeys();
            
            auto resultArray = gcNewArray();
            for (const auto& key : keysVec) {
                auto sv = StringPool::intern(key);
                resultArray->push(stringValue(sv.data()));
            }
            
            return arrayValue(resultArray);
        },
        "keys"
    ));
    
    // values(hashmap) - get all values from a hash map
    globals_->define("values", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isHashMap(args[0])) {
                throw std::runtime_error("values() requires a hashmap argument");
            }
            
            auto map = asHashMap(args[0]);
            auto valuesVec = map->getValues();
            
            auto resultArray = gcNewArray();
            for (const auto& value : valuesVec) {
                resultArray->push(value);
            }
            
            return arrayValue(resultArray);
        },
        "values"
    ));
    
    // has(hashmap, key) - check if a key exists in a hash map
    globals_->define("has", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isHashMap(args[0])) {
                throw std::runtime_error("has() requires a hashmap as first argument");
            }
            if (!isString(args[1]) && !isNumber(args[1]) && !isBool(args[1]) && !isNil(args[1])) {
                throw std::runtime_error("has() requires a string, number, boolean, or nil as key");
            }
            
            auto map = asHashMap(args[0]);
            
            // Convert key to string
            std::string keyStr = valueToString(args[1]);
            
            return boolValue(map->contains(keyStr));
        },
        "has"
    ));
    
    // remove(hashmap, key) - remove a key-value pair from a hash map
    globals_->define("remove", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isHashMap(args[0])) {
                throw std::runtime_error("remove() requires a hashmap as first argument");
            }
            if (!isString(args[1]) && !isNumber(args[1]) && !isBool(args[1]) && !isNil(args[1])) {
                throw std::runtime_error("remove() requires a string, number, boolean, or nil as key");
            }
            
            auto map = asHashMap(args[0]);
            
            // Convert key to string
            std::string keyStr = valueToString(args[1]);
            
            return boolValue(map->remove(keyStr));  // Returns true if removed, false if not found
        },
        "remove"
    ));
    
    // ==================== FILE I/O ENHANCEMENTS (NEW FOR v0.7.9) ====================
    // IO enhancements moved to native_io.cpp
    
    
    // charCodeAt(str, index) - get character code at index
    globals_->define("charCodeAt", std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) throw std::runtime_error("charCodeAt() requires a string as first argument");
            if (!isNumber(args[1])) throw std::runtime_error("charCodeAt() requires a number as index");
            
            std::string str = asString(args[0]);
            int index = static_cast<int>(asNumber(args[1]));
            
            if (index < 0 || index >= static_cast<int>(str.length())) {
                return numberToValue(-1.0); // Return -1 if index is out of bounds
            }
            
            return numberToValue(static_cast<double>(static_cast<unsigned char>(str[index])));
        },
        "charCodeAt"
    ));
    
    // fromCharCode(code) - create string from character code
    globals_->define("fromCharCode", std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) throw std::runtime_error("fromCharCode() requires a number");
            
            int code = static_cast<int>(asNumber(args[0]));
            
            if (code < 0 || code > 255) {
                throw std::runtime_error("Character code must be between 0 and 255");
            }
            
            auto sv = StringPool::intern(std::string(1, static_cast<char>(code)));
            return stringValue(sv.data());
        },
        "fromCharCode"
    ));
    
    // ==================== FUNCTIONAL PROGRAMMING UTILITIES (NEW FOR v0.7.9) ====================
    
    // compose(...functions) — compose right-to-left: compose(f,g)(x) = f(g(x))
    struct ComposeCallable : public Callable {
        std::vector<Value> fns;
        explicit ComposeCallable(std::vector<Value> f) : fns(std::move(f)) {}
        Value call(Interpreter& interp, const std::vector<Value>& args) override {
            Value result = args.empty() ? nilValue() : args[0];
            for (int i = static_cast<int>(fns.size()) - 1; i >= 0; --i) {
                result = asCallable(fns[i])->call(interp, {result});
            }
            return result;
        }
        int arity() const override { return 1; }
        std::string toString() const override { return "<composed>"; }
    };
    globals_->define("compose", callableValue(std::make_shared<NativeFunction>(
        -1,
        [](const std::vector<Value>& args) -> Value {
            for (const auto& a : args)
                if (!isCallable(a)) throw std::runtime_error("compose(): all arguments must be functions");
            // Return a Callable that captures the function list
            struct Composed : public Callable {
                std::vector<Value> fns;
                explicit Composed(std::vector<Value> f) : fns(std::move(f)) {}
                Value call(Interpreter& interp, const std::vector<Value>& a) override {
                    Value r = a.empty() ? nilValue() : a[0];
                    for (int i = static_cast<int>(fns.size()) - 1; i >= 0; --i)
                        r = asCallable(fns[i])->call(interp, {r});
                    return r;
                }
                int arity() const override { return 1; }
                std::string toString() const override { return "<composed>"; }
            };
            return callableValue(std::make_shared<Composed>(args));
        },
        "compose"
    )));

    // pipe(...functions) — pipe left-to-right: pipe(f,g)(x) = g(f(x))
    globals_->define("pipe", callableValue(std::make_shared<NativeFunction>(
        -1,
        [](const std::vector<Value>& args) -> Value {
            for (const auto& a : args)
                if (!isCallable(a)) throw std::runtime_error("pipe(): all arguments must be functions");
            struct Piped : public Callable {
                std::vector<Value> fns;
                explicit Piped(std::vector<Value> f) : fns(std::move(f)) {}
                Value call(Interpreter& interp, const std::vector<Value>& a) override {
                    Value r = a.empty() ? nilValue() : a[0];
                    for (const auto& fn : fns)
                        r = asCallable(fn)->call(interp, {r});
                    return r;
                }
                int arity() const override { return 1; }
                std::string toString() const override { return "<piped>"; }
            };
            return callableValue(std::make_shared<Piped>(args));
        },
        "pipe"
    )));
    
    // ==================== PERFORMANCE UTILITIES (NEW FOR v0.7.9) ====================
    // Sleep moved to native_time.cpp
    
    // benchmark(func, ...args) - measure execution time of function
    globals_->define("benchmark", std::make_shared<NativeFunction>(
        -1, // Variable arity: function + any number of arguments
        [this](const std::vector<Value>& args) -> Value {
            if (args.empty() || !isCallable(args[0])) {
                throw std::runtime_error("benchmark() requires a function as first argument");
            }
            
            auto func = asCallable(args[0]);
            std::vector<Value> callArgs(args.begin() + 1, args.end());
            
            auto start = std::chrono::high_resolution_clock::now();
            Value result = func->call(*this, callArgs);
            auto end = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
            
            // Return an object with result and execution time
            auto resultMap = std::make_shared<ClawHashMap>();
            resultMap->set("result", result);
            resultMap->set("timeMicroseconds", numberToValue(static_cast<double>(duration.count())));
            resultMap->set("timeMilliseconds", numberToValue(static_cast<double>(duration.count()) / 1000.0));
            
            return hashMapValue(resultMap);
        },
        "benchmark"
    ));
    globals_->define("profilePause", std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>&) -> Value { profilerPause(); return nilValue(); },
        "profilePause"
    ));
    globals_->define("profileResume", std::make_shared<NativeFunction>(
        0,
        [](const std::vector<Value>&) -> Value { profilerResume(); return nilValue(); },
        "profileResume"
    ));
}

// ========================================
// STATEMENT EXECUTION
// ========================================

void Interpreter::execute(Stmt* stmt) {
    if (stmt) {
        stmt->accept(*this);
    }
}

void Interpreter::execute(const std::vector<StmtPtr>& statements) {
    tl_current_interpreter = this;
    for (const auto& stmt : statements) {
        execute(stmt.get());
    }
}

void Interpreter::visitExprStmt(ExprStmt* stmt) {
    evaluate(stmt->expr.get());
}

void Interpreter::visitPrintStmt(PrintStmt* stmt) {
    Value value = evaluate(stmt->expr.get());
    if (!globals_->canOutput()) {
        throwRuntimeError(stmt->token, ErrorCode::RUNTIME_ERROR, "Output disabled by sandbox");
    }
    std::cout << valueToDisplayString(value) << "\n";
}

void Interpreter::visitLetStmt(LetStmt* stmt) {
    Value value = nilValue();
    if (stmt->initializer) {
        value = evaluate(stmt->initializer.get());
    }
    environment_->define(stmt->name, value);
}

void Interpreter::visitBlockStmt(BlockStmt* stmt) {
    executeBlock(stmt->statements,
                 std::make_shared<Environment>(environment_));
}

void Interpreter::executeBlock(const std::vector<StmtPtr>& statements,
                                std::shared_ptr<Environment> environment) {
    std::shared_ptr<Environment> previous = environment_;
    try {
        environment_ = environment;
        for (const auto& stmt : statements) {
            execute(stmt.get());
        }
        environment_ = previous;
    } catch (...) {
        environment_ = previous;
        throw;
    }
}

void Interpreter::visitIfStmt(IfStmt* stmt) {
    Value condition = evaluate(stmt->condition.get());
    if (isTruthy(condition)) {
        execute(stmt->thenBranch.get());
    } else if (stmt->elseBranch) {
        execute(stmt->elseBranch.get());
    }
}

void Interpreter::visitWhileStmt(WhileStmt* stmt) {
    while (isTruthy(evaluate(stmt->condition.get()))) {
        try {
            execute(stmt->body.get());
        } catch (const ContinueException&) {
            continue; // Continue to next iteration
        } catch (const BreakException&) {
            break; // Exit the loop
        }
    }
}

void Interpreter::visitRunUntilStmt(RunUntilStmt* stmt) {
    // Run-until: executes body at least once, then continues until condition becomes TRUE
    do {
        try {
            execute(stmt->body.get());
        } catch (const ContinueException&) {
            continue; // Continue to next iteration
        } catch (const BreakException&) {
            break; // Exit the loop
        }
    } while (!isTruthy(evaluate(stmt->condition.get())));
}

void Interpreter::visitForStmt(ForStmt* stmt) {
    // Create new scope for loop
    auto loopEnv = std::make_shared<Environment>(environment_);
    auto previous = environment_;
    try {
        environment_ = loopEnv;
        
        // Execute initializer
        if (stmt->initializer) {
            execute(stmt->initializer.get());
        }
        
        // Condition (default to true if omitted)
        auto checkCondition = [&]() {
            if (stmt->condition) {
                return isTruthy(evaluate(stmt->condition.get()));
            }
            return true;
        };
        
        // Loop with break/continue support
        while (checkCondition()) {
            try {
                execute(stmt->body.get());
            } catch (const ContinueException&) {
                // Continue - execute increment and check condition
            } catch (const BreakException&) {
                break; // Exit the loop
            }
            
            // Execute increment
            if (stmt->increment) {
                evaluate(stmt->increment.get());
            }
        }
        
        environment_ = previous;
    } catch (...) {
        environment_ = previous;
        throw;
    }
}

void Interpreter::visitFnStmt(FnStmt* stmt) {
    // Create a function object that captures the current environment
    auto function = std::make_shared<ClawFunction>(stmt, environment_);
    
    // Define the function in the current scope
    environment_->define(stmt->name, function);
}

void Interpreter::visitReturnStmt(ReturnStmt* stmt) {
    if (stmt->values.size() > 1) {
        // Multiple return values — wrap in array (Lua-style)
        auto arr = gcNewArray();
        for (const auto& v : stmt->values) arr->push(evaluate(v.get()));
        throw ReturnValue(arrayValue(arr));
    }
    Value value = stmt->values.empty() ? nilValue() : evaluate(stmt->values[0].get());
    throw ReturnValue(value);
}

void Interpreter::visitBreakStmt(BreakStmt*) {
    throw BreakException();
}

void Interpreter::visitContinueStmt(ContinueStmt*) {
    throw ContinueException();
}

void Interpreter::visitTryStmt(TryStmt* stmt) {
    if (!stmt->tryBody) return;
    
    try {
        execute(stmt->tryBody.get());
    } catch (const RuntimeError& e) {
        if (!stmt->catchBody) return;
        
        // Create new environment for catch block
        auto catchEnv = std::make_shared<Environment>(environment_);
        
        // Formatted error message with error code
        std::string errorMsg = errorCodeToString(e.code) + ": " + e.what();
        auto sv = StringPool::intern(errorMsg);
        catchEnv->define(stmt->exceptionVar, stringValue(sv.data()));
        
        auto previousEnv = environment_;
        try {
            environment_ = catchEnv;
            execute(stmt->catchBody.get());
            environment_ = previousEnv;
        } catch (...) {
            environment_ = previousEnv;
            throw;
        }
    } catch (const std::exception& e) {
        if (!stmt->catchBody) return;
        
        auto catchEnv = std::make_shared<Environment>(environment_);
        auto sv2 = StringPool::intern(std::string(e.what()));
        catchEnv->define(stmt->exceptionVar, stringValue(sv2.data()));
        
        auto previousEnv = environment_;
        try {
            environment_ = catchEnv;
            execute(stmt->catchBody.get());
            environment_ = previousEnv;
        } catch (...) {
            environment_ = previousEnv;
            throw;
        }
    }
}

void Interpreter::visitThrowStmt(ThrowStmt* stmt) {
    Value value = evaluate(stmt->expression.get());
    std::string message = valueToString(value);
    
    // We'll use a specific error code for user-thrown errors
    throwRuntimeError(stmt->token, ErrorCode::RUNTIME_ERROR, message);
}

void Interpreter::visitImportStmt(ImportStmt* stmt) {
    try {
        // 1. Load the module
        auto module = module_manager_.loadModule(stmt->modulePath, *this);
        
        // 2. Extract requested imports
        for (const auto& name : stmt->imports) {
            try {
                Value exportedValue = module->getExport(name);
                environment_->define(name, exportedValue);
            } catch (...) {
                throwRuntimeError(stmt->token, ErrorCode::UNDEFINED_VARIABLE, 
                    "Module '" + stmt->modulePath + "' does not export '" + name + "'");
            }
        }
    } catch (const std::exception& e) {
        throwRuntimeError(stmt->token, ErrorCode::RUNTIME_ERROR, e.what());
    }
}

void Interpreter::visitSwitchStmt(SwitchStmt* stmt) {
    Value switchVal = evaluate(stmt->expression.get());
    
    int startIndex = -1;
    int defaultIndex = -1;
    for (int i = 0; i < static_cast<int>(stmt->cases.size()); ++i) {
        const auto& c = stmt->cases[i];
        if (c.isDefault) {
            defaultIndex = i;
            continue;
        }
        Value caseVal = evaluate(c.match.get());
        if (isEqual(switchVal, caseVal)) {
            startIndex = i;
            break;
        }
    }
    
    if (startIndex == -1) startIndex = defaultIndex;
    if (startIndex == -1) return;
    
    for (int i = startIndex; i < static_cast<int>(stmt->cases.size()); ++i) {
        const auto& c = stmt->cases[i];
        try {
            executeBlock(c.body, environment_);
        } catch (const BreakException&) {
            return;
        }
    }
}

// ========================================
// EXPRESSION EVALUATION
// ========================================

Value Interpreter::evaluate(Expr* expr) {
    if (expr) {
        return expr->accept(*this);
    }
    return nilValue();
}

Value Interpreter::visitLiteralExpr(LiteralExpr* expr) {
    return expr->value;
}

Value Interpreter::visitVariableExpr(VariableExpr* expr) {
    try {
        return environment_->get(expr->name);
    } catch (const ClawError& e) {
        throwRuntimeError(expr->token, e.code, e.what());
    }
}

Value Interpreter::visitUnaryExpr(UnaryExpr* expr) {
    Value right = evaluate(expr->right.get());
    
    switch (expr->op.type) {
        case TokenType::Minus:
            if (isNumber(right)) return numberToValue(-asNumber(right));
            { Value out; if (tryMetamethod(right, "__unm", {}, out)) return out; }
            checkNumberOperand(expr->op, right);
            return numberToValue(-asNumber(right));
        case TokenType::Bang:
            { Value out; if (tryMetamethod(right, "__not", {}, out)) return out; }
            return boolValue(!isTruthy(right));
        case TokenType::BitNot: {
            checkNumberOperand(expr->op, right);
            auto v = static_cast<int64_t>(asNumber(right));
            return numberToValue(static_cast<double>(~v));
        }
        default:
            throwRuntimeError(expr->op, ErrorCode::TYPE_MISMATCH, "Unknown unary operator");
    }
}

Value Interpreter::visitBinaryExpr(BinaryExpr* expr) {
    Value left = evaluate(expr->left.get());
    Value right = evaluate(expr->right.get());
    
    switch (expr->op.type) {
        case TokenType::Plus:
            if (isNumber(left) && isNumber(right)) {
                return numberToValue(asNumber(left) + asNumber(right));
            }
            if (isString(left) && isString(right)) {
                return stringValue(StringPool::intern(asString(left) + asString(right)).data());
            }
            // Bool + Bool → numeric addition (true=1, false=0)
            if (isBool(left) && isBool(right)) {
                return numberToValue((asBool(left) ? 1.0 : 0.0) + (asBool(right) ? 1.0 : 0.0));
            }
            // Bool + Number / Number + Bool
            if (isBool(left) && isNumber(right)) {
                return numberToValue((asBool(left) ? 1.0 : 0.0) + asNumber(right));
            }
            if (isNumber(left) && isBool(right)) {
                return numberToValue(asNumber(left) + (asBool(right) ? 1.0 : 0.0));
            }
            // Type coercion: string + number or number + string
            if (isString(left) && isNumber(right)) {
                return stringValue(StringPool::intern(asString(left) + valueToString(right)).data());
            }
            if (isNumber(left) && isString(right)) {
                return stringValue(StringPool::intern(valueToString(left) + asString(right)).data());
            }
            // String + Bool / Bool + String
            if (isString(left)) {
                return stringValue(StringPool::intern(asString(left) + valueToString(right)).data());
            }
            if (isString(right)) {
                return stringValue(StringPool::intern(valueToString(left) + asString(right)).data());
            }
            // Metamethod __add
            { Value out; if (tryMetamethod(left, "__add", {right}, out)) return out; }
            { Value out; if (tryMetamethod(right, "__radd", {left}, out)) return out; }
            throwRuntimeError(expr->op, ErrorCode::TYPE_MISMATCH, "Operands must be two numbers or two strings");
            
        case TokenType::Minus:
            if (isNumber(left) && isNumber(right)) return numberToValue(asNumber(left) - asNumber(right));
            { Value out; if (tryMetamethod(left, "__sub", {right}, out)) return out; }
            checkNumberOperands(expr->op, left, right);
            return numberToValue(asNumber(left) - asNumber(right));
        case TokenType::Star:
            if (isNumber(left) && isNumber(right)) return numberToValue(asNumber(left) * asNumber(right));
            { Value out; if (tryMetamethod(left, "__mul", {right}, out)) return out; }
            { Value out; if (tryMetamethod(right, "__mul", {left}, out)) return out; }
            checkNumberOperands(expr->op, left, right);
            return numberToValue(asNumber(left) * asNumber(right));
        case TokenType::Slash:
            if (isNumber(left) && isNumber(right)) {
                if (asNumber(right) == 0.0) throwRuntimeError(expr->op, ErrorCode::DIVISION_BY_ZERO, "Division by zero");
                return numberToValue(asNumber(left) / asNumber(right));
            }
            { Value out; if (tryMetamethod(left, "__div", {right}, out)) return out; }
            checkNumberOperands(expr->op, left, right);
            if (asNumber(right) == 0.0) throwRuntimeError(expr->op, ErrorCode::DIVISION_BY_ZERO, "Division by zero");
            return numberToValue(asNumber(left) / asNumber(right));
        case TokenType::Percent:
            if (isNumber(left) && isNumber(right)) {
                if (asNumber(right) == 0.0) throwRuntimeError(expr->op, ErrorCode::DIVISION_BY_ZERO, "Division by zero");
                return numberToValue(std::fmod(asNumber(left), asNumber(right)));
            }
            { Value out; if (tryMetamethod(left, "__mod", {right}, out)) return out; }
            checkNumberOperands(expr->op, left, right);
            if (asNumber(right) == 0.0) throwRuntimeError(expr->op, ErrorCode::DIVISION_BY_ZERO, "Division by zero");
            return numberToValue(std::fmod(asNumber(left), asNumber(right)));
            
        case TokenType::Greater:
            if (isNumber(left) && isNumber(right))
                return boolValue(asNumber(left) > asNumber(right));
            if (isString(left) && isString(right))
                return boolValue(asString(left) > asString(right));
            { Value out; if (tryMetamethod(left, "__gt", {right}, out)) return out; }
            checkNumberOperands(expr->op, left, right);
            return boolValue(asNumber(left) > asNumber(right));
        case TokenType::GreaterEqual:
            if (isNumber(left) && isNumber(right))
                return boolValue(asNumber(left) >= asNumber(right));
            if (isString(left) && isString(right))
                return boolValue(asString(left) >= asString(right));
            { Value out; if (tryMetamethod(left, "__ge", {right}, out)) return out; }
            checkNumberOperands(expr->op, left, right);
            return boolValue(asNumber(left) >= asNumber(right));
        case TokenType::Less:
            if (isNumber(left) && isNumber(right))
                return boolValue(asNumber(left) < asNumber(right));
            if (isString(left) && isString(right))
                return boolValue(asString(left) < asString(right));
            { Value out; if (tryMetamethod(left, "__lt", {right}, out)) return out; }
            checkNumberOperands(expr->op, left, right);
            return boolValue(asNumber(left) < asNumber(right));
        case TokenType::LessEqual:
            if (isNumber(left) && isNumber(right))
                return boolValue(asNumber(left) <= asNumber(right));
            if (isString(left) && isString(right))
                return boolValue(asString(left) <= asString(right));
            { Value out; if (tryMetamethod(left, "__le", {right}, out)) return out; }
            checkNumberOperands(expr->op, left, right);
            return boolValue(asNumber(left) <= asNumber(right));
            
        case TokenType::EqualEqual: {
            Value out;
            if (tryMetamethod(left, "__eq", {right}, out)) return out;
            return boolValue(isEqual(left, right));
        }
        case TokenType::BangEqual: {
            Value out;
            if (tryMetamethod(left, "__eq", {right}, out)) return boolValue(!isTruthy(out));
            return boolValue(!isEqual(left, right));
        }
        
        // Bitwise operations (integers via truncation)
        case TokenType::BitAnd: {
            checkNumberOperands(expr->op, left, right);
            auto lv = static_cast<int64_t>(asNumber(left));
            auto rv = static_cast<int64_t>(asNumber(right));
            return numberToValue(static_cast<double>(lv & rv));
        }
        case TokenType::BitOr: {
            checkNumberOperands(expr->op, left, right);
            auto lv = static_cast<int64_t>(asNumber(left));
            auto rv = static_cast<int64_t>(asNumber(right));
            return numberToValue(static_cast<double>(lv | rv));
        }
        case TokenType::BitXor: {
            checkNumberOperands(expr->op, left, right);
            auto lv = static_cast<int64_t>(asNumber(left));
            auto rv = static_cast<int64_t>(asNumber(right));
            return numberToValue(static_cast<double>(lv ^ rv));
        }
        case TokenType::ShiftLeft: {
            checkNumberOperands(expr->op, left, right);
            auto lv = static_cast<int64_t>(asNumber(left));
            auto sh = static_cast<int>(asNumber(right));
            if (sh < 0) {
                throwRuntimeError(expr->op, ErrorCode::RUNTIME_ERROR, "Shift count must be non-negative");
            }
            sh &= 63; // limit to width
            return numberToValue(static_cast<double>(lv << sh));
        }
        case TokenType::ShiftRight: {
            checkNumberOperands(expr->op, left, right);
            auto lv = static_cast<int64_t>(asNumber(left));
            auto sh = static_cast<int>(asNumber(right));
            if (sh < 0) {
                throwRuntimeError(expr->op, ErrorCode::RUNTIME_ERROR, "Shift count must be non-negative");
            }
            sh &= 63; // limit to width
            return numberToValue(static_cast<double>(lv >> sh));
        }
            
        default:
            throwRuntimeError(expr->op, ErrorCode::TYPE_MISMATCH, "Unknown binary operator");
    }
}

Value Interpreter::visitLogicalExpr(LogicalExpr* expr) {
    Value left = evaluate(expr->left.get());
    
    // Short-circuit evaluation
    if (expr->op.type == TokenType::Or) {
        if (isTruthy(left)) return left;
    } else {
        if (!isTruthy(left)) return left;
    }
    
    return evaluate(expr->right.get());
}

Value Interpreter::visitGroupingExpr(GroupingExpr* expr) {
    return evaluate(expr->expression.get());
}

Value Interpreter::visitCallExpr(CallExpr* expr) {
    // Evaluate the callee (the thing being called)
    Value callee = evaluate(expr->callee.get());
    
    // Evaluate all the arguments
    std::vector<Value> arguments;
    for (const auto& arg : expr->arguments) {
        // Handle spread: ...array expands into individual arguments
        if (auto* spread = dynamic_cast<SpreadExpr*>(arg.get())) {
            Value spreadVal = evaluate(spread->expr.get());
            if (isArray(spreadVal)) {
                auto arr = asArray(spreadVal);
                for (int i = 0; i < arr->length(); i++) {
                    arguments.push_back(arr->get(i));
                }
            } else {
                arguments.push_back(spreadVal);
            }
        } else {
            arguments.push_back(evaluate(arg.get()));
        }
    }
    
    if (!isCallable(callee) && !isClass(callee)) {
        throwRuntimeError(
            expr->token,
            ErrorCode::NOT_CALLABLE,
            "Can only call functions and classes"
        );
    }
    
    std::shared_ptr<Callable> function;
    if (isClass(callee)) {
        function = asClass(callee);
    } else {
        function = asCallable(callee);
    }
    
    // Check arity (number of arguments)
    if (function->arity() != -1 && arguments.size() != static_cast<size_t>(function->arity())) {
        throwRuntimeError(
            expr->token,
            ErrorCode::ARGUMENT_COUNT_MISMATCH,
            "Expected " + std::to_string(function->arity()) +
            " arguments but got " + std::to_string(arguments.size())
        );
    }
    
    // Call the function!
    return function->call(*this, arguments);
}

Value Interpreter::visitAssignExpr(AssignExpr* expr) {
    Value value = evaluate(expr->value.get());
    try {
        environment_->assign(expr->name, value);
    } catch (const std::runtime_error&) {
        // If variable doesn't exist, create it (implicit declaration)
        environment_->define(expr->name, value);
    }
    return value;
}

Value Interpreter::visitCompoundAssignExpr(CompoundAssignExpr* expr) {
    Value current;
    try {
        current = environment_->get(expr->name);
    } catch (const ClawError& e) {
        throwRuntimeError(expr->token, e.code, e.what());
    }
    
    Value operand = evaluate(expr->value.get());
    Value result = nilValue();
    
    switch (expr->op.type) {
        case TokenType::PlusEqual:
            if (isNumber(current) && isNumber(operand)) {
                result = numberToValue(asNumber(current) + asNumber(operand));
            } else if (isString(current) && isString(operand)) {
                auto sv = StringPool::intern(asString(current) + asString(operand));
                result = stringValue(sv.data());
            } else if (isString(current) && isNumber(operand)) {
                auto sv = StringPool::intern(asString(current) + valueToString(operand));
                result = stringValue(sv.data());
            } else {
                throwRuntimeError(expr->op, ErrorCode::TYPE_MISMATCH, "Operands must be compatible for +=");
            }
            break;
        case TokenType::MinusEqual:
            checkNumberOperands(expr->op, current, operand);
            result = numberToValue(asNumber(current) - asNumber(operand));
            break;
        case TokenType::StarEqual:
            checkNumberOperands(expr->op, current, operand);
            result = numberToValue(asNumber(current) * asNumber(operand));
            break;
        case TokenType::SlashEqual:
            checkNumberOperands(expr->op, current, operand);
            if (asNumber(operand) == 0.0) {
                throwRuntimeError(expr->op, ErrorCode::DIVISION_BY_ZERO, "Division by zero");
            }
            result = numberToValue(asNumber(current) / asNumber(operand));
            break;
        case TokenType::BitAndEqual:
        case TokenType::BitAnd: {
            checkNumberOperands(expr->op, current, operand);
            auto lv = static_cast<int64_t>(asNumber(current));
            auto rv = static_cast<int64_t>(asNumber(operand));
            result = numberToValue(static_cast<double>(lv & rv));
            break;
        }
        case TokenType::BitOrEqual:
        case TokenType::BitOr: {
            checkNumberOperands(expr->op, current, operand);
            auto lv = static_cast<int64_t>(asNumber(current));
            auto rv = static_cast<int64_t>(asNumber(operand));
            result = numberToValue(static_cast<double>(lv | rv));
            break;
        }
        case TokenType::BitXorEqual:
        case TokenType::BitXor: {
            checkNumberOperands(expr->op, current, operand);
            auto lv = static_cast<int64_t>(asNumber(current));
            auto rv = static_cast<int64_t>(asNumber(operand));
            result = numberToValue(static_cast<double>(lv ^ rv));
            break;
        }
        case TokenType::ShiftLeftEqual:
        case TokenType::ShiftLeft: {
            checkNumberOperands(expr->op, current, operand);
            auto lv = static_cast<int64_t>(asNumber(current));
            auto sh = static_cast<int>(asNumber(operand));
            if (sh < 0) {
                throwRuntimeError(expr->op, ErrorCode::RUNTIME_ERROR, "Shift count must be non-negative");
            }
            sh &= 63;
            result = numberToValue(static_cast<double>(lv << sh));
            break;
        }
        case TokenType::ShiftRightEqual:
        case TokenType::ShiftRight: {
            checkNumberOperands(expr->op, current, operand);
            auto lv = static_cast<int64_t>(asNumber(current));
            auto sh = static_cast<int>(asNumber(operand));
            if (sh < 0) {
                throwRuntimeError(expr->op, ErrorCode::RUNTIME_ERROR, "Shift count must be non-negative");
            }
            sh &= 63;
            result = numberToValue(static_cast<double>(lv >> sh));
            break;
        }
        default:
            throwRuntimeError(expr->op, ErrorCode::TYPE_MISMATCH, "Unknown compound assignment operator");
    }
    
    try {
        environment_->assign(expr->name, result);
    } catch (const ClawError& e) {
        throwRuntimeError(expr->token, e.code, e.what());
    }
    return result;
}

Value Interpreter::visitUpdateExpr(UpdateExpr* expr) {
    Value current;
    try {
        current = environment_->get(expr->name);
    } catch (const ClawError& e) {
        throwRuntimeError(expr->token, e.code, e.what());
    }
    
    if (!isNumber(current)) {
        throwRuntimeError(expr->op, ErrorCode::TYPE_MISMATCH, "Operand must be a number for increment/decrement");
    }
    
    double oldValue = asNumber(current);
    double newValue;
    if (expr->op.type == TokenType::PlusPlus) {
        newValue = oldValue + 1;
    } else {
        newValue = oldValue - 1;
    }
    
    try {
        environment_->assign(expr->name, numberToValue(newValue));
    } catch (const ClawError& e) {
        throwRuntimeError(expr->token, e.code, e.what());
    }
    
    // Return old value for postfix, new value for prefix
    return expr->prefix ? numberToValue(newValue) : numberToValue(oldValue);
}

Value Interpreter::visitUpdateMemberExpr(UpdateMemberExpr* expr) {
    Value object = evaluate(expr->object.get());
    
    // Hash map field
    if (isHashMap(object)) {
        auto map = asHashMap(object);
        Value cur = map->get(expr->member);
        if (!isNumber(cur)) {
            throwRuntimeError(expr->op, ErrorCode::TYPE_MISMATCH, "Operand must be a number for increment/decrement");
        }
        double oldVal = asNumber(cur);
        double newVal = (expr->op.type == TokenType::PlusPlus) ? (oldVal + 1) : (oldVal - 1);
        map->set(expr->member, numberToValue(newVal));
        return expr->prefix ? numberToValue(newVal) : numberToValue(oldVal);
    }
    
    // Class instance field
    if (isInstance(object)) {
        auto inst = asInstance(object);
        auto sv = StringPool::intern(expr->member);
        Token memberTok(TokenType::Identifier, sv, expr->token.line, expr->token.column);
        Value cur = inst->get(memberTok);
        if (!isNumber(cur)) {
            throwRuntimeError(expr->op, ErrorCode::TYPE_MISMATCH, "Operand must be a number for increment/decrement");
        }
        double oldVal = asNumber(cur);
        double newVal = (expr->op.type == TokenType::PlusPlus) ? (oldVal + 1) : (oldVal - 1);
        inst->set(memberTok, numberToValue(newVal));
        return expr->prefix ? numberToValue(newVal) : numberToValue(oldVal);
    }
    
    throwRuntimeError(expr->token, ErrorCode::RUNTIME_ERROR, "Invalid object for member update");
}

Value Interpreter::visitUpdateIndexExpr(UpdateIndexExpr* expr) {
    Value object = evaluate(expr->object.get());
    Value index = evaluate(expr->index.get());
    
    // Array index
    if (isArray(object)) {
        auto array = asArray(object);
        if (!isNumber(index)) {
            throwRuntimeError(expr->token, ErrorCode::TYPE_MISMATCH, "Array index must be a number");
        }
        int idx = static_cast<int>(asNumber(index));
        if (idx < 0 || idx >= array->length()) {
            throwRuntimeError(expr->token, ErrorCode::INDEX_OUT_OF_BOUNDS,
                "Index " + std::to_string(idx) + " out of bounds [0, " + std::to_string(array->length() - 1) + "]");
        }
        Value cur = array->get(idx);
        if (!isNumber(cur)) {
            throwRuntimeError(expr->op, ErrorCode::TYPE_MISMATCH, "Operand must be a number for increment/decrement");
        }
        double oldVal = asNumber(cur);
        double newVal = (expr->op.type == TokenType::PlusPlus) ? (oldVal + 1) : (oldVal - 1);
        array->set(idx, numberToValue(newVal));
        return expr->prefix ? numberToValue(newVal) : numberToValue(oldVal);
    }
    
    // Hash map index
    if (isHashMap(object)) {
        auto map = asHashMap(object);
        std::string key;
        if (isString(index)) key = asString(index);
        else if (isNumber(index)) key = std::to_string(asNumber(index));
        else if (isNil(index)) key = "nil";
        else if (isBool(index)) key = asBool(index) ? "true" : "false";
        else {
            throwRuntimeError(expr->token, ErrorCode::TYPE_MISMATCH, "Hash map index must be a string, number, boolean, or nil");
        }
        Value cur = map->get(key);
        if (!isNumber(cur)) {
            throwRuntimeError(expr->op, ErrorCode::TYPE_MISMATCH, "Operand must be a number for increment/decrement");
        }
        double oldVal = asNumber(cur);
        double newVal = (expr->op.type == TokenType::PlusPlus) ? (oldVal + 1) : (oldVal - 1);
        map->set(key, numberToValue(newVal));
        return expr->prefix ? numberToValue(newVal) : numberToValue(oldVal);
    }
    
    throwRuntimeError(expr->token, ErrorCode::NOT_INDEXABLE, "Can only index arrays and hash maps");
}

Value Interpreter::visitSetExpr(SetExpr* expr) {
    Value object = evaluate(expr->object.get());

    if (isInstance(object)) {
        Value value = evaluate(expr->value.get());
        // expr->token is the '=' token; use expr->member for the property name
        Token memberToken(TokenType::Identifier,
                          StringPool::intern(expr->member),
                          expr->token.line, expr->token.column);
        asInstance(object)->set(memberToken, value);
        return value;
    } else if (isHashMap(object)) {
        Value value = evaluate(expr->value.get());
        asHashMap(object)->set(expr->member, value);
        return value;
    }

    throwRuntimeError(expr->token, ErrorCode::RUNTIME_ERROR, "Only instances and hash maps have fields.");
}

Value Interpreter::visitThisExpr(ThisExpr* expr) {
    try {
        return environment_->get("this");
    } catch (const ClawError& e) {
        throwRuntimeError(expr->token, e.code, e.what());
    }
}

Value Interpreter::visitSuperExpr(SuperExpr* expr) {
    try {
        // 1. Look up 'super' in the environment
        Value superValue = environment_->get("super");
        if (!isClass(superValue)) {
            throwRuntimeError(expr->token, ErrorCode::RUNTIME_ERROR, "Can only use 'super' in a class with a superclass.");
        }
        auto superclass = asClass(superValue);

        // 2. Look up 'this' (the instance) to bind the method to
        Value thisValue = environment_->get("this");
        if (!isInstance(thisValue)) {
             throwRuntimeError(expr->token, ErrorCode::RUNTIME_ERROR, "Can only use 'super' inside a class method.");
        }
        auto instance = asInstance(thisValue);

        // 3. Find method in superclass
        auto method = superclass->findMethod(expr->method);

        if (!method) {
            throwRuntimeError(expr->token, ErrorCode::RUNTIME_ERROR, "Undefined property '" + expr->method + "'.");
        }

        // 4. Bind instance to method
        return callableValue(method->bind(instance));
    } catch (const ClawError& e) {
        throwRuntimeError(expr->token, e.code, e.what());
    }
}

Value Interpreter::visitTernaryExpr(TernaryExpr* expr) {
    if (isTruthy(evaluate(expr->condition.get()))) {
        return evaluate(expr->thenBranch.get());
    }
    return evaluate(expr->elseBranch.get());
}

// ========================================
// ARRAY EVALUATION
// ========================================

Value Interpreter::visitArrayExpr(ArrayExpr* expr) {
    std::vector<Value> elements;
    
    // Evaluate all element expressions
    for (const auto& elem : expr->elements) {
        elements.push_back(evaluate(elem.get()));
    }
    
    // Create and return array
    return arrayValue(gcNewArray(elements));
}

Value Interpreter::visitIndexExpr(IndexExpr* expr) {
    Value object = evaluate(expr->object.get());
    Value index = evaluate(expr->index.get());
    
    // Handle arrays
    if (isArray(object)) {
        auto array = asArray(object);
        
        // Index must be a number
        if (!isNumber(index)) {
            throwRuntimeError(expr->token, ErrorCode::TYPE_MISMATCH, "Array index must be a number");
        }
        
        int idx = static_cast<int>(asNumber(index));
        
        // Check bounds
        if (idx < 0 || idx >= array->length()) {
            throwRuntimeError(expr->token, ErrorCode::INDEX_OUT_OF_BOUNDS, 
                "Index " + std::to_string(idx) + " out of bounds [0, " + std::to_string(array->length() - 1) + "]");
        }
        
        return array->get(idx);
    }
    
    // Handle hash maps
    if (isHashMap(object)) {
        auto map = asHashMap(object);
        
        // Convert index to string key
        std::string key;
        if (isString(index)) {
            key = asString(index);
        } else if (isNumber(index)) {
            // Convert number to string representation
            double num = asNumber(index);
            if (num == static_cast<long long>(num)) {
                key = std::to_string(static_cast<long long>(num));
            } else {
                key = std::to_string(num);
                // Remove trailing zeros after decimal point
                key.erase(key.find_last_not_of('0') + 1, std::string::npos);
                key.erase(key.find_last_not_of('.') + 1, std::string::npos);
            }
        } else if (isNil(index)) {
            key = "nil";
        } else if (isBool(index)) {
            key = asBool(index) ? "true" : "false";
        } else {
            throwRuntimeError(expr->token, ErrorCode::TYPE_MISMATCH, "Hash map index must be a string, number, boolean, or nil");
        }
        
        return map->get(key);
    }
    
    throwRuntimeError(expr->token, ErrorCode::NOT_INDEXABLE, "Can only index arrays and hash maps");
}

Value Interpreter::visitIndexAssignExpr(IndexAssignExpr* expr) {
    Value object = evaluate(expr->object.get());
    Value index = evaluate(expr->index.get());
    Value value = evaluate(expr->value.get());
    
    // Handle arrays
    if (isArray(object)) {
        auto array = asArray(object);
        
        // Index must be a number
        if (!isNumber(index)) {
            throwRuntimeError(expr->token, ErrorCode::TYPE_MISMATCH, "Array index must be a number");
        }
        
        int idx = static_cast<int>(asNumber(index));
        
        // Check bounds
        if (idx < 0 || idx >= array->length()) {
            throwRuntimeError(expr->token, ErrorCode::INDEX_OUT_OF_BOUNDS, 
                "Index " + std::to_string(idx) + " out of bounds [0, " + std::to_string(array->length() - 1) + "]");
        }
        
        array->set(idx, value);
        return value;
    }
    
    // Handle hash maps
    if (isHashMap(object)) {
        auto map = asHashMap(object);
        
        // Convert index to string key
        std::string key;
        if (isString(index)) {
            key = asString(index);
        } else if (isNumber(index)) {
            // Convert number to string representation
            if (asNumber(index) == static_cast<long long>(asNumber(index))) {
                key = std::to_string(static_cast<long long>(asNumber(index)));
            } else {
                key = std::to_string(asNumber(index));
            }
        } else if (isNil(index)) {
            key = "nil";
        } else if (isBool(index)) {
            key = asBool(index) ? "true" : "false";
        } else {
            throwRuntimeError(expr->token, ErrorCode::TYPE_MISMATCH, "Hash map index must be a string, number, boolean, or nil");
        }
        
        map->set(key, value);
        return value;
    }
    
    throwRuntimeError(expr->token, ErrorCode::NOT_INDEXABLE, "Can only index arrays and hash maps");
}

Value Interpreter::visitCompoundMemberAssignExpr(CompoundMemberAssignExpr* expr) {
    Value object = evaluate(expr->object.get());
    Value operand = evaluate(expr->value.get());
    Value current;
    bool isMap = false;
    if (isHashMap(object)) {
        auto map = asHashMap(object);
        current = map->get(expr->member);
        isMap = true;
    } else if (isInstance(object)) {
        auto inst = asInstance(object);
        auto sv = StringPool::intern(expr->member);
        Token memberTok(TokenType::Identifier, sv, expr->token.line, expr->token.column);
        current = inst->get(memberTok);
    } else {
        throwRuntimeError(expr->token, ErrorCode::RUNTIME_ERROR, "Invalid object for member compound assignment");
    }
    
    Value result = nilValue();
    switch (expr->op.type) {
        case TokenType::PlusEqual:
            if (isNumber(current) && isNumber(operand)) {
                result = numberToValue(asNumber(current) + asNumber(operand));
            } else if (isString(current) && isString(operand)) {
                auto sv = StringPool::intern(asString(current) + asString(operand));
                result = stringValue(sv.data());
            } else if (isString(current) && isNumber(operand)) {
                auto sv = StringPool::intern(asString(current) + valueToString(operand));
                result = stringValue(sv.data());
            } else {
                throwRuntimeError(expr->op, ErrorCode::TYPE_MISMATCH, "Operands must be compatible for +=");
            }
            break;
        case TokenType::MinusEqual:
            checkNumberOperands(expr->op, current, operand);
            result = numberToValue(asNumber(current) - asNumber(operand));
            break;
        case TokenType::StarEqual:
            checkNumberOperands(expr->op, current, operand);
            result = numberToValue(asNumber(current) * asNumber(operand));
            break;
        case TokenType::SlashEqual:
            checkNumberOperands(expr->op, current, operand);
            if (asNumber(operand) == 0.0) {
                throwRuntimeError(expr->op, ErrorCode::DIVISION_BY_ZERO, "Division by zero");
            }
            result = numberToValue(asNumber(current) / asNumber(operand));
            break;
        case TokenType::BitAndEqual: {
            checkNumberOperands(expr->op, current, operand);
            auto lv = static_cast<int64_t>(asNumber(current));
            auto rv = static_cast<int64_t>(asNumber(operand));
            result = numberToValue(static_cast<double>(lv & rv));
            break;
        }
        case TokenType::BitOrEqual: {
            checkNumberOperands(expr->op, current, operand);
            auto lv = static_cast<int64_t>(asNumber(current));
            auto rv = static_cast<int64_t>(asNumber(operand));
            result = numberToValue(static_cast<double>(lv | rv));
            break;
        }
        case TokenType::BitXorEqual: {
            checkNumberOperands(expr->op, current, operand);
            auto lv = static_cast<int64_t>(asNumber(current));
            auto rv = static_cast<int64_t>(asNumber(operand));
            result = numberToValue(static_cast<double>(lv ^ rv));
            break;
        }
        case TokenType::ShiftLeftEqual: {
            checkNumberOperands(expr->op, current, operand);
            auto lv = static_cast<int64_t>(asNumber(current));
            auto sh = static_cast<int>(asNumber(operand));
            if (sh < 0) {
                throwRuntimeError(expr->op, ErrorCode::RUNTIME_ERROR, "Shift count must be non-negative");
            }
            sh &= 63;
            result = numberToValue(static_cast<double>(lv << sh));
            break;
        }
        case TokenType::ShiftRightEqual: {
            checkNumberOperands(expr->op, current, operand);
            auto lv = static_cast<int64_t>(asNumber(current));
            auto sh = static_cast<int>(asNumber(operand));
            if (sh < 0) {
                throwRuntimeError(expr->op, ErrorCode::RUNTIME_ERROR, "Shift count must be non-negative");
            }
            sh &= 63;
            result = numberToValue(static_cast<double>(lv >> sh));
            break;
        }
        default:
            throwRuntimeError(expr->op, ErrorCode::TYPE_MISMATCH, "Unknown compound assignment operator");
    }
    
    if (isMap) {
        asHashMap(object)->set(expr->member, result);
    } else {
        auto sv2 = StringPool::intern(expr->member);
        Token memberTok2(TokenType::Identifier, sv2, expr->token.line, expr->token.column);
        asInstance(object)->set(memberTok2, result);
    }
    return result;
}

Value Interpreter::visitCompoundIndexAssignExpr(CompoundIndexAssignExpr* expr) {
    Value object = evaluate(expr->object.get());
    Value index = evaluate(expr->index.get());
    Value operand = evaluate(expr->value.get());
    
    if (isArray(object)) {
        auto array = asArray(object);
        if (!isNumber(index)) {
            throwRuntimeError(expr->token, ErrorCode::TYPE_MISMATCH, "Array index must be a number");
        }
        int idx = static_cast<int>(asNumber(index));
        if (idx < 0 || idx >= array->length()) {
            throwRuntimeError(expr->token, ErrorCode::INDEX_OUT_OF_BOUNDS,
                "Index " + std::to_string(idx) + " out of bounds [0, " + std::to_string(array->length() - 1) + "]");
        }
        Value current = array->get(idx);
        Value result = nilValue();
        switch (expr->op.type) {
            case TokenType::PlusEqual:
                if (isNumber(current) && isNumber(operand)) {
                    result = numberToValue(asNumber(current) + asNumber(operand));
                } else if (isString(current) && isString(operand)) {
                    auto sv = StringPool::intern(asString(current) + asString(operand));
                    result = stringValue(sv.data());
                } else if (isString(current) && isNumber(operand)) {
                    auto sv = StringPool::intern(asString(current) + valueToString(operand));
                    result = stringValue(sv.data());
                } else {
                    throwRuntimeError(expr->op, ErrorCode::TYPE_MISMATCH, "Operands must be compatible for +=");
                }
                break;
            case TokenType::MinusEqual:
                checkNumberOperands(expr->op, current, operand);
                result = numberToValue(asNumber(current) - asNumber(operand));
                break;
            case TokenType::StarEqual:
                checkNumberOperands(expr->op, current, operand);
                result = numberToValue(asNumber(current) * asNumber(operand));
                break;
            case TokenType::SlashEqual:
                checkNumberOperands(expr->op, current, operand);
                if (asNumber(operand) == 0.0) {
                    throwRuntimeError(expr->op, ErrorCode::DIVISION_BY_ZERO, "Division by zero");
                }
                result = numberToValue(asNumber(current) / asNumber(operand));
                break;
            case TokenType::BitAndEqual: {
                checkNumberOperands(expr->op, current, operand);
                auto lv = static_cast<int64_t>(asNumber(current));
                auto rv = static_cast<int64_t>(asNumber(operand));
                result = numberToValue(static_cast<double>(lv & rv));
                break;
            }
            case TokenType::BitOrEqual: {
                checkNumberOperands(expr->op, current, operand);
                auto lv = static_cast<int64_t>(asNumber(current));
                auto rv = static_cast<int64_t>(asNumber(operand));
                result = numberToValue(static_cast<double>(lv | rv));
                break;
            }
            case TokenType::BitXorEqual: {
                checkNumberOperands(expr->op, current, operand);
                auto lv = static_cast<int64_t>(asNumber(current));
                auto rv = static_cast<int64_t>(asNumber(operand));
                result = numberToValue(static_cast<double>(lv ^ rv));
                break;
            }
            case TokenType::ShiftLeftEqual: {
                checkNumberOperands(expr->op, current, operand);
                auto lv = static_cast<int64_t>(asNumber(current));
                auto sh = static_cast<int>(asNumber(operand));
                if (sh < 0) {
                    throwRuntimeError(expr->op, ErrorCode::RUNTIME_ERROR, "Shift count must be non-negative");
                }
                sh &= 63;
                result = numberToValue(static_cast<double>(lv << sh));
                break;
            }
            case TokenType::ShiftRightEqual: {
                checkNumberOperands(expr->op, current, operand);
                auto lv = static_cast<int64_t>(asNumber(current));
                auto sh = static_cast<int>(asNumber(operand));
                if (sh < 0) {
                    throwRuntimeError(expr->op, ErrorCode::RUNTIME_ERROR, "Shift count must be non-negative");
                }
                sh &= 63;
                result = numberToValue(static_cast<double>(lv >> sh));
                break;
            }
            default:
                throwRuntimeError(expr->op, ErrorCode::TYPE_MISMATCH, "Unknown compound assignment operator");
        }
        array->set(idx, result);
        return result;
    }
    
    if (isHashMap(object)) {
        auto map = asHashMap(object);
        std::string key;
        if (isString(index)) {
            key = asString(index);
        } else if (isNumber(index)) {
            if (asNumber(index) == static_cast<long long>(asNumber(index))) {
                key = std::to_string(static_cast<long long>(asNumber(index)));
            } else {
                key = std::to_string(asNumber(index));
            }
        } else if (isNil(index)) {
            key = "nil";
        } else if (isBool(index)) {
            key = asBool(index) ? "true" : "false";
        } else {
            throwRuntimeError(expr->token, ErrorCode::TYPE_MISMATCH, "Hash map index must be a string, number, boolean, or nil");
        }
        Value current = map->get(key);
        Value result = nilValue();
        switch (expr->op.type) {
            case TokenType::PlusEqual:
                if (isNumber(current) && isNumber(operand)) {
                    result = numberToValue(asNumber(current) + asNumber(operand));
                } else if (isString(current) && isString(operand)) {
                    auto sv = StringPool::intern(asString(current) + asString(operand));
                    result = stringValue(sv.data());
                } else if (isString(current) && isNumber(operand)) {
                    auto sv = StringPool::intern(asString(current) + valueToString(operand));
                    result = stringValue(sv.data());
                } else {
                    throwRuntimeError(expr->op, ErrorCode::TYPE_MISMATCH, "Operands must be compatible for +=");
                }
                break;
            case TokenType::MinusEqual:
                checkNumberOperands(expr->op, current, operand);
                result = numberToValue(asNumber(current) - asNumber(operand));
                break;
            case TokenType::StarEqual:
                checkNumberOperands(expr->op, current, operand);
                result = numberToValue(asNumber(current) * asNumber(operand));
                break;
            case TokenType::SlashEqual:
                checkNumberOperands(expr->op, current, operand);
                if (asNumber(operand) == 0.0) {
                    throwRuntimeError(expr->op, ErrorCode::DIVISION_BY_ZERO, "Division by zero");
                }
                result = numberToValue(asNumber(current) / asNumber(operand));
                break;
            case TokenType::BitAndEqual: {
                checkNumberOperands(expr->op, current, operand);
                auto lv = static_cast<int64_t>(asNumber(current));
                auto rv = static_cast<int64_t>(asNumber(operand));
                result = numberToValue(static_cast<double>(lv & rv));
                break;
            }
            case TokenType::BitOrEqual: {
                checkNumberOperands(expr->op, current, operand);
                auto lv = static_cast<int64_t>(asNumber(current));
                auto rv = static_cast<int64_t>(asNumber(operand));
                result = numberToValue(static_cast<double>(lv | rv));
                break;
            }
            case TokenType::BitXorEqual: {
                checkNumberOperands(expr->op, current, operand);
                auto lv = static_cast<int64_t>(asNumber(current));
                auto rv = static_cast<int64_t>(asNumber(operand));
                result = numberToValue(static_cast<double>(lv ^ rv));
                break;
            }
            case TokenType::ShiftLeftEqual: {
                checkNumberOperands(expr->op, current, operand);
                auto lv = static_cast<int64_t>(asNumber(current));
                auto sh = static_cast<int>(asNumber(operand));
                if (sh < 0) {
                    throwRuntimeError(expr->op, ErrorCode::RUNTIME_ERROR, "Shift count must be non-negative");
                }
                sh &= 63;
                result = numberToValue(static_cast<double>(lv << sh));
                break;
            }
            case TokenType::ShiftRightEqual: {
                checkNumberOperands(expr->op, current, operand);
                auto lv = static_cast<int64_t>(asNumber(current));
                auto sh = static_cast<int>(asNumber(operand));
                if (sh < 0) {
                    throwRuntimeError(expr->op, ErrorCode::RUNTIME_ERROR, "Shift count must be non-negative");
                }
                sh &= 63;
                result = numberToValue(static_cast<double>(lv >> sh));
                break;
            }
            default:
                throwRuntimeError(expr->op, ErrorCode::TYPE_MISMATCH, "Unknown compound assignment operator");
        }
        map->set(key, result);
        return result;
    }
    
    throwRuntimeError(expr->token, ErrorCode::NOT_INDEXABLE, "Can only index arrays and hash maps");
}
Value Interpreter::visitMemberExpr(MemberExpr* expr) {
    Value object = evaluate(expr->object.get());

    // Handle strings — method dispatch (s.upper(), s.split(), etc.)
    if (isString(object)) {
        return stringMethodDispatch(asString(object), expr->member, expr->token);
    }

    // Handle arrays
    if (isArray(object)) {
        auto array = asArray(object);
        
        // Handle array.length
        if (expr->member == "length") {
            return numberToValue(static_cast<double>(array->length()));
        }
        
        // Handle array.push - return a callable that modifies the array
        if (expr->member == "push") {
            return callableValue(std::make_shared<NativeFunction>(
                1,
                [array](const std::vector<Value>& args) -> Value {
                    if (!args.empty()) {
                        array->push(args[0]);
                    }
                    return nilValue(); // returns nil
                },
                "push"
            ));
        }
        
        // Handle array.pop
        if (expr->member == "pop") {
            return callableValue(std::make_shared<NativeFunction>(
                0,
                [array](const std::vector<Value>&) -> Value {
                    if (array->size() > 0) {
                        return array->pop();
                    }
                    return nilValue(); // return nil for empty array
                },
                "pop"
            ));
        }
        
        // Handle array.reverse
        if (expr->member == "reverse") {
            return callableValue(std::make_shared<NativeFunction>(
                0,
                [array](const std::vector<Value>&) -> Value {
                    array->reverse();
                    return nilValue();
                },
                "reverse"
            ));
        }
        
        // Handle array.map
        if (expr->member == "map") {
            return callableValue(std::make_shared<NativeFunction>(
                1,
                [this, array](const std::vector<Value>& args) -> Value {
                    if (!isCallable(args[0])) {
                        throw std::runtime_error("E2001: map() requires a function argument");
                    }
                    
                    auto function = asCallable(args[0]);
                    auto newArray = std::make_shared<ClawArray>();
                    
                    for (size_t i = 0; i < array->size(); ++i) {
                        std::vector<Value> callArgs = { array->get(static_cast<int>(i)) };
                        newArray->push(function->call(*this, callArgs));
                    }
                    
                    return arrayValue(newArray);
                },
                "map"
            ));
        }
        
        // Handle array.filter
        if (expr->member == "filter") {
            return callableValue(std::make_shared<NativeFunction>(
                1,
                [this, array](const std::vector<Value>& args) -> Value {
                    if (!isCallable(args[0])) {
                        throw std::runtime_error("E2001: filter() requires a function argument");
                    }
                    
                    auto function = asCallable(args[0]);
                    auto newArray = std::make_shared<ClawArray>();
                    
                    for (size_t i = 0; i < array->size(); ++i) {
                        Value item = array->get(static_cast<int>(i));
                        std::vector<Value> callArgs = { item };
                        if (isTruthy(function->call(*this, callArgs))) {
                            newArray->push(item);
                        }
                    }
                    
                    return arrayValue(newArray);
                },
                "filter"
            ));
        }
        
        // Handle array.reduce
        if (expr->member == "reduce") {
            return callableValue(std::make_shared<NativeFunction>(
                2, // accumulator function and initial value
                [this, array](const std::vector<Value>& args) -> Value {
                    if (!isCallable(args[0])) {
                        throw std::runtime_error("E2001: reduce() requires a function argument");
                    }
                    
                    auto function = asCallable(args[0]);
                    Value accumulator = args[1];
                    
                    for (size_t i = 0; i < array->size(); ++i) {
                        std::vector<Value> callArgs = { accumulator, array->get(static_cast<int>(i)) };
                        accumulator = function->call(*this, callArgs);
                    }
                    
                    return accumulator;
                },
                "reduce"
            ));
        }
        
        // Handle array.forEach
        if (expr->member == "forEach") {
            return callableValue(std::make_shared<NativeFunction>(
                1,
                [this, array](const std::vector<Value>& args) -> Value {
                    if (!isCallable(args[0])) {
                        throw std::runtime_error("E2001: forEach() requires a function argument");
                    }
                    
                    auto function = asCallable(args[0]);
                    
                    for (size_t i = 0; i < array->size(); ++i) {
                        std::vector<Value> callArgs = { array->get(static_cast<int>(i)) };
                        function->call(*this, callArgs);
                    }
                    
                    return nilValue();
                },
                "forEach"
            ));
        }
        
        // Handle array.join
        if (expr->member == "join") {
            return callableValue(std::make_shared<NativeFunction>(
                1,
                [array](const std::vector<Value>& args) -> Value {
                    std::string separator = ", ";
                    if (!args.empty() && isString(args[0])) {
                        separator = asString(args[0]);
                    }
                    auto sv = StringPool::intern(array->join(separator));
                    return stringValue(sv.data());
                },
                "join"
            ));
        }
        
        // array.concat(other) — returns new array with elements of both
        if (expr->member == "concat") {
            auto array = asArray(object);
            return callableValue(std::make_shared<NativeFunction>(
                -1,
                [array](const std::vector<Value>& args) -> Value {
                    auto result = gcNewArray();
                    for (int i = 0; i < array->length(); i++) result->push(array->get(i));
                    for (const auto& arg : args) {
                        if (isArray(arg)) {
                            auto other = asArray(arg);
                            for (int i = 0; i < other->length(); i++) result->push(other->get(i));
                        } else {
                            result->push(arg);
                        }
                    }
                    return arrayValue(result);
                },
                "concat"
            ));
        }

        // array.flat(depth=1) — flatten nested arrays
        if (expr->member == "flat") {
            auto array = asArray(object);
            return callableValue(std::make_shared<NativeFunction>(
                -1,
                [array](const std::vector<Value>& args) -> Value {
                    int depth = (args.size() >= 1 && isNumber(args[0])) ? static_cast<int>(asNumber(args[0])) : 1;
                    std::function<void(std::shared_ptr<ClawArray>, std::shared_ptr<ClawArray>, int)> flatten;
                    flatten = [&flatten](std::shared_ptr<ClawArray> src, std::shared_ptr<ClawArray> dst, int d) {
                        for (int i = 0; i < src->length(); i++) {
                            Value v = src->get(i);
                            if (d > 0 && isArray(v)) flatten(asArray(v), dst, d - 1);
                            else dst->push(v);
                        }
                    };
                    auto result = gcNewArray();
                    flatten(array, result, depth);
                    return arrayValue(result);
                },
                "flat"
            ));
        }

        // array.indexOf(value) — find first index of value, -1 if not found
        if (expr->member == "indexOf") {
            auto array = asArray(object);
            return callableValue(std::make_shared<NativeFunction>(
                1,
                [array](const std::vector<Value>& args) -> Value {
                    for (int i = 0; i < array->length(); i++) {
                        if (isEqual(array->get(i), args[0])) return numberToValue(static_cast<double>(i));
                    }
                    return numberToValue(-1.0);
                },
                "indexOf"
            ));
        }

        // array.includes(value) — true if value is in array
        if (expr->member == "includes") {
            auto array = asArray(object);
            return callableValue(std::make_shared<NativeFunction>(
                1,
                [array](const std::vector<Value>& args) -> Value {
                    for (int i = 0; i < array->length(); i++) {
                        if (isEqual(array->get(i), args[0])) return boolValue(true);
                    }
                    return boolValue(false);
                },
                "includes"
            ));
        }

        // array.find(predicate) — first element matching predicate
        if (expr->member == "find") {
            auto array = asArray(object);
            return callableValue(std::make_shared<NativeFunction>(
                1,
                [array, this](const std::vector<Value>& args) -> Value {
                    if (args.empty() || !isCallable(args[0])) throw std::runtime_error("find() requires a function");
                    auto func = asCallable(args[0]);
                    for (int i = 0; i < array->length(); i++) {
                        Value el = array->get(i);
                        if (isTruthy(func->call(*this, {el}))) return el;
                    }
                    return nilValue();
                },
                "find"
            ));
        }

        // array.findIndex(predicate) — index of first element matching predicate
        if (expr->member == "findIndex") {
            auto array = asArray(object);
            return callableValue(std::make_shared<NativeFunction>(
                1,
                [array, this](const std::vector<Value>& args) -> Value {
                    if (args.empty() || !isCallable(args[0])) throw std::runtime_error("findIndex() requires a function");
                    auto func = asCallable(args[0]);
                    for (int i = 0; i < array->length(); i++) {
                        Value el = array->get(i);
                        if (isTruthy(func->call(*this, {el}))) return numberToValue(static_cast<double>(i));
                    }
                    return numberToValue(-1.0);
                },
                "findIndex"
            ));
        }

        // array.every(predicate) — true if all elements match
        if (expr->member == "every") {
            auto array = asArray(object);
            return callableValue(std::make_shared<NativeFunction>(
                1,
                [array, this](const std::vector<Value>& args) -> Value {
                    if (args.empty() || !isCallable(args[0])) throw std::runtime_error("every() requires a function");
                    auto func = asCallable(args[0]);
                    for (int i = 0; i < array->length(); i++) {
                        if (!isTruthy(func->call(*this, {array->get(i)}))) return boolValue(false);
                    }
                    return boolValue(true);
                },
                "every"
            ));
        }

        // array.some(predicate) — true if any element matches
        if (expr->member == "some") {
            auto array = asArray(object);
            return callableValue(std::make_shared<NativeFunction>(
                1,
                [array, this](const std::vector<Value>& args) -> Value {
                    if (args.empty() || !isCallable(args[0])) throw std::runtime_error("some() requires a function");
                    auto func = asCallable(args[0]);
                    for (int i = 0; i < array->length(); i++) {
                        if (isTruthy(func->call(*this, {array->get(i)}))) return boolValue(true);
                    }
                    return boolValue(false);
                },
                "some"
            ));
        }

        // array.fill(value, start=0, end=length) — fill range with value
        if (expr->member == "fill") {
            auto array = asArray(object);
            return callableValue(std::make_shared<NativeFunction>(
                -1,
                [array](const std::vector<Value>& args) -> Value {
                    if (args.empty()) throw std::runtime_error("fill() requires a value");
                    Value val = args[0];
                    int start = (args.size() >= 2 && isNumber(args[1])) ? static_cast<int>(asNumber(args[1])) : 0;
                    int end   = (args.size() >= 3 && isNumber(args[2])) ? static_cast<int>(asNumber(args[2])) : array->length();
                    if (start < 0) start = 0;
                    if (end > array->length()) end = array->length();
                    for (int i = start; i < end; i++) array->set(i, val);
                    return arrayValue(array);
                },
                "fill"
            ));
        }

        throwRuntimeError(expr->token, ErrorCode::UNDEFINED_VARIABLE, "Unknown array member: " + expr->member);
    }
    
    // Handle hash maps
    if (isHashMap(object)) {
        auto map = asHashMap(object);
        
        // Handle hash map properties/methods
        // If the map stores a callable under this key (e.g. Set/Queue/Stack methods),
        // prefer the stored value so user-defined collections can override built-ins.
        if (map->contains(expr->member)) {
            Value stored = map->get(expr->member);
            if (isCallable(stored)) return stored;
        }
        if (expr->member == "size") {
            return numberToValue(static_cast<double>(map->size()));
        }
        
        if (expr->member == "keys") {
            return callableValue(std::make_shared<NativeFunction>(
                0,
                [map](const std::vector<Value>&) -> Value {
                    auto keysVec = map->getKeys();
                    
                    auto resultArray = gcNewArray();
                    for (const auto& key : keysVec) {
                        auto sv = StringPool::intern(key);
                        resultArray->push(stringValue(sv.data()));
                    }
                    
                    return arrayValue(resultArray);
                },
                "hashmap.keys"
            ));
        }
        
        if (expr->member == "values") {
            return callableValue(std::make_shared<NativeFunction>(
                0,
                [map](const std::vector<Value>&) -> Value {
                    auto valuesVec = map->getValues();
                    
                    auto resultArray = gcNewArray();
                    for (const auto& value : valuesVec) {
                        resultArray->push(value);
                    }
                    
                    return arrayValue(resultArray);
                },
                "hashmap.values"
            ));
        }
        
        if (expr->member == "has") {
            return callableValue(std::make_shared<NativeFunction>(
                1,
                [map](const std::vector<Value>& args) -> Value {
                    // Convert key to string
                    std::string keyStr = valueToString(args[0]);
                    return boolValue(map->contains(keyStr));
                },
                "hashmap.has"
            ));
        }
        
        if (expr->member == "remove") {
            return callableValue(std::make_shared<NativeFunction>(
                1,
                [map](const std::vector<Value>& args) -> Value {
                    // Convert key to string
                    std::string keyStr = valueToString(args[0]);
                    return boolValue(map->remove(keyStr));  // Returns true if removed, false if not found
                },
                "hashmap.remove"
            ));
        }
        
        // Dynamic key lookup for hash maps
        if (map->contains(expr->member)) {
            return map->get(expr->member);
        }
        
        throwRuntimeError(expr->token, ErrorCode::UNDEFINED_VARIABLE, "Unknown hash map member: " + expr->member);
    }
    
    // Handle class instances — check __index metamethod if property not found
    if (isInstance(object)) {
        auto inst = asInstance(object);
        // Try direct property first
        try {
            return inst->get(expr->token);
        } catch (...) {}
        // Try __index metamethod
        Value out;
        if (tryMetamethod(object, "__index", {stringValue(StringPool::intern(expr->member).data())}, out)) {
            return out;
        }
        // Re-throw original error
        return inst->get(expr->token);
    }
    
   throwRuntimeError(expr->token, ErrorCode::NOT_INDEXABLE, "Only arrays, hash maps, and class instances have members");
}


void Interpreter::visitClassStmt(ClassStmt* stmt) {
    std::shared_ptr<ClawClass> superclass = nullptr;
    if (stmt->superclass) {
        Value super = evaluate(stmt->superclass.get());
        if (!isClass(super)) {
            throwRuntimeError(stmt->token, ErrorCode::RUNTIME_ERROR, "Superclass must be a class.");
        }
        superclass = asClass(super);
    }

    environment_->define(stmt->name, nilValue());

    // If there's a superclass, we create a new environment for the methods
    // that contains 'super'
    auto oldEnv = environment_;
    if (superclass) {
        environment_ = std::make_shared<Environment>(environment_);
        environment_->define("super", classValue(superclass));
    }

    std::unordered_map<std::string, std::shared_ptr<ClawFunction>> methods;
    for (auto& member : stmt->members) {
        if (member.method) {
            auto function = std::make_shared<ClawFunction>(
                member.method.get(), environment_, member.name == "init");
            methods[member.name] = function;
        }
    }

    auto cls = std::make_shared<ClawClass>(stmt->name, superclass, std::move(methods));

    if (superclass) {
        environment_ = oldEnv;
    }

    environment_->assign(stmt->name, classValue(cls));
}

Value Interpreter::visitHashMapExpr(HashMapExpr* expr) {
    auto hashMap = gcNewHashMap();
    
    for (const auto& [keyExpr, valueExpr] : expr->keyValuePairs) {
        Value key = evaluate(keyExpr.get());
        Value value = evaluate(valueExpr.get());
        
        // Convert key to string representation (for storage in hash map)
        std::string keyStr = valueToString(key);  // Use the same string representation as valueToString
        
        hashMap->set(keyStr, value);
    }
    
    return hashMapValue(hashMap);
}

Value Interpreter::visitFunctionExpr(FunctionExpr* expr) {
    // Create a function expression that can execute the function body
    // We'll store the parameters and a reference to the original function expression
    struct FunctionExpressionCallable : public Callable {
        std::vector<std::string> parameters;
        const FunctionExpr* func_expr;
        std::shared_ptr<Environment> closure;
        bool hasRest;

        FunctionExpressionCallable(std::vector<std::string> params,
                                 const FunctionExpr* expr,
                                 std::shared_ptr<Environment> env,
                                 bool rest = false)
            : parameters(std::move(params)), func_expr(expr), closure(std::move(env)), hasRest(rest) {}

        Value call(Interpreter& interp, const std::vector<Value>& arguments) override {
            // Create new environment for function execution
            auto functionEnv = std::make_shared<Environment>(closure);

            // Bind parameters to arguments, handling rest params
            if (hasRest && !parameters.empty()) {
                // Bind normal params
                size_t normalCount = parameters.size() - 1;
                for (size_t i = 0; i < normalCount && i < arguments.size(); i++) {
                    functionEnv->define(parameters[i], arguments[i]);
                }
                // Collect rest args into array
                auto restArr = std::make_shared<ClawArray>();
                for (size_t i = normalCount; i < arguments.size(); i++) {
                    restArr->push(arguments[i]);
                }
                functionEnv->define(parameters.back(), arrayValue(restArr));
            } else {
                for (size_t i = 0; i < parameters.size() && i < arguments.size(); i++) {
                    functionEnv->define(parameters[i], arguments[i]);
                }
            }
            
            // Push to call stack
            interp.getCallStack().push("<anonymous>", func_expr->token.line);

            // Save current environment and switch to function environment
            auto oldEnv = interp.environment_;
            interp.environment_ = functionEnv;
            
            // Execute function body
            Value result = nilValue();
            try {
                for (const auto& stmt : func_expr->body) {
                    interp.execute(stmt.get());
                }
                interp.getCallStack().pop();
            } catch (const ReturnValue& returnValue) {
                result = returnValue.value;
                // Restore environment before returning
                interp.getCallStack().pop();
                interp.environment_ = oldEnv;
                return result;
            } catch (...) {
                interp.getCallStack().pop();
                interp.environment_ = oldEnv;
                throw;
            }
            
            // Restore the original environment
            interp.environment_ = oldEnv;
            
            return result; // Return nil if no explicit return
        }
        
        int arity() const override {
            return hasRest ? -1 : static_cast<int>(parameters.size());
        }
        
        std::string toString() const override {
            return "<anonymous function>";
        }
    };
    
    return callableValue(std::make_shared<FunctionExpressionCallable>(
        expr->parameters, expr, environment_, expr->hasRest));
}

void Interpreter::checkNumberOperand(const Token& op, const Value& operand) {
    if (isNumber(operand)) return;
    throwRuntimeError(op, ErrorCode::TYPE_MISMATCH, "Operand must be a number");
}

void Interpreter::checkNumberOperands(const Token& op, const Value& left, const Value& right) {
    if (isNumber(left) && isNumber(right)) return;
    throwRuntimeError(op, ErrorCode::TYPE_MISMATCH, "Operands must be numbers");
}

// ============================================================
// Metamethod helpers
// ============================================================

bool Interpreter::tryMetamethod(Value obj, const std::string& name,
                                 const std::vector<Value>& args, Value& out) {
    if (!isInstance(obj)) return false;
    auto inst = asInstance(obj);
    if (!inst) return false;
    auto sv = StringPool::intern(name);
    Token tok(TokenType::Identifier, sv, 0);
    try {
        Value method = inst->get(tok);
        if (isCallable(method)) {
            out = asCallable(method)->call(*this, args);
            return true;
        }
    } catch (...) {}
    return false;
}

std::string Interpreter::valueToDisplayString(Value v) {
    if (isInstance(v)) {
        Value out;
        if (tryMetamethod(v, "__str", {}, out)) {
            return valueToString(out);
        }
    }
    return valueToString(v);
}

Value Interpreter::stringMethodDispatch(const std::string& str, const std::string& member,
                                         const Token& tok) {
    // Property: length
    if (member == "length") {
        return numberToValue(static_cast<double>(str.length()));
    }
    // upper() / toUpperCase()
    if (member == "upper" || member == "toUpperCase") {
        return callableValue(std::make_shared<NativeFunction>(0, [str](const std::vector<Value>&) -> Value {
            std::string s = str;
            for (auto& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            return stringValue(StringPool::intern(s).data());
        }, "upper"));
    }
    // lower() / toLowerCase()
    if (member == "lower" || member == "toLowerCase") {
        return callableValue(std::make_shared<NativeFunction>(0, [str](const std::vector<Value>&) -> Value {
            std::string s = str;
            for (auto& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            return stringValue(StringPool::intern(s).data());
        }, "lower"));
    }
    // trim()
    if (member == "trim") {
        return callableValue(std::make_shared<NativeFunction>(0, [str](const std::vector<Value>&) -> Value {
            size_t start = str.find_first_not_of(" \t\r\n");
            if (start == std::string::npos) return stringValue(StringPool::intern("").data());
            size_t end = str.find_last_not_of(" \t\r\n");
            return stringValue(StringPool::intern(str.substr(start, end - start + 1)).data());
        }, "trim"));
    }
    // trimStart() / trimLeft()
    if (member == "trimStart" || member == "trimLeft") {
        return callableValue(std::make_shared<NativeFunction>(0, [str](const std::vector<Value>&) -> Value {
            size_t start = str.find_first_not_of(" \t\r\n");
            if (start == std::string::npos) return stringValue(StringPool::intern("").data());
            return stringValue(StringPool::intern(str.substr(start)).data());
        }, "trimStart"));
    }
    // trimEnd() / trimRight()
    if (member == "trimEnd" || member == "trimRight") {
        return callableValue(std::make_shared<NativeFunction>(0, [str](const std::vector<Value>&) -> Value {
            size_t end = str.find_last_not_of(" \t\r\n");
            if (end == std::string::npos) return stringValue(StringPool::intern("").data());
            return stringValue(StringPool::intern(str.substr(0, end + 1)).data());
        }, "trimEnd"));
    }
    // split(sep)
    if (member == "split") {
        return callableValue(std::make_shared<NativeFunction>(-1, [str](const std::vector<Value>& args) -> Value {
            std::string sep = (args.size() >= 1 && isString(args[0])) ? asString(args[0]) : " ";
            auto result = gcNewArray();
            if (sep.empty()) {
                for (char c : str) result->push(stringValue(StringPool::intern(std::string(1, c)).data()));
                return arrayValue(result);
            }
            size_t pos = 0, found;
            while ((found = str.find(sep, pos)) != std::string::npos) {
                result->push(stringValue(StringPool::intern(str.substr(pos, found - pos)).data()));
                pos = found + sep.length();
            }
            result->push(stringValue(StringPool::intern(str.substr(pos)).data()));
            return arrayValue(result);
        }, "split"));
    }
    // replace(old, new)
    if (member == "replace") {
        return callableValue(std::make_shared<NativeFunction>(2, [str](const std::vector<Value>& args) -> Value {
            if (!isString(args[0]) || !isString(args[1]))
                throw std::runtime_error("replace() requires string arguments");
            std::string from = asString(args[0]);
            std::string to   = asString(args[1]);
            std::string result = str;
            size_t pos = 0;
            while ((pos = result.find(from, pos)) != std::string::npos) {
                result.replace(pos, from.length(), to);
                pos += to.length();
            }
            return stringValue(StringPool::intern(result).data());
        }, "replace"));
    }
    // replaceFirst(old, new) — replace only first occurrence
    if (member == "replaceFirst") {
        return callableValue(std::make_shared<NativeFunction>(2, [str](const std::vector<Value>& args) -> Value {
            if (!isString(args[0]) || !isString(args[1]))
                throw std::runtime_error("replaceFirst() requires string arguments");
            std::string from = asString(args[0]);
            std::string to   = asString(args[1]);
            std::string result = str;
            size_t pos = result.find(from);
            if (pos != std::string::npos) result.replace(pos, from.length(), to);
            return stringValue(StringPool::intern(result).data());
        }, "replaceFirst"));
    }
    // startsWith(prefix)
    if (member == "startsWith") {
        return callableValue(std::make_shared<NativeFunction>(1, [str](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) return boolValue(false);
            std::string prefix = asString(args[0]);
            return boolValue(str.length() >= prefix.length() &&
                             str.substr(0, prefix.length()) == prefix);
        }, "startsWith"));
    }
    // endsWith(suffix)
    if (member == "endsWith") {
        return callableValue(std::make_shared<NativeFunction>(1, [str](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) return boolValue(false);
            std::string suffix = asString(args[0]);
            if (suffix.length() > str.length()) return boolValue(false);
            return boolValue(str.substr(str.length() - suffix.length()) == suffix);
        }, "endsWith"));
    }
    // includes(sub) / contains(sub)
    if (member == "includes" || member == "contains") {
        return callableValue(std::make_shared<NativeFunction>(1, [str](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) return boolValue(false);
            return boolValue(str.find(asString(args[0])) != std::string::npos);
        }, "includes"));
    }
    // indexOf(sub)
    if (member == "indexOf") {
        return callableValue(std::make_shared<NativeFunction>(1, [str](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) return numberToValue(-1.0);
            size_t pos = str.find(asString(args[0]));
            return numberToValue(pos == std::string::npos ? -1.0 : static_cast<double>(pos));
        }, "indexOf"));
    }
    // lastIndexOf(sub)
    if (member == "lastIndexOf") {
        return callableValue(std::make_shared<NativeFunction>(1, [str](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) return numberToValue(-1.0);
            size_t pos = str.rfind(asString(args[0]));
            return numberToValue(pos == std::string::npos ? -1.0 : static_cast<double>(pos));
        }, "lastIndexOf"));
    }
    // slice(start, end?) / substring(start, end?) / substr(start, len?)
    if (member == "slice" || member == "substring") {
        return callableValue(std::make_shared<NativeFunction>(-1, [str](const std::vector<Value>& args) -> Value {
            int len = static_cast<int>(str.length());
            int start = (args.size() >= 1 && isNumber(args[0])) ? static_cast<int>(asNumber(args[0])) : 0;
            int end   = (args.size() >= 2 && isNumber(args[1])) ? static_cast<int>(asNumber(args[1])) : len;
            if (start < 0) start = std::max(0, len + start);
            if (end   < 0) end   = std::max(0, len + end);
            start = std::min(start, len);
            end   = std::min(end,   len);
            if (start >= end) return stringValue(StringPool::intern("").data());
            return stringValue(StringPool::intern(str.substr(start, end - start)).data());
        }, "slice"));
    }
    // repeat(n)
    if (member == "repeat") {
        return callableValue(std::make_shared<NativeFunction>(1, [str](const std::vector<Value>& args) -> Value {
            int n = (args.size() >= 1 && isNumber(args[0])) ? static_cast<int>(asNumber(args[0])) : 0;
            if (n <= 0) return stringValue(StringPool::intern("").data());
            std::string result;
            result.reserve(str.length() * static_cast<size_t>(n));
            for (int i = 0; i < n; i++) result += str;
            return stringValue(StringPool::intern(result).data());
        }, "repeat"));
    }
    // padStart(len, pad?) / padLeft(len, pad?)
    if (member == "padStart" || member == "padLeft") {
        return callableValue(std::make_shared<NativeFunction>(-1, [str](const std::vector<Value>& args) -> Value {
            int targetLen = (args.size() >= 1 && isNumber(args[0])) ? static_cast<int>(asNumber(args[0])) : 0;
            std::string pad = (args.size() >= 2 && isString(args[1])) ? asString(args[1]) : " ";
            if (pad.empty()) pad = " ";
            std::string result = str;
            while (static_cast<int>(result.length()) < targetLen) result = pad + result;
            if (static_cast<int>(result.length()) > targetLen)
                result = result.substr(result.length() - static_cast<size_t>(targetLen));
            return stringValue(StringPool::intern(result).data());
        }, "padStart"));
    }
    // padEnd(len, pad?) / padRight(len, pad?)
    if (member == "padEnd" || member == "padRight") {
        return callableValue(std::make_shared<NativeFunction>(-1, [str](const std::vector<Value>& args) -> Value {
            int targetLen = (args.size() >= 1 && isNumber(args[0])) ? static_cast<int>(asNumber(args[0])) : 0;
            std::string pad = (args.size() >= 2 && isString(args[1])) ? asString(args[1]) : " ";
            if (pad.empty()) pad = " ";
            std::string result = str;
            while (static_cast<int>(result.length()) < targetLen) result += pad;
            if (static_cast<int>(result.length()) > targetLen)
                result = result.substr(0, static_cast<size_t>(targetLen));
            return stringValue(StringPool::intern(result).data());
        }, "padEnd"));
    }
    // charAt(i)
    if (member == "charAt") {
        return callableValue(std::make_shared<NativeFunction>(1, [str](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) return stringValue(StringPool::intern("").data());
            int idx = static_cast<int>(asNumber(args[0]));
            if (idx < 0 || idx >= static_cast<int>(str.length()))
                return stringValue(StringPool::intern("").data());
            return stringValue(StringPool::intern(std::string(1, str[static_cast<size_t>(idx)])).data());
        }, "charAt"));
    }
    // charCodeAt(i)
    if (member == "charCodeAt") {
        return callableValue(std::make_shared<NativeFunction>(1, [str](const std::vector<Value>& args) -> Value {
            if (!isNumber(args[0])) return numberToValue(-1.0);
            int idx = static_cast<int>(asNumber(args[0]));
            if (idx < 0 || idx >= static_cast<int>(str.length())) return numberToValue(-1.0);
            return numberToValue(static_cast<double>(static_cast<unsigned char>(str[static_cast<size_t>(idx)])));
        }, "charCodeAt"));
    }
    // toNumber()
    if (member == "toNumber") {
        return callableValue(std::make_shared<NativeFunction>(0, [str](const std::vector<Value>&) -> Value {
            try { return numberToValue(std::stod(str)); }
            catch (...) { return numberToValue(std::nan("")); }
        }, "toNumber"));
    }
    // isEmpty()
    if (member == "isEmpty") {
        return callableValue(std::make_shared<NativeFunction>(0, [str](const std::vector<Value>&) -> Value {
            return boolValue(str.empty());
        }, "isEmpty"));
    }
    // reverse()
    if (member == "reverse") {
        return callableValue(std::make_shared<NativeFunction>(0, [str](const std::vector<Value>&) -> Value {
            std::string s = str;
            std::reverse(s.begin(), s.end());
            return stringValue(StringPool::intern(s).data());
        }, "reverse"));
    }
    // count(sub) — count occurrences
    if (member == "count") {
        return callableValue(std::make_shared<NativeFunction>(1, [str](const std::vector<Value>& args) -> Value {
            if (!isString(args[0])) return numberToValue(0.0);
            std::string sub = asString(args[0]);
            if (sub.empty()) return numberToValue(0.0);
            int count = 0;
            size_t pos = 0;
            while ((pos = str.find(sub, pos)) != std::string::npos) { ++count; pos += sub.length(); }
            return numberToValue(static_cast<double>(count));
        }, "count"));
    }
    // Unknown member — throw error
    throwRuntimeError(tok, ErrorCode::UNDEFINED_VARIABLE,
        "String has no member '" + member + "'");
}

// ============================================================
// New ExprVisitor implementations
// ============================================================

Value Interpreter::visitFStringExpr(FStringExpr* expr) {
    std::string result;
    for (auto& seg : expr->segments) {
        if (seg.isExpr) {
            result += valueToString(evaluate(seg.expr.get()));
        } else {
            result += seg.text;
        }
    }
    return stringValue(StringPool::intern(result).data());
}

Value Interpreter::visitTemplateExpr(TemplateExpr* expr) {
    std::string result;
    for (auto& seg : expr->segments) {
        if (seg.isExpr) {
            result += valueToString(evaluate(seg.expr.get()));
        } else {
            result += seg.text;
        }
    }
    return stringValue(StringPool::intern(result).data());
}

Value Interpreter::visitSpreadExpr(SpreadExpr* expr) {
    // Spread is handled by the call site; here just evaluate the inner expr
    return evaluate(expr->expr.get());
}

Value Interpreter::visitOptionalChainExpr(OptionalChainExpr* expr) {
    Value obj = evaluate(expr->object.get());
    if (isNil(obj)) return nilValue();

    switch (expr->kind) {
        case OptionalChainExpr::Kind::Member: {
            if (isHashMap(obj)) return asHashMap(obj)->get(expr->member);
            if (isInstance(obj)) {
                auto sv = StringPool::intern(expr->member);
                Token tok(TokenType::Identifier, sv, expr->token.line);
                return asInstance(obj)->get(tok);
            }
            return nilValue();
        }
        case OptionalChainExpr::Kind::Index: {
            Value idx = evaluate(expr->index.get());
            if (isArray(obj) && isNumber(idx)) {
                int i = static_cast<int>(asNumber(idx));
                auto arr = asArray(obj);
                if (i < 0 || i >= arr->length()) return nilValue();
                return arr->get(i);
            }
            if (isHashMap(obj)) return asHashMap(obj)->get(valueToString(idx));
            return nilValue();
        }
        case OptionalChainExpr::Kind::Call: {
            if (!isCallable(obj)) return nilValue();
            std::vector<Value> args;
            for (auto& a : expr->args) args.push_back(evaluate(a.get()));
            return asCallable(obj)->call(*this, args);
        }
    }
    return nilValue();
}

Value Interpreter::visitNullCoalesceExpr(NullCoalesceExpr* expr) {
    Value left = evaluate(expr->left.get());
    if (!isNil(left)) return left;
    return evaluate(expr->right.get());
}

Value Interpreter::visitPipeExpr(PipeExpr* expr) {
    Value arg = evaluate(expr->left.get());
    Value fn  = evaluate(expr->right.get());
    if (!isCallable(fn)) {
        throwRuntimeError(expr->token, ErrorCode::NOT_CALLABLE,
            "Right side of |> must be callable");
    }
    return asCallable(fn)->call(*this, {arg});
}

Value Interpreter::visitAwaitExpr(AwaitExpr* expr) {
    // Synchronous mode: await is a no-op, just evaluate the inner expression
    return evaluate(expr->expr.get());
}

Value Interpreter::visitYieldExpr(YieldExpr* expr) {
    Value value = expr->expr ? evaluate(expr->expr.get()) : nilValue();
    // Check if we are inside a running coroutine
    claw::Coroutine* activeCo = claw::Coroutine::current_;
    if (activeCo) {
        return claw::Coroutine::doYield(value);
    }
    return value;
}

Value Interpreter::visitMatchExpr(MatchExpr* expr) {
    Value subject = evaluate(expr->subject.get());
    for (auto& arm : expr->arms) {
        if (arm.isDefault) {
            if (arm.bodyExpr) return evaluate(arm.bodyExpr.get());
            if (!arm.body.empty()) {
                executeBlock(arm.body, std::make_shared<Environment>(environment_));
            }
            return nilValue();
        }
        for (auto& pat : arm.patterns) {
            Value patVal = evaluate(pat.get());
            if (arm.guard) {
                if (!isTruthy(evaluate(arm.guard.get()))) continue;
            }
            if (isEqual(subject, patVal)) {
                if (arm.bodyExpr) return evaluate(arm.bodyExpr.get());
                if (!arm.body.empty()) {
                    executeBlock(arm.body, std::make_shared<Environment>(environment_));
                }
                return nilValue();
            }
        }
    }
    return nilValue();
}

Value Interpreter::visitComprehensionExpr(ComprehensionExpr* expr) {
    Value iterableVal = evaluate(expr->iterable.get());
    auto result = gcNewArray();

    auto iterate = [&](const std::function<void(Value)>& fn) {
        if (isArray(iterableVal)) {
            auto arr = asArray(iterableVal);
            for (int i = 0; i < arr->length(); ++i) fn(arr->get(i));
        } else if (isHashMap(iterableVal)) {
            auto map = asHashMap(iterableVal);
            for (auto& k : map->getKeys()) {
                fn(stringValue(StringPool::intern(k).data()));
            }
        }
    };

    iterate([&](Value item) {
        auto loopEnv = std::make_shared<Environment>(environment_);
        loopEnv->define(expr->varName, item);
        auto prev = environment_;
        environment_ = loopEnv;
        bool pass = true;
        if (expr->condition) pass = isTruthy(evaluate(expr->condition.get()));
        if (pass) result->push(evaluate(expr->body.get()));
        environment_ = prev;
    });

    return arrayValue(result);
}

Value Interpreter::visitDestructureArrayExpr(DestructureArrayExpr* expr) {
    Value val = evaluate(expr->value.get());
    if (!isArray(val)) {
        throwRuntimeError(expr->token, ErrorCode::TYPE_MISMATCH,
            "Array destructuring requires an array");
    }
    auto arr = asArray(val);
    for (size_t i = 0; i < expr->elements.size(); ++i) {
        auto& elem = expr->elements[i];
        if (elem.isRest) {
            auto rest = gcNewArray();
            for (int j = static_cast<int>(i); j < arr->length(); ++j)
                rest->push(arr->get(j));
            environment_->define(elem.name, arrayValue(rest));
            break;
        }
        Value v = (static_cast<int>(i) < arr->length()) ? arr->get(static_cast<int>(i)) : nilValue();
        if (isNil(v) && elem.defaultVal) v = evaluate(elem.defaultVal.get());
        environment_->define(elem.name, v);
    }
    return val;
}

Value Interpreter::visitDestructureObjectExpr(DestructureObjectExpr* expr) {
    Value val = evaluate(expr->value.get());
    for (auto& prop : expr->properties) {
        if (prop.isRest) continue; // TODO
        Value v = nilValue();
        if (isHashMap(val)) v = asHashMap(val)->get(prop.key);
        else if (isInstance(val)) {
            auto sv = StringPool::intern(prop.key);
            Token tok(TokenType::Identifier, sv, expr->token.line);
            v = asInstance(val)->get(tok);
        }
        if (isNil(v) && prop.defaultVal) v = evaluate(prop.defaultVal.get());
        std::string bindName = prop.alias.empty() ? prop.key : prop.alias;
        environment_->define(bindName, v);
    }
    return val;
}

Value Interpreter::visitTypeAnnotationExpr(TypeAnnotationExpr* expr) {
    return evaluate(expr->expr.get());
}

Value Interpreter::visitNewExpr(NewExpr* expr) {
    Value callee = evaluate(expr->callee.get());
    std::vector<Value> args;
    for (auto& a : expr->arguments) args.push_back(evaluate(a.get()));
    if (isClass(callee)) {
        return asClass(callee)->call(*this, args);
    }
    if (isCallable(callee)) {
        return asCallable(callee)->call(*this, args);
    }
    throwRuntimeError(expr->token, ErrorCode::NOT_CALLABLE, "new requires a class");
    return nilValue(); // unreachable
}

Value Interpreter::visitMetaExpr(MetaExpr* expr) {
    // Metatables not yet implemented; return nil
    (void)expr;
    return nilValue();
}

// ============================================================
// New StmtVisitor implementations
// ============================================================

void Interpreter::visitConstStmt(ConstStmt* stmt) {
    Value value = nilValue();
    if (stmt->initializer) value = evaluate(stmt->initializer.get());
    environment_->define(stmt->name, value);
}

void Interpreter::visitEnumStmt(EnumStmt* stmt) {
    auto map = gcNewHashMap();
    double idx = 0.0;
    for (auto& mem : stmt->members) {
        Value v = mem.value ? evaluate(mem.value.get()) : numberToValue(idx);
        map->set(mem.name, v);
        idx += 1.0;
    }
    environment_->define(stmt->name, hashMapValue(map));
}

void Interpreter::visitInterfaceStmt(InterfaceStmt*) {
    // Interfaces are structural — no runtime representation needed
}

void Interpreter::visitForOfStmt(ForOfStmt* stmt) {
    Value iterableVal = evaluate(stmt->iterable.get());

    auto runBody = [&](Value item) -> bool {
        auto loopEnv = std::make_shared<Environment>(environment_);
        loopEnv->define(stmt->varName, item);
        auto prev = environment_;
        environment_ = loopEnv;
        bool shouldBreak = false;
        try {
            execute(stmt->body.get());
        } catch (const BreakException&) {
            shouldBreak = true;
        } catch (const ContinueException&) {
            // continue to next iteration
        }
        environment_ = prev;
        return !shouldBreak;
    };

    if (isArray(iterableVal)) {
        auto arr = asArray(iterableVal);
        for (int i = 0; i < arr->length(); ++i)
            if (!runBody(arr->get(i))) break;
    } else if (isHashMap(iterableVal)) {
        // Check if this is a coroutine (has "resume" and "isDead" keys)
        auto map = asHashMap(iterableVal);
        bool isCoroObj = map->contains("resume") && map->contains("isDead");
        if (isCoroObj) {
            // Coroutine generator protocol: resume until dead, skip nil return on completion
            Value resumeFn = map->get("resume");
            Value isDeadFn = map->get("isDead");
            while (true) {
                // Check isDead()
                Value deadVal = nilValue();
                if (isCallable(isDeadFn)) {
                    deadVal = asCallable(isDeadFn)->call(*this, {});
                }
                if (isTruthy(deadVal)) break;
                // Resume
                Value yielded = nilValue();
                if (isCallable(resumeFn)) {
                    yielded = asCallable(resumeFn)->call(*this, {});
                }
                // After resume, check dead again — if now dead, the yielded value
                // is the return value (not a yielded value), so skip it
                Value deadAfter = nilValue();
                if (isCallable(isDeadFn)) {
                    deadAfter = asCallable(isDeadFn)->call(*this, {});
                }
                if (isTruthy(deadAfter)) break;
                if (!runBody(yielded)) break;
            }
        } else {
            for (auto& k : map->getKeys())
                if (!runBody(stringValue(StringPool::intern(k).data()))) break;
        }
    } else if (isString(iterableVal)) {
        std::string s = asString(iterableVal);
        for (char c : s)
            if (!runBody(stringValue(StringPool::intern(std::string(1, c)).data()))) break;
    }
}

void Interpreter::visitForInStmt(ForInStmt* stmt) {
    Value objVal = evaluate(stmt->object.get());

    auto runBody = [&](Value key) -> bool {
        auto loopEnv = std::make_shared<Environment>(environment_);
        loopEnv->define(stmt->varName, key);
        auto prev = environment_;
        environment_ = loopEnv;
        bool shouldBreak = false;
        try {
            execute(stmt->body.get());
        } catch (const BreakException&) {
            shouldBreak = true;
        } catch (const ContinueException&) {}
        environment_ = prev;
        return !shouldBreak;
    };

    if (isHashMap(objVal)) {
        auto map = asHashMap(objVal);
        for (auto& k : map->getKeys())
            if (!runBody(stringValue(StringPool::intern(k).data()))) break;
    } else if (isArray(objVal)) {
        auto arr = asArray(objVal);
        for (int i = 0; i < arr->length(); ++i)
            if (!runBody(numberToValue(static_cast<double>(i)))) break;
    }
}

void Interpreter::visitDeferStmt(DeferStmt* stmt) {
    // Simple eager execution (full defer stack not yet implemented)
    execute(stmt->body.get());
}

void Interpreter::visitAsyncFnStmt(AsyncFnStmt* stmt) {
    if (stmt->fn) visitFnStmt(stmt->fn.get());
}

void Interpreter::visitWithStmt(WithStmt* stmt) {
    Value resource = evaluate(stmt->resource.get());
    auto withEnv = std::make_shared<Environment>(environment_);
    withEnv->define(stmt->varName, resource);
    auto prev = environment_;
    environment_ = withEnv;
    try {
        execute(stmt->body.get());
    } catch (...) {
        environment_ = prev;
        throw;
    }
    environment_ = prev;
}

void Interpreter::visitLabeledStmt(LabeledStmt* stmt) {
    execute(stmt->body.get());
}

void Interpreter::visitMultiLetStmt(MultiLetStmt* stmt) {
    Value val = stmt->initializer ? evaluate(stmt->initializer.get()) : nilValue();
    for (size_t i = 0; i < stmt->names.size(); ++i) {
        Value v = nilValue();
        if (isArray(val)) {
            auto arr = asArray(val);
            if (static_cast<int>(i) < arr->length()) v = arr->get(static_cast<int>(i));
        }
        environment_->define(stmt->names[i], v);
    }
}

void Interpreter::visitExportStmt(ExportStmt* stmt) {
    // Export is handled by the module system; no-op at interpreter level
    (void)stmt;
}

void Interpreter::visitDecoratorStmt(DecoratorStmt* stmt) {
    if (stmt->target) execute(stmt->target.get());
}

} // namespace claw
