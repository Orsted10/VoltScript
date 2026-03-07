#include "callable.h"
#include "interpreter.h"
#include "stmt.h"
#include "environment.h"
#include "stack_trace.h"
#include "class.h"
#include "interpreter/gc_alloc.h"
#include <sstream>

namespace claw {

// ========================================
// ClawFunction (User-defined functions)
// ========================================

ClawFunction::ClawFunction(FnStmt* declaration, 
                           std::shared_ptr<Environment> closure,
                           bool isInitializer)
    : declaration_(declaration), closure_(closure), isInitializer_(isInitializer) {}

std::shared_ptr<ClawFunction> ClawFunction::bind(std::shared_ptr<ClawInstance> instance) {
    auto environment = std::make_shared<Environment>(closure_);
    environment->define("this", instanceValue(instance));
    return std::make_shared<ClawFunction>(declaration_, environment, isInitializer_);
}

Value ClawFunction::call(Interpreter& interpreter, 
                        const std::vector<Value>& arguments) {
    // Create a new environment for this function call
    // The closure is the parent (so we can access captured variables)
    auto environment = std::make_shared<Environment>(closure_);
    
    // Bind parameters to arguments — handle rest parameters
    const auto& params = declaration_->params;
    for (size_t i = 0; i < params.size(); i++) {
        if (params[i].isRest) {
            // Collect all remaining arguments into an array
            auto restArr = gcNewArray();
            for (size_t j = i; j < arguments.size(); j++) {
                restArr->push(arguments[j]);
            }
            environment->define(params[i].name, arrayValue(restArr));
            break; // rest must be last param
        }
        Value val = (i < arguments.size()) ? arguments[i] : nilValue();
        // Apply default value if argument is nil and default exists
        if (isNil(val) && params[i].defaultValue) {
            val = interpreter.evaluate(params[i].defaultValue.get());
        }
        environment->define(params[i].name, val);
    }
    
    // Push to call stack
    try {
        interpreter.getCallStack().push(declaration_->name, declaration_->token.line);
    } catch (const std::runtime_error& e) {
        throw RuntimeError(declaration_->token, ErrorCode::STACK_OVERFLOW, e.what(), interpreter.getCallStack().get_frames());
    }
    
    // Execute the function body
    try {
        interpreter.executeBlock(declaration_->body, environment);
        interpreter.getCallStack().pop();
    } catch (const ReturnValue& returnValue) {
        // Return statement throws a special exception with the value
        interpreter.getCallStack().pop();
        
        // If it's an initializer, we always return 'this' (the instance)
        if (isInitializer_) return closure_->get("this");
        
        return returnValue.value;
    } catch (...) {
        // Ensure we pop even on other exceptions (like RuntimeErrors)
        interpreter.getCallStack().pop();
        throw;
    }
    
    // If it's an initializer, we always return 'this' (the instance)
    if (isInitializer_) return closure_->get("this");
    
    // If no return statement, functions return nil
    return nilValue();
}

int ClawFunction::arity() const {
    // If any parameter is a rest param, accept variable number of arguments
    for (const auto& p : declaration_->params) {
        if (p.isRest) return -1;
    }
    return static_cast<int>(declaration_->params.size());
}

std::string ClawFunction::toString() const {
    return "<fn " + declaration_->name + ">";
}

// ========================================
// NativeFunction (Built-in C++ functions)
// ========================================

NativeFunction::NativeFunction(int arity, NativeFn function, std::string name)
    : arity_(arity), function_(function), name_(std::move(name)) {}

Value NativeFunction::call(Interpreter& interpreter, 
                          const std::vector<Value>& arguments) {
    // Just call the C++ function we wrapped
    try {
        interpreter.getCallStack().push(name_, -1); // Native functions don't have a line number
    } catch (const std::runtime_error&) {
        // For native functions, we don't have a token easily available here
        // but it will likely be caught by the caller's push
        throw;
    }

    try {
        Value result = function_(arguments);
        interpreter.getCallStack().pop();
        return result;
    } catch (...) {
        interpreter.getCallStack().pop();
        throw;
    }
}

int NativeFunction::arity() const {
    return arity_;
}

std::string NativeFunction::toString() const {
    return "<native fn " + name_ + ">";
}

} // namespace claw
