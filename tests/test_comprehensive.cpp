#include <gtest/gtest.h>
#include "interpreter/interpreter.h"
#include "compiler/compiler.h"
#include "vm/vm.h"
#include "features/array.h"
#include "features/class.h"
#include "interpreter/value.h"
#include "version.h"

using namespace claw;

class ComprehensiveTest : public ::testing::Test {
protected:
    void SetUp() override {
        interpreter = std::make_unique<Interpreter>();
    }
    
    std::unique_ptr<Interpreter> interpreter;
};

// Test basic arithmetic operations
TEST_F(ComprehensiveTest, BasicArithmetic) {
    Value result = interpreter->evaluate("2 + 3 * 4");
    EXPECT_TRUE(result.isNumber());
    EXPECT_EQ(result.asNumber(), 14);
    
    result = interpreter->evaluate("(2 + 3) * 4");
    EXPECT_EQ(result.asNumber(), 20);
    
    result = interpreter->evaluate("10 / 3");
    EXPECT_DOUBLE_EQ(result.asNumber(), 10.0 / 3.0);
}

// Test variable operations
TEST_F(ComprehensiveTest, VariableOperations) {
    interpreter->evaluate("let x = 42");
    Value result = interpreter->evaluate("x");
    EXPECT_EQ(result.asNumber(), 42);
    
    interpreter->evaluate("x = x + 8");
    result = interpreter->evaluate("x");
    EXPECT_EQ(result.asNumber(), 50);
}

// Test string operations
TEST_F(ComprehensiveTest, StringOperations) {
    Value result = interpreter->evaluate("\"Hello\" + \" \" + \"World\"");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "Hello World");
    
    result = interpreter->evaluate("len(\"test\")");
    EXPECT_EQ(result.asNumber(), 4);
}

// Test array operations
TEST_F(ComprehensiveTest, ArrayOperations) {
    interpreter->evaluate("let arr = [1, 2, 3, 4, 5]");
    
    Value result = interpreter->evaluate("arr.length");
    EXPECT_EQ(result.asNumber(), 5);
    
    result = interpreter->evaluate("arr[0]");
    EXPECT_EQ(result.asNumber(), 1);
    
    interpreter->evaluate("arr.push(6)");
    result = interpreter->evaluate("arr.length");
    EXPECT_EQ(result.asNumber(), 6);
}

// Test hash map operations
TEST_F(ComprehensiveTest, HashMapOperations) {
    interpreter->evaluate("let map = {\"key\": \"value\", \"num\": 42}");
    
    Value result = interpreter->evaluate("map[\"key\"]");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "value");
    
    result = interpreter->evaluate("map[\"num\"]");
    EXPECT_EQ(result.asNumber(), 42);
    
    interpreter->evaluate("map[\"new\"] = true");
    result = interpreter->evaluate("map[\"new\"]");
    EXPECT_TRUE(result.isBool());
    EXPECT_TRUE(result.asBool());
}

// Test function definitions and calls
TEST_F(ComprehensiveTest, FunctionOperations) {
    interpreter->evaluate("fn add(a, b) { return a + b; }");
    
    Value result = interpreter->evaluate("add(3, 4)");
    EXPECT_EQ(result.asNumber(), 7);
    
    // Test recursive function
    interpreter->evaluate("fn factorial(n) { if (n <= 1) return 1; return n * factorial(n - 1); }");
    result = interpreter->evaluate("factorial(5)");
    EXPECT_EQ(result.asNumber(), 120);
}

// Test class operations
TEST_F(ComprehensiveTest, ClassOperations) {
    interpreter->evaluate(R"(
class Point {
    init(x, y) {
        this.x = x;
        this.y = y;
    }
    
    distance() {
        return sqrt(this.x * this.x + this.y * this.y);
    }
}
)");
    
    interpreter->evaluate("let p = Point(3, 4)");
    Value result = interpreter->evaluate("p.distance()");
    EXPECT_EQ(result.asNumber(), 5.0);
    
    result = interpreter->evaluate("p.x");
    EXPECT_EQ(result.asNumber(), 3);
}

