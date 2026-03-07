#include <gtest/gtest.h>
#include "interpreter/interpreter.h"
#include "interpreter/gc_alloc.h"
#include "features/array.h"
#include "features/class.h"
#include "interpreter/value.h"

using namespace claw;

class MemoryManagementTest : public ::testing::Test {
protected:
    void SetUp() override {
        interpreter = std::make_unique<Interpreter>();
        gc = std::make_unique<GCAllocator>();
    }
    
    void TearDown() override {
        interpreter.reset();
        gc.reset();
    }
    
    std::unique_ptr<Interpreter> interpreter;
    std::unique_ptr<GCAllocator> gc;
};

// Test garbage collection with arrays
TEST_F(MemoryManagementTest, ArrayGarbageCollection) {
    // Create multiple arrays
    interpreter->evaluate("let arr1 = [1, 2, 3, 4, 5]");
    interpreter->evaluate("let arr2 = [\"hello\", \"world\", \"test\"]");
    interpreter->evaluate("let arr3 = [[1, 2], [3, 4], [5, 6]]");
    
    // Force garbage collection
    gc->collect();
    
    // Arrays should still be accessible
    Value result = interpreter->evaluate("arr1.length");
    EXPECT_EQ(result.asNumber(), 5);
    
    result = interpreter->evaluate("arr2[0]");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "hello");
    
    result = interpreter->evaluate("arr3[1][0]");
    EXPECT_EQ(result.asNumber(), 3);
}

// Test garbage collection with objects
TEST_F(MemoryManagementTest, ObjectGarbageCollection) {
    interpreter->evaluate(R"(
class TestClass {
    init(value) {
        this.value = value;
        this.data = [1, 2, 3];
    }
    
    getValue() {
        return this.value;
    }
}
)");
    
    interpreter->evaluate("let obj1 = TestClass(42)");
    interpreter->evaluate("let obj2 = TestClass(\"hello\")");
    
    // Force garbage collection
    gc->collect();
    
    // Objects should still be accessible
    Value result = interpreter->evaluate("obj1.getValue()");
    EXPECT_EQ(result.asNumber(), 42);
    
    result = interpreter->evaluate("obj2.getValue()");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "hello");
    
    result = interpreter->evaluate("obj1.data.length");
    EXPECT_EQ(result.asNumber(), 3);
}

// Test memory leak prevention with closures
TEST_F(MemoryManagementTest, ClosureMemoryLeakPrevention) {
    interpreter->evaluate(R"(
fn createClosure() {
    let largeArray = [];
    for (let i = 0; i < 1000; i = i + 1) {
        largeArray.push(i);
    }
    
    fn inner() {
        return largeArray.length;
    }
    
    return inner;
}
)");
    
    // Create closure
    interpreter->evaluate("let closure = createClosure()");
    
    // Original largeArray should be kept alive by closure
    gc->collect();
    
    Value result = interpreter->evaluate("closure()");
    EXPECT_EQ(result.asNumber(), 1000);
}

// Test circular reference handling
TEST_F(MemoryManagementTest, CircularReferenceHandling) {
    interpreter->evaluate(R"(
class Node {
    init() {
        this.next = nil;
    }
    
    setNext(node) {
        this.next = node;
    }
}
)");
    
    interpreter->evaluate("let node1 = Node()");
    interpreter->evaluate("let node2 = Node()");
    interpreter->evaluate("node1.setNext(node2)");
    interpreter->evaluate("node2.setNext(node1)"); // Create circular reference
    
    // Force garbage collection
    gc->collect();
    
    // Should not crash and nodes should still be accessible
    Value result = interpreter->evaluate("node1.next.next === node1");
    EXPECT_TRUE(result.isBool());
    EXPECT_TRUE(result.asBool());
}

