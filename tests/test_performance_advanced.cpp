#include <gtest/gtest.h>
#include "interpreter/interpreter.h"
#include "compiler/compiler.h"
#include "vm/vm.h"
#include "interpreter/value.h"
#include <chrono>
#include <thread>

using namespace claw;

class AdvancedPerformanceTest : public ::testing::Test {
protected:
    void SetUp() override {
        interpreter = std::make_unique<Interpreter>();
    }
    
    std::unique_ptr<Interpreter> interpreter;
    
    template<typename Func>
    double measureTime(Func&& func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        return duration.count() / 1000.0;
    }
};

// Test large-scale data processing
TEST_F(AdvancedPerformanceTest, LargeScaleDataProcessing) {
    double time = measureTime([this]() {
        interpreter->evaluate(R"(
let dataset = [];
for (let i = 0; i < 100000; i = i + 1) {
    dataset.push({
        "id": i,
        "value": i * 2.5,
        "category": i % 10,
        "active": i % 2 === 0
    });
}

let filtered = [];
for (let i = 0; i < dataset.length; i = i + 1) {
    if (dataset[i].active && dataset[i].value > 100) {
        filtered.push(dataset[i]);
    }
}

let sum = 0;
for (let i = 0; i < filtered.length; i = i + 1) {
    sum = sum + filtered[i].value;
}
)");
    });
    
    EXPECT_LT(time, 2000.0); // Less than 2 seconds
    
    Value result = interpreter->evaluate("sum");
    EXPECT_TRUE(result.isNumber());
    EXPECT_GT(result.asNumber(), 0);
}

// Test complex algorithm performance
TEST_F(AdvancedPerformanceTest, ComplexAlgorithmPerformance) {
    interpreter->evaluate(R"(
fn quickSort(arr, left, right) {
    if (left >= right) return;
    
    let pivot = arr[(left + right) / 2];
    let i = left;
    let j = right;
    
    while (i <= j) {
        while (arr[i] < pivot) i = i + 1;
        while (arr[j] > pivot) j = j - 1;
        
        if (i <= j) {
            let temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
            i = i + 1;
            j = j - 1;
        }
    }
    
    quickSort(arr, left, j);
    quickSort(arr, i, right);
}
)");
    
    double time = measureTime([this]() {
        interpreter->evaluate(R"(
let largeArray = [];
for (let i = 0; i < 10000; i = i + 1) {
    largeArray.push(10000 - i); // Reverse order for worst case
}

quickSort(largeArray, 0, largeArray.length - 1);
)");
    });
    
    EXPECT_LT(time, 1500.0); // Less than 1.5 seconds
    
    // Verify sorting worked
    Value result = interpreter->evaluate("largeArray[0]");
    EXPECT_EQ(result.asNumber(), 0);
    
    result = interpreter->evaluate("largeArray[9999]");
    EXPECT_EQ(result.asNumber(), 10000);
}

// Test string processing performance
TEST_F(AdvancedPerformanceTest, StringProcessingPerformance) {
    double time = measureTime([this]() {
        interpreter->evaluate(R"(
let text = "Lorem ipsum dolor sit amet, consectetur adipiscing elit. ";
let words = [];
let word = "";

for (let i = 0; i < text.length; i = i + 1) {
    let char = substring(text, i, 1);
    if (char === " ") {
        if (word.length > 0) {
            words.push(word);
            word = "";
        }
    } else {
        word = word + char;
    }
}

if (word.length > 0) {
    words.push(word);
}

let wordCount = words.length;
)");
    });
    
    EXPECT_LT(time, 100.0); // Less than 0.1 seconds
    
    Value result = interpreter->evaluate("wordCount");
    EXPECT_EQ(result.asNumber(), 8);
}

// Test mathematical computation performance
TEST_F(AdvancedPerformanceTest, MathematicalComputationPerformance) {
    interpreter->evaluate(R"(
fn matrixMultiply(a, b) {
    let rowsA = a.length;
    let colsA = a[0].length;
    let colsB = b[0].length;
    
    let result = [];
    for (let i = 0; i < rowsA; i = i + 1) {
        result.push([]);
        for (let j = 0; j < colsB; j = j + 1) {
            result[i].push(0);
            for (let k = 0; k < colsA; k = k + 1) {
                result[i][j] = result[i][j] + a[i][k] * b[k][j];
            }
        }
    }
    return result;
}
)");
    
    double time = measureTime([this]() {
        interpreter->evaluate(R"(
let matrixA = [];
let matrixB = [];

// Create 50x50 matrices
for (let i = 0; i < 50; i = i + 1) {
    matrixA.push([]);
    matrixB.push([]);
    for (let j = 0; j < 50; j = j + 1) {
        matrixA[i].push(i + j);
        matrixB[i].push(i * j + 1);
    }
}

let result = matrixMultiply(matrixA, matrixB);
)");
    });
    
    EXPECT_LT(time, 3000.0); // Less than 3 seconds
    
    Value result = interpreter->evaluate("result[0][0]");
    EXPECT_TRUE(result.isNumber());
}