// Test control flow
TEST_F(ComprehensiveTest, ControlFlow) {
    // If-else
    Value result = interpreter->evaluate("if (true) { 42 } else { 24 }");
    EXPECT_EQ(result.asNumber(), 42);
    
    result = interpreter->evaluate("if (false) { 42 } else { 24 }");
    EXPECT_EQ(result.asNumber(), 24);
    
    // While loop
    interpreter->evaluate(R"(
let sum = 0;
let i = 0;
while (i < 5) {
    sum = sum + i;
    i = i + 1;
}
)");
    Value result2 = interpreter->evaluate("sum");
    EXPECT_EQ(result2.asNumber(), 10); // 0 + 1 + 2 + 3 + 4
    
    // For loop
    interpreter->evaluate(R"(
let product = 1;
for (let i = 1; i <= 4; i = i + 1) {
    product = product * i;
}
)");
    result2 = interpreter->evaluate("product");
    EXPECT_EQ(result2.asNumber(), 24); // 1 * 2 * 3 * 4
}

// Test error handling
TEST_F(ComprehensiveTest, ErrorHandling) {
    // Test division by zero
    EXPECT_THROW(interpreter->evaluate("1 / 0"), std::runtime_error);
    
    // Test undefined variable
    EXPECT_THROW(interpreter->evaluate("undefined_var"), std::runtime_error);
    
    // Test type errors
    EXPECT_THROW(interpreter->evaluate("\"string\" + 42"), std::runtime_error);
}

// Test built-in functions
TEST_F(ComprehensiveTest, BuiltInFunctions) {
    // Math functions
    Value result = interpreter->evaluate("abs(-5)");
    EXPECT_EQ(result.asNumber(), 5);
    
    result = interpreter->evaluate("sqrt(16)");
    EXPECT_EQ(result.asNumber(), 4);
    
    // Type functions
    result = interpreter->evaluate("type(42)");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "number");
    
    result = interpreter->evaluate("type(\"hello\")");
    EXPECT_EQ(result.asString(), "string");
}

// Test closure and scoping
TEST_F(ComprehensiveTest, ClosureScoping) {
    interpreter->evaluate(R"(
fn makeCounter() {
    let count = 0;
    fn increment() {
        count = count + 1;
        return count;
    }
    return increment;
}
)");
    
    Value result = interpreter->evaluate("let counter = makeCounter()");
    EXPECT_TRUE(result.isCallable());
    
    result = interpreter->evaluate("counter()");
    EXPECT_EQ(result.asNumber(), 1);
    
    result = interpreter->evaluate("counter()");
    EXPECT_EQ(result.asNumber(), 2);
}

// Test performance with large data
TEST_F(ComprehensiveTest, PerformanceTest) {
    // Test large array operations
    interpreter->evaluate(R"(
let largeArray = [];
for (let i = 0; i < 10000; i = i + 1) {
    largeArray.push(i);
}
)");
    
    Value result = interpreter->evaluate("largeArray.length");
    EXPECT_EQ(result.asNumber(), 10000);
    
    // Test array sum
    result = interpreter->evaluate(R"(
let sum = 0;
for (let i = 0; i < largeArray.length; i = i + 1) {
    sum = sum + largeArray[i];
}
sum
)");
    // Sum of 0 to 9999 = 9999 * 10000 / 2 = 49,995,000
    EXPECT_EQ(result.asNumber(), 49995000);
}

// Test version information
TEST_F(ComprehensiveTest, VersionInfo) {
    EXPECT_STRNE(CLAW_VERSION, "");
    EXPECT_GE(CLAW_VERSION_MAJOR, 3);
    EXPECT_GE(CLAW_VERSION_MINOR, 0);
    EXPECT_GE(CLAW_VERSION_PATCH, 0);
}

// Test memory management
TEST_F(ComprehensiveTest, MemoryManagement) {
    // Create and destroy many objects to test garbage collection
    for (int i = 0; i < 1000; ++i) {
        interpreter->evaluate("let temp = [1, 2, 3, 4, 5]");
        interpreter->evaluate("let obj = {\"value\": " + std::to_string(i) + "}");
    }
    
    // Should not crash and should still work
    Value result = interpreter->evaluate("5 + 3");
    EXPECT_EQ(result.asNumber(), 8);
}

// Main test runner
int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
