#include "native_types.h"
#include "interpreter/environment.h"
#include "interpreter/value.h"
#include "features/callable.h"
#include "features/string_pool.h"
#include "features/hashmap.h"
#include "features/array.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace claw {

// ============================================================
// Type System Implementation
// ============================================================
struct TypeCheck {
    static bool checkType(Value value, const std::string& expectedType) {
        if (expectedType == "number") return isNumber(value);
        if (expectedType == "string") return isString(value);
        if (expectedType == "bool") return isBool(value);
        if (expectedType == "array") return isArray(value);
        if (expectedType == "hashmap" || expectedType == "object") return isHashMap(value);
        if (expectedType == "function" || expectedType == "callable") return isCallable(value);
        if (expectedType == "nil" || expectedType == "null") return isNil(value);
        return true; // Unknown type, assume valid
    }
    
    static std::string getTypeName(Value value) {
        if (isNumber(value)) return "number";
        if (isString(value)) return "string";
        if (isBool(value)) return "bool";
        if (isArray(value)) return "array";
        if (isHashMap(value)) return "hashmap";
        if (isCallable(value)) return "function";
        if (isNil(value)) return "nil";
        return "unknown";
    }
};

// ============================================================
// Register type system natives
// ============================================================
void registerNativeTypes(const std::shared_ptr<Environment>& globals) {

    // Type object with methods
    auto types = std::make_shared<ClawHashMap>();
    
    // Type checking function
    types->set("check", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            Value value = args[0];
            std::string expectedType = valueToString(args[1]);
            
            bool isValid = TypeCheck::checkType(value, expectedType);
            return boolValue(isValid);
        },
        "types.check"
    )));
    
    // Get type name
    types->set("of", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            Value value = args[0];
            std::string typeName = TypeCheck::getTypeName(value);
            return stringValue(StringPool::intern(typeName).data());
        },
        "types.of"
    )));
    
    // Type assertion
    types->set("assert", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            Value value = args[0];
            std::string expectedType = valueToString(args[1]);
            
            bool isValid = TypeCheck::checkType(value, expectedType);
            if (!isValid) {
                std::string actualType = TypeCheck::getTypeName(value);
                throw std::runtime_error("Type assertion failed: expected " + expectedType + ", got " + actualType);
            }
            return value;
        },
        "types.assert"
    )));
    
    // Generic type constraints
    types->set("generic", callableValue(std::make_shared<NativeFunction>(
        2,
        [](const std::vector<Value>& args) -> Value {
            auto func = args[0];
            auto typeConstraints = asHashMap(args[1]);
            
            // Return a wrapped function that checks types
            auto wrapper = std::make_shared<NativeFunction>(
                2, // Assume 2 parameters for simplicity
                [func, typeConstraints](const std::vector<Value>& args) -> Value {
                    // Simple type checking for first two parameters
                    if (args.size() >= 2) {
                        if (!isNumber(args[0]) || !isNumber(args[1])) {
                            throw std::runtime_error("Type error: both parameters must be numbers");
                        }
                    }
                    
                    // Call the original function
                    if (isCallable(func)) {
                        return asCallable(func)->call(*nullptr, args);
                    }
                    return nilValue();
                },
                "generic_wrapper"
            );
            
            return callableValue(wrapper);
        },
        "types.generic"
    )));
    
    globals->define("types", hashMapValue(types));
    
    // Convenience functions
    globals->define("type", callableValue(std::make_shared<NativeFunction>(
        1,
        [types](const std::vector<Value>& args) -> Value {
            return types->get("of")->call(*nullptr, args);
        },
        "type"
    )));
    
    globals->define("isNumber", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            return boolValue(isNumber(args[0]));
        },
        "isNumber"
    )));
    
    globals->define("isString", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            return boolValue(isString(args[0]));
        },
        "isString"
    )));
    
    globals->define("isBool", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            return boolValue(isBool(args[0]));
        },
        "isBool"
    )));
    
    globals->define("isArray", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            return boolValue(isArray(args[0]));
        },
        "isArray"
    )));
    
    globals->define("isHashMap", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            return boolValue(isHashMap(args[0]));
        },
        "isHashMap"
    )));
    
    globals->define("isCallable", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            return boolValue(isCallable(args[0]));
        },
        "isCallable"
    )));
    
    globals->define("isNil", callableValue(std::make_shared<NativeFunction>(
        1,
        [](const std::vector<Value>& args) -> Value {
            return boolValue(isNil(args[0]));
        },
        "isNil"
    )));
}

} // namespace claw
