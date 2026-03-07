# ClawScript Documentation

## ClawScript v3.0.0 Complete Documentation

## Table of Contents

1. [Overview](#overview)
2. [Installation](#installation)
3. [Language Reference](#language-reference)
4. [Standard Library](#standard-library)
5. [API Reference](#api-reference)
6. [User Guide](#user-guide)
7. [Migration Guide](#migration-guide)
8. [Performance Guide](#performance-guide)
9. [Examples](#examples)
10. [Contributing](#contributing)

---

## Overview

ClawScript is a high-performance programming language implemented in C++20, designed for professional software development. It provides a complete execution environment with optimized bytecode virtual machine, comprehensive standard library, and enterprise-grade features.

### Key Features

- **Dynamic Typing**: Flexible type system with runtime type checking
- **Object-Oriented Programming**: Classes, inheritance, and polymorphism
- **Functional Programming**: First-class functions, closures, and higher-order functions
- **Comprehensive Standard Library**: Built-in functions for common operations
- **Performance Optimizations**: JIT compilation and optimized bytecode VM
- **Memory Management**: Automatic garbage collection with manual control options
- **Error Handling**: Robust error reporting and exception handling
- **File I/O**: Comprehensive file system operations
- **Cross-Platform**: Runs on Windows, Linux, and macOS

### Architecture

```
┌─────────────────┐
│   ClawScript    │ Source Code (.claw)
└─────────┬───────┘
          │
┌─────────▼───────┐
│   Lexer         │ Tokenization
└─────────┬───────┘
          │
┌─────────▼───────┐
│   Parser        │ AST Generation
└─────────┬───────┘
          │
┌─────────▼───────┐
│   Compiler      │ Bytecode Generation
└─────────┬───────┘
          │
┌─────────▼───────┐
│   VM            │ Bytecode Execution
└─────────┬───────┘
          │
┌─────────▼───────┐
│   Runtime       │ Memory Management, I/O
└─────────────────┘
```

---

## Installation

### System Requirements

- **Operating System**: Windows 10+, Linux (Ubuntu 18.04+), macOS 10.15+
- **Compiler**: C++20 compatible (GCC 10+, Clang 12+, MSVC 2019+)
- **Memory**: Minimum 512MB RAM, 2GB+ recommended
- **Storage**: 100MB for installation, 500MB+ for development

### Quick Installation

```bash
# Clone repository
git clone https://github.com/your-org/clawscript.git
cd clawscript

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Test installation
./clawscript --version
```

### Installation Options

| Option | Description | Default |
|--------|-------------|---------|
| `DCMAKE_BUILD_TYPE` | Build configuration | Release |
| `DCLAW_ENABLE_JIT` | Enable JIT compilation | OFF |
| `DCLAW_ENABLE_AOT` | Enable AOT compilation | OFF |
| `DCLAW_ENABLE_PROFILER` | Enable profiler | OFF |

---

## Language Reference

### Basic Syntax

```claw
// Variables
let name = "ClawScript";
let version = 3.0;
let isReady = true;
let data = nil;

// Comments
// Single line comment
/* Multi-line comment */

// Functions
fn add(a, b) {
    return a + b;
}

// Classes
class Calculator {
    init() {
        this.result = 0;
    }
    
    add(value) {
        this.result = this.result + value;
        return this;
    }
}
```

### Data Types

| Type | Description | Example |
|------|-------------|---------|
| `number` | Numeric values (integers and floats) | `42`, `3.14` |
| `string` | Text values | `"Hello, World!"` |
| `boolean` | Logical values | `true`, `false` |
| `null` | Absence of value | `nil` |
| `array` | Ordered collections | `[1, 2, 3]` |
| `object` | Key-value pairs | `{"key": "value"}` |
| `function` | Callable objects | `fn(x) { return x * 2; }` |

### Control Flow

```claw
// If statements
if (condition) {
    // Code
} else if (anotherCondition) {
    // Code
} else {
    // Code
}

// Loops
while (condition) {
    // Code
}

for (let i = 0; i < 10; i = i + 1) {
    // Code
}

run {
    // Code
} until (condition);
```

### Operators

| Category | Operators |
|----------|-----------|
| Arithmetic | `+`, `-`, `*`, `/`, `%` |
| Comparison | `==`, `!=`, `<`, `>`, `<=`, `>=` |
| Logical | `&&`, `||`, `!` |
| Assignment | `=`, `+=`, `-=` |
| Ternary | `condition ? value1 : value2` |

---

## Standard Library

### Core Functions

```claw
// Type conversion
print type("hello");     // "string"
print str(42);           // "42"
print num("3.14");       // 3.14

// Utility functions
print clock();           // Current time in milliseconds
sleep(1000);            // Sleep for 1 second
print random();         // Random number 0-1
```

### Array Methods

```claw
let arr = [1, 2, 3, 4, 5];

// Properties
print arr.length;        // 5

// Methods
arr.push(6);            // Add element
let last = arr.pop();    // Remove last element
arr.reverse();           // Reverse array

// Functional methods
let doubled = arr.map(fn(x) { return x * 2; });
let evens = arr.filter(fn(x) { return x % 2 === 0; });
let sum = arr.reduce(fn(acc, x) { return acc + x; }, 0);
```

### String Methods

```claw
let text = "Hello, World!";

// Properties
print text.length;       // 13

// Methods
print substring(text, 0, 5);     // "Hello"
print indexOf(text, "World");     // 7
print toUpper(text);              // "HELLO, WORLD!"
print toLower(text);              // "hello, world!"
print trim("  spaced  ");         // "spaced"
```

### Object Methods

```claw
let obj = {"name": "Alice", "age": 25};

// Properties
print obj.size;          // 2

// Utility functions
print keys(obj);         // ["name", "age"]
print values(obj);       // ["Alice", 25]
print has(obj, "name");  // true
remove(obj, "age");      // Remove property
```

### Mathematical Functions

```claw
// Basic
print abs(-5);           // 5
print sqrt(16);          // 4
print pow(2, 3);         // 8
print min(5, 3);         // 3
print max(5, 3);         // 5

// Trigonometric
print sin(0);            // 0
print cos(0);            // 1
print tan(0);            // 0

// Rounding
print round(3.7);        // 4
print floor(3.7);        // 3
print ceil(3.2);         // 4
```

### File I/O

```claw
// Basic operations
let content = readFile("data.txt");
writeFile("output.txt", "Hello, World!");

// File information
print fileExists("data.txt");     // true/false
print fileSize("data.txt");        // Size in bytes

// File management
deleteFile("temp.txt");            // Delete file
```

---

## API Reference

### Global Functions

#### `print(value)`
Outputs a value to the console.

```claw
print "Hello, World!";
print 42;
print [1, 2, 3];
```

#### `input(prompt)` → string
Reads user input from the console.

```claw
let name = input("Enter your name: ");
print "Hello, " + name;
```

#### `exit(code)` → void
Exits the program with the specified exit code.

```claw
exit(0);  // Success
exit(1);  // Error
```

### Type Functions

#### `type(value)` → string
Returns the type of the value.

```claw
print type("hello");  // "string"
print type(42);       // "number"
print type(true);     // "boolean"
```

#### `str(value)` → string
Converts a value to a string.

```claw
print str(42);        // "42"
print str(true);      // "true"
```

#### `num(string)` → number
Converts a string to a number.

```claw
print num("42");      // 42
print num("3.14");    // 3.14
```

---

## User Guide

### Getting Started

1. **Installation**: Follow the installation instructions above
2. **First Program**: Create a file `hello.claw` with `print "Hello, World!";`
3. **Run**: Execute with `./clawscript hello.claw`
4. **Interactive Mode**: Use `./clawscript` for REPL

### Basic Programming

```claw
// Variables and types
let message = "Welcome to ClawScript!";
let count = 10;
let ready = true;

// Control flow
if (ready) {
    print message;
    for (let i = 0; i < count; i = i + 1) {
        print "Item " + (i + 1);
    }
}

// Functions
fn greet(name) {
    return "Hello, " + name + "!";
}

print greet("ClawScript");
```

### Object-Oriented Programming

```claw
// Class definition
class Person {
    init(name, age) {
        this.name = name;
        this.age = age;
    }
    
    introduce() {
        return "Hi, I'm " + this.name + " and I'm " + this.age;
    }
    
    haveBirthday() {
        this.age = this.age + 1;
        print "Happy birthday, " + this.name + "!";
    }
}

// Usage
let alice = Person("Alice", 25);
print alice.introduce();
alice.haveBirthday();
```

### File Operations

```claw
// Reading files
fn processFile(filename) {
    if (!fileExists(filename)) {
        print "File not found: " + filename;
        return;
    }
    
    let content = readFile(filename);
    let lines = split(content, "\n");
    
    print "Processing " + lines.length + " lines...";
    for (let i = 0; i < lines.length; i = i + 1) {
        if (lines[i].length > 0) {
            print "Line " + (i + 1) + ": " + lines[i];
        }
    }
}

processFile("data.txt");
```

---

## Migration Guide

### Migrating from v2.0.0 to v3.0.0

#### Breaking Changes

1. **File Extensions**: `.volt` → `.claw`
2. **Array Length**: `len(array)` → `array.length`
3. **Object Size**: `size(object)` → `object.size`
4. **CLI Output**: Removed emojis from command-line interface

#### Migration Steps

1. **Backup**: Create backup of existing code
2. **Rename Files**: Change `.volt` to `.claw`
3. **Update Function Calls**: Replace deprecated functions
4. **Test**: Run existing test suite
5. **Update Documentation**: Update any references

#### Code Examples

```claw
// Before v3.0.0
let arr = [1, 2, 3];
let length = len(arr);

let obj = {"a": 1, "b": 2};
let size = size(obj);

// After v3.0.0
let arr = [1, 2, 3];
let length = arr.length;

let obj = {"a": 1, "b": 2};
let size = obj.size;
```

---

## Performance Guide

### Optimization Tips

1. **Use Appropriate Data Structures**
   - Arrays for ordered data
   - Objects for key-value lookups
   - Use `keys()` and `values()` for iteration

2. **Minimize Object Creation**
   - Reuse objects when possible
   - Use object pools for frequently created objects
   - Avoid creating objects in tight loops

3. **Optimize Loops**
   - Cache frequently accessed properties
   - Use reverse loops when possible
   - Avoid repeated function calls

4. **String Operations**
   - Use `join()` for large concatenations
   - Cache string lengths
   - Use appropriate string methods

### Performance Measurement

```claw
fn measureTime(description, func) {
    let start = clock();
    func();
    let end = clock();
    print description + ": " + (end - start) + " ms";
}

measureTime("Array creation", fn() {
    let arr = [];
    for (let i = 0; i < 10000; i = i + 1) {
        arr.push(i);
    }
});
```

### JIT Compilation

Enable JIT for performance-critical applications:

```bash
./clawscript --jit=aggressive program.claw
```

JIT works best with:
- Stable types
- Predictable control flow
- Hot loops
- Simple arithmetic operations

---

## Examples

### Complete Application Example

```claw
// task_manager.claw - Task management system

class Task {
    init(title, priority) {
        this.title = title;
        this.priority = priority;
        this.completed = false;
        this.createdAt = clock();
    }
    
    complete() {
        this.completed = true;
        this.completedAt = clock();
    }
    
    getInfo() {
        let status = this.completed ? "Completed" : "Pending";
        return "[" + status + "] " + this.title + " (Priority: " + this.priority + ")";
    }
}

class TaskManager {
    init() {
        this.tasks = [];
        this.nextId = 1;
    }
    
    addTask(title, priority) {
        let task = Task(title, priority);
        task.id = this.nextId;
        this.tasks.push(task);
        this.nextId = this.nextId + 1;
        print "Task added: " + task.getInfo();
        return task;
    }
    
    completeTask(taskId) {
        for (let i = 0; i < this.tasks.length; i = i + 1) {
            if (this.tasks[i].id === taskId) {
                this.tasks[i].complete();
                print "Task completed: " + this.tasks[i].getInfo();
                return true;
            }
        }
        print "Task not found: " + taskId;
        return false;
    }
    
    listTasks() {
        print "\n=== Task List ===";
        if (this.tasks.length === 0) {
            print "No tasks found.";
            return;
        }
        
        for (let i = 0; i < this.tasks.length; i = i + 1) {
            print (i + 1) + ". " + this.tasks[i].getInfo();
        }
        print "================";
    }
    
    getStatistics() {
        let total = this.tasks.length;
        let completed = 0;
        let pending = 0;
        
        for (let i = 0; i < this.tasks.length; i = i + 1) {
            if (this.tasks[i].completed) {
                completed = completed + 1;
            } else {
                pending = pending + 1;
            }
        }
        
        print "\n=== Statistics ===";
        print "Total tasks: " + total;
        print "Completed: " + completed;
        print "Pending: " + pending;
        print "Completion rate: " + ((completed / total) * 100) + "%";
        print "================";
    }
    
    saveToFile(filename) {
        let data = {
            "tasks": this.tasks,
            "nextId": this.nextId,
            "savedAt": clock()
        };
        
        let json = str(data);  // Simplified JSON serialization
        writeFile(filename, json);
        print "Tasks saved to " + filename;
    }
    
    loadFromFile(filename) {
        if (!fileExists(filename)) {
            print "No saved file found: " + filename;
            return false;
        }
        
        let json = readFile(filename);
        // Simplified JSON parsing would go here
        print "Tasks loaded from " + filename;
        return true;
    }
}

// Main application
fn main() {
    print "=== ClawScript Task Manager ===";
    
    let manager = TaskManager();
    
    // Add some tasks
    manager.addTask("Write documentation", "High");
    manager.addTask("Fix bugs", "Medium");
    manager.addTask("Review code", "Low");
    
    // List tasks
    manager.listTasks();
    
    // Complete a task
    manager.completeTask(1);
    
    // List updated tasks
    manager.listTasks();
    
    // Show statistics
    manager.getStatistics();
    
    // Save tasks
    manager.saveToFile("tasks.claw");
    
    print "\nTask Manager demo completed!";
}

// Run the application
main();
```

### Data Processing Example

```claw
// data_processor.claw - Data analysis and processing

class DataProcessor {
    init() {
        this.data = [];
        this.statistics = {};
    }
    
    loadData(filename) {
        if (!fileExists(filename)) {
            print "Data file not found: " + filename;
            return false;
        }
        
        let content = readFile(filename);
        let lines = split(content, "\n");
        
        print "Loading " + lines.length + " data points...";
        
        for (let i = 0; i < lines.length; i = i + 1) {
            if (lines[i].length > 0) {
                let fields = split(lines[i], ",");
                if (fields.length >= 2) {
                    this.data.push({
                        "value": num(fields[0]),
                        "category": fields[1],
                        "timestamp": fields.length > 2 ? fields[2] : str(clock())
                    });
                }
            }
        }
        
        print "Loaded " + this.data.length + " valid data points";
        return true;
    }
    
    calculateStatistics() {
        if (this.data.length === 0) {
            print "No data to analyze";
            return;
        }
        
        let values = this.data.map(fn(item) { return item["value"]; });
        
        this.statistics = {
            "count": values.length,
            "sum": values.reduce(fn(acc, x) { return acc + x; }, 0),
            "min": values.reduce(fn(acc, x) { return min(acc, x); }, values[0]),
            "max": values.reduce(fn(acc, x) { return max(acc, x); }, values[0]),
            "mean": 0,
            "categories": {}
        };
        
        this.statistics["mean"] = this.statistics["sum"] / this.statistics["count"];
        
        // Category analysis
        for (let i = 0; i < this.data.length; i = i + 1) {
            let category = this.data[i]["category"];
            if (!has(this.statistics["categories"], category)) {
                this.statistics["categories"][category] = 0;
            }
            this.statistics["categories"][category] = this.statistics["categories"][category] + 1;
        }
        
        this.printStatistics();
    }
    
    printStatistics() {
        print "\n=== Data Statistics ===";
        print "Count: " + this.statistics["count"];
        print "Sum: " + this.statistics["sum"];
        print "Min: " + this.statistics["min"];
        print "Max: " + this.statistics["max"];
        print "Mean: " + this.statistics["mean"];
        
        print "\nCategories:";
        let categories = this.statistics["categories"];
        let categoryKeys = keys(categories);
        for (let i = 0; i < categoryKeys.length; i = i + 1) {
            let key = categoryKeys[i];
            print "  " + key + ": " + categories[key];
        }
        print "======================";
    }
    
    filterByCategory(category) {
        return this.data.filter(fn(item) { return item["category"] === category; });
    }
    
    filterByValue(minValue, maxValue) {
        return this.data.filter(fn(item) { 
            return item["value"] >= minValue && item["value"] <= maxValue; 
        });
    }
    
    exportResults(filename) {
        let output = "Value,Category,Timestamp\n";
        
        for (let i = 0; i < this.data.length; i = i + 1) {
            let item = this.data[i];
            output = output + item["value"] + "," + item["category"] + "," + item["timestamp"] + "\n";
        }
        
        writeFile(filename, output);
        print "Results exported to " + filename;
    }
}

// Sample data generation
fn generateSampleData(filename, count) {
    let categories = ["A", "B", "C", "D"];
    let data = "";
    
    for (let i = 0; i < count; i = i + 1) {
        let value = random() * 100;
        let category = categories[floor(random() * categories.length)];
        let timestamp = str(clock());
        
        data = data + value + "," + category + "," + timestamp + "\n";
    }
    
    writeFile(filename, data);
    print "Generated " + count + " sample data points in " + filename;
}

// Main execution
fn main() {
    print "=== ClawScript Data Processor ===";
    
    let filename = "sample_data.csv";
    
    // Generate sample data
    generateSampleData(filename, 1000);
    
    // Process data
    let processor = DataProcessor();
    processor.loadData(filename);
    processor.calculateStatistics();
    
    // Filter examples
    print "\nFiltering examples:";
    let categoryA = processor.filterByCategory("A");
    print "Category A items: " + categoryA.length;
    
    let range50to75 = processor.filterByValue(50, 75);
    print "Values between 50-75: " + range50to75.length;
    
    // Export results
    processor.exportResults("processed_data.csv");
    
    print "\nData processing completed!";
}

main();
```

---

## Contributing

### Development Setup

1. **Clone Repository**: `git clone https://github.com/your-org/clawscript.git`
2. **Install Dependencies**: CMake, C++20 compiler
3. **Build**: `mkdir build && cd build && cmake .. && make`
4. **Test**: `make test`

### Code Style

- Use 4 spaces for indentation
- Use camelCase for variables and functions
- Use PascalCase for classes
- Add comments for complex logic
- Follow existing code patterns

### Submitting Changes

1. Fork the repository
2. Create feature branch
3. Make changes with tests
4. Ensure all tests pass
5. Submit pull request

### Testing

```bash
# Run all tests
./clawscript_test

# Run specific test
./clawscript_test --test=array_operations

# Performance tests
./clawscript_test --performance
```

---

This documentation provides a comprehensive guide to ClawScript v3.0.0. For specific API details, see the individual reference documents. For community support and contributions, visit the GitHub repository.
