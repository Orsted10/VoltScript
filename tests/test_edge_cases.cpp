#include <gtest/gtest.h>
#include "interpreter/interpreter.h"
#include "compiler/compiler.h"
#include "vm/vm.h"
#include "interpreter/value.h"
#include <limits>

using namespace claw;

class EdgeCasesTest : public ::testing::Test {
protected:
    void SetUp() override {
        interpreter = std::make_unique<Interpreter>();
    }
    
    std::unique_ptr<Interpreter> interpreter;
};

// Test numeric edge cases
TEST_F(EdgeCasesTest, NumericEdgeCases) {
    // Test very large numbers
    Value result = interpreter->evaluate("9007199254740992 + 1");
    EXPECT_TRUE(result.isNumber());
    
    // Test very small numbers
    result = interpreter->evaluate("0.000000000000001 * 0.000000000000001");
    EXPECT_TRUE(result.isNumber());
    
    // Test infinity and NaN handling
    result = interpreter->evaluate("1 / 0");
    EXPECT_TRUE(result.isNumber());
    EXPECT_TRUE(std::isinf(result.asNumber()));
    
    // Test negative zero
    result = interpreter->evaluate("-0");
    EXPECT_TRUE(result.isNumber());
    EXPECT_EQ(result.asNumber(), -0.0);
    
    // Test maximum safe integer
    result = interpreter->evaluate("9007199254740991");
    EXPECT_TRUE(result.isNumber());
    EXPECT_EQ(result.asNumber(), 9007199254740991.0);
}

// Test string edge cases
TEST_F(EdgeCasesTest, StringEdgeCases) {
    // Test empty string operations
    Value result = interpreter->evaluate("\"\".length");
    EXPECT_EQ(result.asNumber(), 0);
    
    result = interpreter->evaluate("\"\" + \"test\"");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "test");
    
    // Test very long string
    std::string longString(10000, 'x');
    interpreter->evaluate("let longStr = \"" + longString + "\"");
    result = interpreter->evaluate("longStr.length");
    EXPECT_EQ(result.asNumber(), 10000);
    
    // Test string with special characters
    result = interpreter->evaluate("\"Hello\\nWorld\\t!\\\"Quote\\'\"");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "Hello\nWorld\t!\"Quote'");
    
    // Test Unicode characters
    result = interpreter->evaluate("\"Hello 世界 🌍\"");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "Hello 世界 🌍");
}

// Test array edge cases
TEST_F(EdgeCasesTest, ArrayEdgeCases) {
    // Test empty array
    Value result = interpreter->evaluate("[].length");
    EXPECT_EQ(result.asNumber(), 0);
    
    result = interpreter->evaluate("[].push(42)");
    EXPECT_EQ(result.asNumber(), 1);
    
    // Test very large array
    interpreter->evaluate(R"(
let largeArray = [];
for (let i = 0; i < 100000; i = i + 1) {
    largeArray.push(i);
}
)");
    result = interpreter->evaluate("largeArray.length");
    EXPECT_EQ(result.asNumber(), 100000);
    
    // Test sparse array
    interpreter->evaluate("let sparse = []; sparse[1000] = 42;");
    result = interpreter->evaluate("sparse[1000]");
    EXPECT_EQ(result.asNumber(), 42);
    
    result = interpreter->evaluate("sparse[999]");
    EXPECT_TRUE(result.isNil());
    
    // Test array with mixed types
    interpreter->evaluate("let mixed = [42, \"hello\", true, nil, [1,2,3], {\"key\": \"value\"}]");
    result = interpreter->evaluate("mixed[0]");
    EXPECT_EQ(result.asNumber(), 42);
    
    result = interpreter->evaluate("mixed[1]");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "hello");
    
    result = interpreter->evaluate("mixed[2]");
    EXPECT_TRUE(result.isBool());
    EXPECT_TRUE(result.asBool());
    
    result = interpreter->evaluate("mixed[3]");
    EXPECT_TRUE(result.isNil());
}

// Test object edge cases
TEST_F(EdgeCasesTest, ObjectEdgeCases) {
    // Test empty object
    Value result = interpreter->evaluate("{}");
    EXPECT_TRUE(result.isObject());
    
    // Test object with special keys
    interpreter->evaluate("let special = {\"\": \"empty\", \" \": \"space\", \"\\n\": \"newline\"}");
    result = interpreter->evaluate("special[\"\"]");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "empty");
    
    // Test object with numeric keys
    interpreter->evaluate("let numericKeys = {0: \"zero\", 1: \"one\", \"2\": \"two\"}");
    result = interpreter->evaluate("numericKeys[0]");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "zero");
    
    result = interpreter->evaluate("numericKeys[\"2\"]");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "two");
    
    // Test deeply nested object
    interpreter->evaluate(R"(
let deep = {};
let current = deep;
for (let i = 0; i < 1000; i = i + 1) {
    current.next = {};
    current = current.next;
}
current.value = "deep value";
)");
    result = interpreter->evaluate("deep.next.next.next.value");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "deep value");
}