// Test memory usage with large data structures
TEST_F(MemoryManagementTest, LargeDataStructureMemory) {
    // Create large nested structure
    interpreter->evaluate(R"(
let largeData = {
    "arrays": [],
    "objects": {},
    "strings": ""
};

// Create large arrays
for (let i = 0; i < 100; i = i + 1) {
    let subArray = [];
    for (let j = 0; j < 100; j = j + 1) {
        subArray.push(i * 100 + j);
    }
    largeData.arrays.push(subArray);
}

// Create large objects
for (let i = 0; i < 100; i = i + 1) {
    let key = "prop" + i;
    largeData.objects[key] = "value" + i;
}

// Create large string
for (let i = 0; i < 1000; i = i + 1) {
    largeData.strings = largeData.strings + "x";
}
)");
    
    // Force garbage collection
    gc->collect();
    
    // Verify data integrity
    Value result = interpreter->evaluate("largeData.arrays[50][50]");
    EXPECT_EQ(result.asNumber(), 5050);
    
    result = interpreter->evaluate("largeData.objects[\"prop42\"]");
    EXPECT_TRUE(result.isString());
    EXPECT_EQ(result.asString(), "value42");
    
    result = interpreter->evaluate("largeData.strings.length");
    EXPECT_EQ(result.asNumber(), 1000);
}

// Test memory pool efficiency
TEST_F(MemoryManagementTest, MemoryPoolEfficiency) {
    // Create and destroy many small objects
    for (int i = 0; i < 10000; ++i) {
        interpreter->evaluate("let temp = [1, 2, 3]");
        interpreter->evaluate("let tempObj = {\"value\": " + std::to_string(i) + "}");
    }
    
    // Force garbage collection
    gc->collect();
    
    // Should still work without memory issues
    Value result = interpreter->evaluate("42 + 8");
    EXPECT_EQ(result.asNumber(), 50);
}

// Test string interning memory efficiency
TEST_F(MemoryManagementTest, StringInterningEfficiency) {
    // Create many identical strings to test interning
    for (int i = 0; i < 1000; ++i) {
        interpreter->evaluate("let str" + std::to_string(i) + " = \"test_string\"");
    }
    
    // Force garbage collection
    gc->collect();
    
    // All strings should be identical (interned)
    interpreter->evaluate("let result = (str0 === str999)");
    Value result = interpreter->evaluate("result");
    EXPECT_TRUE(result.isBool());
    EXPECT_TRUE(result.asBool());
}

// Test memory pressure handling
TEST_F(MemoryManagementTest, MemoryPressureHandling) {
    // Create memory pressure
    interpreter->evaluate(R"(
let pressureArray = [];
for (let i = 0; i < 50000; i = i + 1) {
    pressureArray.push({
        "id": i,
        "data": "large_string_data_" + i,
        "nested": {"level": 1, "value": i * 2}
    });
}
)");
    
    // Should handle memory pressure gracefully
    gc->collect();
    
    // Verify some data still exists
    Value result = interpreter->evaluate("pressureArray[25000].id");
    EXPECT_EQ(result.asNumber(), 25000);
    
    result = interpreter->evaluate("pressureArray[25000].nested.value");
    EXPECT_EQ(result.asNumber(), 50000);
}

// Test allocator statistics
TEST_F(MemoryManagementTest, AllocatorStatistics) {
    // Get initial statistics
    size_t initialAllocated = gc->getAllocatedBytes();
    size_t initialObjects = gc->getObjectCount();
    
    // Allocate some objects
    interpreter->evaluate("let testArray = [1, 2, 3, 4, 5]");
    interpreter->evaluate("let testObject = {\"key\": \"value\"}");
    
    size_t afterAlloc = gc->getAllocatedBytes();
    size_t afterObjects = gc->getObjectCount();
    
    EXPECT_GT(afterAlloc, initialAllocated);
    EXPECT_GT(afterObjects, initialObjects);
    
    // Force garbage collection
    gc->collect();
    
    size_t afterGC = gc->getAllocatedBytes();
    size_t afterGCObjects = gc->getObjectCount();
    
    // Should free some memory (but not all due to live references)
    EXPECT_LE(afterGC, afterAlloc);
}

// Test memory leak detection
TEST_F(MemoryManagementTest, MemoryLeakDetection) {
    size_t initialObjects = gc->getObjectCount();
    
    {
        // Create temporary scope
        interpreter->evaluate("let tempArray = [1, 2, 3, 4, 5]");
        interpreter->evaluate("let tempObject = {\"temp\": true}");
        interpreter->evaluate("let tempFunction = fn() { return 42; }");
    }
    
    // Clear references (simulate scope exit)
    interpreter->evaluate("tempArray = nil");
    interpreter->evaluate("tempObject = nil");
    interpreter->evaluate("tempFunction = nil");
    
    // Force garbage collection
    gc->collect();
    
    size_t finalObjects = gc->getObjectCount();
    
    // Should be close to initial count (allowing for some overhead)
    EXPECT_LT(finalObjects, initialObjects + 10);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