// Test recursive algorithm optimization
TEST_F(AdvancedPerformanceTest, RecursiveAlgorithmOptimization) {
    interpreter->evaluate(R"(
fn memoizedFibonacci(n, memo) {
    if (memo[n] !== nil) return memo[n];
    if (n <= 1) return n;
    
    memo[n] = memoizedFibonacci(n - 1, memo) + memoizedFibonacci(n - 2, memo);
    return memo[n];
}
)");
    
    double time = measureTime([this]() {
        interpreter->evaluate(R"(
let memo = {};
let result = memoizedFibonacci(40, memo);
)");
    });
    
    EXPECT_LT(time, 500.0); // Less than 0.5 seconds with memoization
    
    Value result = interpreter->evaluate("result");
    EXPECT_EQ(result.asNumber(), 102334155);
}

// Test object-oriented performance
TEST_F(AdvancedPerformanceTest, ObjectOrientedPerformance) {
    interpreter->evaluate(R"(
class Particle {
    init(x, y, vx, vy) {
        this.x = x;
        this.y = y;
        this.vx = vx;
        this.vy = vy;
    }
    
    update(dt) {
        this.x = this.x + this.vx * dt;
        this.y = this.y + this.vy * dt;
    }
    
    distance(other) {
        let dx = this.x - other.x;
        let dy = this.y - other.y;
        return sqrt(dx * dx + dy * dy);
    }
}
)");
    
    double time = measureTime([this]() {
        interpreter->evaluate(R"(
let particles = [];
for (let i = 0; i < 1000; i = i + 1) {
    particles.push(Particle(
        random() * 100,
        random() * 100,
        random() * 2 - 1,
        random() * 2 - 1
    ));
}

// Simulate 100 time steps
for (let step = 0; step < 100; step = step + 1) {
    for (let i = 0; i < particles.length; i = i + 1) {
        particles[i].update(0.1);
    }
}

let totalDistance = 0;
for (let i = 0; i < particles.length - 1; i = i + 1) {
    totalDistance = totalDistance + particles[i].distance(particles[i + 1]);
}
)");
    });
    
    EXPECT_LT(time, 2000.0); // Less than 2 seconds
    
    Value result = interpreter->evaluate("totalDistance");
    EXPECT_TRUE(result.isNumber());
    EXPECT_GT(result.asNumber(), 0);
}

// Test memory pressure handling
TEST_F(AdvancedPerformanceTest, MemoryPressureHandling) {
    double time = measureTime([this]() {
        interpreter->evaluate(R"(
let memoryTest = [];
for (let i = 0; i < 50000; i = i + 1) {
    let obj = {
        "id": i,
        "data": new Array(100).fill(i),
        "metadata": {
            "created": i,
            "updated": i * 2,
            "tags": ["tag1", "tag2", "tag3"]
        }
    };
    memoryTest.push(obj);
}

// Process the data
let sum = 0;
for (let i = 0; i < memoryTest.length; i = i + 1) {
    sum = sum + memoryTest[i].id;
}
)");
    });
    
    EXPECT_LT(time, 5000.0); // Less than 5 seconds
    
    Value result = interpreter->evaluate("sum");
    EXPECT_EQ(result.asNumber(), 1249750000); // Sum of 0 to 49999
}

// Test concurrent-like operations
TEST_F(AdvancedPerformanceTest, ConcurrentLikeOperations) {
    double time = measureTime([this]() {
        interpreter->evaluate(R"(
let tasks = [];
let results = [];

// Simulate multiple tasks
for (let taskId = 0; taskId < 8; taskId = taskId + 1) {
    let taskResult = 0;
    let start = taskId * 12500;
    let end = (taskId + 1) * 12500;
    
    for (let i = start; i < end; i = i + 1) {
        taskResult = taskResult + sqrt(i) + abs(i - 62500);
    }
    
    results.push(taskResult);
}

let finalResult = 0;
for (let i = 0; i < results.length; i = i + 1) {
    finalResult = finalResult + results[i];
}
)");
    });
    
    EXPECT_LT(time, 3000.0); // Less than 3 seconds
    
    Value result = interpreter->evaluate("finalResult");
    EXPECT_TRUE(result.isNumber());
    EXPECT_GT(result.asNumber(), 0);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