// Test function edge cases
TEST_F(EdgeCasesTest, FunctionEdgeCases) {
    // Test function with no parameters
    interpreter->evaluate("fn noParams() { return 42; }");
    Value result = interpreter->evaluate("noParams()");
    EXPECT_EQ(result.asNumber(), 42);
    
    // Test function with many parameters
    interpreter->evaluate("fn manyParams(a,b,c,d,e,f,g,h,i,j) { return a+b+c+d+e+f+g+h+i+j; }");
    result = interpreter->evaluate("manyParams(1,2,3,4,5,6,7,8,9,10)");
    EXPECT_EQ(result.asNumber(), 55);
    
    // Test recursive function with deep recursion
    interpreter->evaluate("fn deepRecursion(n) { if (n <= 0) return 0; return deepRecursion(n - 1); }");
    
    // Should handle reasonable recursion depth
    EXPECT_THROW(interpreter->evaluate("deepRecursion(10000)"), std::runtime_error);
    
    // Test function returning different types
    interpreter->evaluate(R"(
fn returnType(type) {
    if (type === "number") return 42;
    if (type === "string") return "hello";
    if (type === "bool") return true;
    if (type === "array") return [1,2,3];
    if (type === "object") return {"key": "value"};
    return nil;
}
)");
    
    result = interpreter->evaluate("returnType(\"number\")");
    EXPECT_EQ(result.asNumber(), 42);
    
    result = interpreter->evaluate("returnType(\"string\")");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "hello");
}

// Test control flow edge cases
TEST_F(EdgeCasesTest, ControlFlowEdgeCases) {
    // Test nested loops with break/continue
    interpreter->evaluate(R"(
let result = 0;
for (let i = 0; i < 10; i = i + 1) {
    if (i === 5) break;
    for (let j = 0; j < 10; j = j + 1) {
        if (j === 3) continue;
        result = result + 1;
    }
}
)");
    Value result_val = interpreter->evaluate("result");
    // 5 iterations of outer loop * 7 iterations of inner loop (j=0,1,2,4,5,6,7,8,9 but continue at j=3)
    EXPECT_EQ(result_val.asNumber(), 40);
    
    // Test ternary operator chaining
    result = interpreter->evaluate("true ? 1 : false ? 2 : 3");
    EXPECT_EQ(result.asNumber(), 1);
    
    result = interpreter->evaluate("false ? 1 : true ? 2 : 3");
    EXPECT_EQ(result.asNumber(), 2);
    
    result = interpreter->evaluate("false ? 1 : false ? 2 : 3");
    EXPECT_EQ(result.asNumber(), 3);
}

// Test class edge cases
TEST_F(EdgeCasesTest, ClassEdgeCases) {
    // Test class without init method
    interpreter->evaluate(R"(
class NoInit {
    method() {
        return 42;
    }
}
)");
    
    Value result = interpreter->evaluate("let obj = NoInit(); obj.method()");
    EXPECT_EQ(result.asNumber(), 42);
    
    // Test class inheritance chain
    interpreter->evaluate(R"(
class A {
    init() { this.value = "A"; }
    method() { return "A.method"; }
}
class B extends A {
    init() { super.init(); this.value = "B"; }
    method() { return "B.method"; }
}
class C extends B {
    init() { super.init(); this.value = "C"; }
    method() { return "C.method"; }
}
)");
    
    interpreter->evaluate("let c = C();");
    result = interpreter->evaluate("c.value");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "C");
    
    result = interpreter->evaluate("c.method()");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "C.method");
}

// Test error handling edge cases
TEST_F(EdgeCasesTest, ErrorHandlingEdgeCases) {
    // Test division by zero in different contexts
    EXPECT_THROW(interpreter->evaluate("1 / 0"), std::runtime_error);
    EXPECT_THROW(interpreter->evaluate("0 / 0"), std::runtime_error);
    EXPECT_THROW(interpreter->evaluate("-1 / 0"), std::runtime_error);
    
    // Test invalid operations
    EXPECT_THROW(interpreter->evaluate("nil + 42"), std::runtime_error);
    EXPECT_THROW(interpreter->evaluate("true * false"), std::runtime_error);
    EXPECT_THROW(interpreter->evaluate("\"string\" - 42"), std::runtime_error);
    EXPECT_THROW(interpreter->evaluate("42 / \"string\""), std::runtime_error);
    
    // Test out of bounds access
    interpreter->evaluate("let arr = [1,2,3];");
    EXPECT_THROW(interpreter->evaluate("arr[3]"), std::runtime_error);
    EXPECT_THROW(interpreter->evaluate("arr[-1]"), std::runtime_error);
    
    // Test undefined properties
    interpreter->evaluate("let obj = {\"defined\": 42};");
    EXPECT_THROW(interpreter->evaluate("obj.undefined"), std::runtime_error);
}

// Test memory edge cases
TEST_F(EdgeCasesTest, MemoryEdgeCases) {
    // Test memory exhaustion simulation
    EXPECT_THROW(interpreter->evaluate(R"(
let hugeArray = [];
for (let i = 0; i < 10000000; i = i + 1) {
    hugeArray.push(new Array(1000).fill(i));
}
)'), std::runtime_error);
    
    // Test circular reference cleanup
    interpreter->evaluate(R"(
class Circular {
    init() {
        this.self = this;
    }
}
let circular = Circular();
circular = nil; // Break reference
)");
    
    // Should not crash after garbage collection
    Value result = interpreter->evaluate("42");
    EXPECT_EQ(result.asNumber(), 42);
}

// Test type conversion edge cases
TEST_F(EdgeCasesTest, TypeConversionEdgeCases) {
    // Test string to number conversion
    Value result = interpreter->evaluate("num(\"42\")");
    EXPECT_EQ(result.asNumber(), 42);
    
    result = interpreter->evaluate("num(\"3.14159\")");
    EXPECT_DOUBLE_EQ(result.asNumber(), 3.14159);
    
    // Test invalid string to number
    EXPECT_THROW(interpreter->evaluate("num(\"invalid\")"), std::runtime_error);
    
    // Test number to string conversion
    result = interpreter->evaluate("str(42)");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "42");
    
    result = interpreter->evaluate("str(3.14159)");
    EXPECT_TRUE(result.isString());
    // Note: exact string representation may vary
    
    // Test boolean conversion
    result = interpreter->evaluate("str(true)");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "true");
    
    result = interpreter->evaluate("str(false)");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "false");
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
