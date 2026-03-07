# ClawScript User Guide

## ClawScript v3.0.0 User Guide

## Table of Contents

1. [Installation](#installation)
2. [Your First Program](#your-first-program)
3. [Language Fundamentals](#language-fundamentals)
4. [Working with Data](#working-with-data)
5. [Control Flow](#control-flow)
6. [Functions](#functions)
7. [Object-Oriented Programming](#object-oriented-programming)
8. [File Operations](#file-operations)
9. [Error Handling](#error-handling)
10. [Best Practices](#best-practices)
11. [Common Patterns](#common-patterns)
12. [Troubleshooting](#troubleshooting)

---

## Installation

### System Requirements

- **Operating System**: Windows, Linux, macOS
- **Compiler**: C++20 compatible (GCC 10+, Clang 12+, MSVC 2019+)
- **Build System**: CMake 3.16 or higher
- **Memory**: Minimum 512MB RAM, 2GB+ recommended
- **Storage**: 100MB for installation, 500MB+ for development

### Installing from Source

1. **Clone the Repository**
   ```bash
   git clone https://github.com/your-org/clawscript.git
   cd clawscript
   ```

2. **Build the Project**
   ```bash
   mkdir build && cd build
   cmake .. -DCMAKE_BUILD_TYPE=Release
   make -j$(nproc)  # Linux/macOS
   # On Windows: cmake --build . --config Release
   ```

3. **Verify Installation**
   ```bash
   ./clawscript --version
   ```

### Installation Options

#### Development Build
```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

#### Release Build (Optimized)
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release
```

#### With JIT Support
```bash
cmake .. -DCLAW_ENABLE_JIT=ON
```

---

## Your First Program

### Hello World

Create a file named `hello.claw`:

```claw
// hello.claw
print "Hello, World!";
```

Run it:

```bash
./clawscript hello.claw
```

### Interactive Mode

Start the REPL (Read-Eval-Print Loop):

```bash
./clawscript
```

You'll see:
```
ClawScript v3.0.0 REPL
Type 'exit' to quit, 'history' to show command history
Commands: clear (reset environment), help (show this message)

> print "Hello, ClawScript!"
Hello, ClawScript!
> 
```

### A More Complex Example

Create `calculator.claw`:

```claw
// calculator.claw
class Calculator {
    init() {
        this.result = 0;
    }
    
    add(value) {
        this.result = this.result + value;
        return this;
    }
    
    multiply(value) {
        this.result = this.result * value;
        return this;
    }
    
    divide(value) {
        if (value === 0) {
            print "Error: Cannot divide by zero";
            return this;
        }
        this.result = this.result / value;
        return this;
    }
    
    getResult() {
        return this.result;
    }
    
    reset() {
        this.result = 0;
        return this;
    }
}

// Usage examples
let calc = Calculator();

print "Basic operations:";
calc.add(10).multiply(2).divide(5);
print "Result: " + calc.getResult();  // 4

print "\nChained operations:";
calc.reset().add(5).add(3).multiply(2);
print "Result: " + calc.getResult();  // 16

print "\nError handling:";
calc.reset().add(10).divide(0);  // Shows error message
print "Final result: " + calc.getResult();  // Still 10
```

---

## Language Fundamentals

### Variables and Types

```claw
// Variable declaration with let
let name = "Alice";
let age = 25;
let height = 5.6;
let isStudent = true;
let grades = [90, 85, 92];
let profile = {
    "name": "Alice",
    "age": 25,
    "active": true
};

// Type checking
print type(name);        // "string"
print type(age);         // "number"
print type(isStudent);   // "boolean"
print type(grades);      // "array"
print type(profile);     // "object"
```

### Comments

```claw
// Single line comment

/*
   Multi-line comment
   that spans multiple lines
*/

/* Nested comment example:
   // This is inside a multi-line comment
   /* This can be nested too */
*/
```

### Basic Operations

```claw
// Arithmetic
let a = 10;
let b = 3;
print a + b;    // 13
print a - b;    // 7
print a * b;    // 30
print a / b;    // 3.333...
print a % b;    // 1 (if modulo is supported)

// Comparison
print a > b;     // true
print a < b;     // false
print a >= b;    // true
print a <= b;    // false
print a === b;   // false (exact equality)
print a !== b;   // true

// Logical
let x = true;
let y = false;
print x && y;   // false (logical AND)
print x || y;   // true (logical OR)
print !x;       // false (logical NOT)

// String operations
let first = "Hello";
let last = "World";
print first + " " + last;  // "Hello World"
```

---

## Working with Data

### Arrays

```claw
// Creating arrays
let numbers = [1, 2, 3, 4, 5];
let mixed = [42, "hello", true, nil, [1, 2]];
let empty = [];

// Accessing elements
print numbers[0];     // 1 (first element)
print numbers[4];     // 5 (last element)
print numbers[-1];    // Error: negative index not supported

// Modifying arrays
numbers[2] = 99;      // [1, 2, 99, 4, 5]
numbers.push(6);      // [1, 2, 99, 4, 5, 6]
numbers.reverse();    // [6, 5, 4, 99, 2, 1]

// Array methods
print numbers.length; // 6

let doubled = numbers.map(fn(x) { return x * 2; });
print doubled;        // [12, 10, 8, 198, 4, 2]

let evens = numbers.filter(fn(x) { return x % 2 === 0; });
print evens;          // [6, 4, 2]

let sum = numbers.reduce(fn(acc, x) { return acc + x; }, 0);
print sum;            // 117

let joined = numbers.join(", ");
print joined;         // "6, 5, 4, 99, 2, 1"
```

### Hash Maps (Objects)

```claw
// Creating objects
let person = {
    "name": "Alice Johnson",
    "age": 25,
    "email": "alice@example.com",
    "active": true
};

// Accessing properties
print person["name"];     // "Alice Johnson"
print person["age"];      // 25

// Modifying properties
person["age"] = 26;
person["phone"] = "555-1234";

// Object methods
print person.size;        // 4 (before adding phone)

let keys = keys(person);
print keys;               // ["name", "age", "email", "active"]

let values = values(person);
print values;             // ["Alice Johnson", 25, "alice@example.com", true]

print has(person, "email");  // true
print has(person, "address"); // false

remove(person, "active");
print person;             // No longer has "active" key
```

### Strings

```claw
let text = "Hello, ClawScript!";

// String properties
print text.length;        // 17

// String methods
print substring(text, 0, 5);     // "Hello"
print indexOf(text, "Claw");     // 7
print toUpper(text);             // "HELLO, CLAWSCRIPT!"
print toLower(text);             // "hello, clawscript!"
print trim("  spaced  ");        // "spaced"

// Advanced string operations
let words = split(text, " ");
print words;               // ["Hello,", "ClawScript!"]

let replaced = replace(text, "ClawScript", "World");
print replaced;           // "Hello, World!"

print startsWith(text, "Hello");   // true
print endsWith(text, "!");         // true
```

---

## Control Flow

### Conditional Statements

```claw
let score = 85;

if (score >= 90) {
    print "Grade: A";
} else if (score >= 80) {
    print "Grade: B";
} else if (score >= 70) {
    print "Grade: C";
} else if (score >= 60) {
    print "Grade: D";
} else {
    print "Grade: F";
}

// Ternary operator
let status = (score >= 60) ? "Pass" : "Fail";
print "Status: " + status;
```

### Loops

```claw
// While loop
let count = 0;
while (count < 5) {
    print "Count: " + count;
    count = count + 1;
}

// For loop
for (let i = 0; i < 5; i = i + 1) {
    print "Iteration " + i;
}

// Run-until loop (executes at least once)
let number = 0;
run {
    print "Number: " + number;
    number = number + 1;
} until (number >= 3);

// Loop control
for (let i = 0; i < 10; i = i + 1) {
    if (i === 5) {
        break;  // Exit loop
    }
    if (i % 2 === 0) {
        continue;  // Skip to next iteration
    }
    print "Odd number: " + i;
}
```

### Iterating Over Data Structures

```claw
// Iterate over array
let fruits = ["apple", "banana", "orange"];
for (let i = 0; i < fruits.length; i = i + 1) {
    print "Fruit " + i + ": " + fruits[i];
}

// Iterate over object keys
let person = {"name": "Alice", "age": 25, "city": "New York"};
let personKeys = keys(person);
for (let i = 0; i < personKeys.length; i = i + 1) {
    let key = personKeys[i];
    let value = person[key];
    print key + ": " + value;
}
```

---

## Functions

### Basic Functions

```claw
// Function declaration
fn greet(name) {
    return "Hello, " + name + "!";
}

print greet("Alice");  // "Hello, Alice!"

// Function with multiple parameters
fn add(a, b) {
    return a + b;
}

print add(5, 3);  // 8

// Function with default-like behavior
fn power(base, exponent) {
    if (exponent === nil) {
        exponent = 2;  // Default exponent
    }
    return pow(base, exponent);
}

print power(3);      // 9 (3^2)
print power(3, 3);   // 27 (3^3)
```

### Recursive Functions

```claw
// Factorial
fn factorial(n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

print factorial(5);  // 120

// Fibonacci
fn fibonacci(n) {
    if (n <= 1) {
        return n;
    }
    return fibonacci(n - 1) + fibonacci(n - 2);
}

print fibonacci(10);  // 55
```

### Higher-Order Functions

```claw
// Function that takes another function
fn applyTwice(func, value) {
    return func(func(value));
}

fn double(x) {
    return x * 2;
}

print applyTwice(double, 5);  // 20 (double(double(5)))

// Function that returns another function
fn createMultiplier(factor) {
    return fn(x) {
        return x * factor;
    };
}

let triple = createMultiplier(3);
print triple(10);  // 30

let quadruple = createMultiplier(4);
print quadruple(10);  // 40
```

### Closures

```claw
// Counter with closure
fn makeCounter() {
    let count = 0;
    return fn() {
        count = count + 1;
        return count;
    };
}

let counter1 = makeCounter();
let counter2 = makeCounter();

print counter1();  // 1
print counter1();  // 2
print counter2();  // 1 (independent counter)
print counter1();  // 3

// Closure with parameters
fn makeAdder(base) {
    return fn(x) {
        return base + x;
    };
}

let add5 = makeAdder(5);
let add10 = makeAdder(10);

print add5(3);   // 8
print add10(3);  // 13
```

---

## Object-Oriented Programming

### Classes and Objects

```claw
// Basic class definition
class Person {
    init(name, age) {
        this.name = name;
        this.age = age;
    }
    
    introduce() {
        return "Hi, I'm " + this.name + " and I'm " + this.age + " years old.";
    }
    
    haveBirthday() {
        this.age = this.age + 1;
        print "Happy birthday, " + this.name + "! You are now " + this.age;
    }
}

// Creating objects
let alice = Person("Alice", 25);
let bob = Person("Bob", 30);

print alice.introduce();  // "Hi, I'm Alice and I'm 25 years old."
print bob.introduce();    // "Hi, I'm Bob and I'm 30 years old."

alice.haveBirthday();     // "Happy birthday, Alice! You are now 26"
print alice.age;          // 26
```

### Inheritance

```claw
// Parent class
class Animal {
    init(name) {
        this.name = name;
    }
    
    speak() {
        return this.name + " makes a sound";
    }
    
    eat() {
        return this.name + " is eating";
    }
}

// Child class
class Dog extends Animal {
    init(name, breed) {
        super.init(name);  // Call parent constructor
        this.breed = breed;
    }
    
    speak() {
        return this.name + " barks!";
    }
    
    wagTail() {
        return this.name + " wags tail happily";
    }
}

// Another child class
class Cat extends Animal {
    speak() {
        return this.name + " meows";
    }
    
    purr() {
        return this.name + " purrs contentedly";
    }
}

let dog = Dog("Buddy", "Golden Retriever");
let cat = Cat("Whiskers");

print dog.speak();        // "Buddy barks!"
print dog.eat();          // "Buddy is eating"
print dog.wagTail();      // "Buddy wags tail happily"

print cat.speak();        // "Whiskers meows"
print cat.eat();          // "Whiskers is eating"
print cat.purr();         // "Whiskers purrs contentedly"
```

### Practical Example: Bank Account

```claw
class BankAccount {
    init(owner, balance) {
        this.owner = owner;
        this.balance = balance;
        this.transactions = [];
    }
    
    deposit(amount) {
        if (amount <= 0) {
            print "Error: Deposit amount must be positive";
            return false;
        }
        this.balance = this.balance + amount;
        this.transactions.push({
            "type": "deposit",
            "amount": amount,
            "balance": this.balance
        });
        print "Deposited: $" + amount + ". New balance: $" + this.balance;
        return true;
    }
    
    withdraw(amount) {
        if (amount <= 0) {
            print "Error: Withdrawal amount must be positive";
            return false;
        }
        if (amount > this.balance) {
            print "Error: Insufficient funds";
            return false;
        }
        this.balance = this.balance - amount;
        this.transactions.push({
            "type": "withdrawal",
            "amount": amount,
            "balance": this.balance
        });
        print "Withdrew: $" + amount + ". New balance: $" + this.balance;
        return true;
    }
    
    getBalance() {
        return this.balance;
    }
    
    getStatement() {
        print "Account Statement for " + this.owner;
        print "Current Balance: $" + this.balance;
        print "Recent Transactions:";
        for (let i = this.transactions.length - 1; i >= 0 && i >= this.transactions.length - 5; i = i - 1) {
            let transaction = this.transactions[i];
            print "  " + transaction["type"] + ": $" + transaction["amount"] + " (Balance: $" + transaction["balance"] + ")";
        }
    }
}

// Usage
let account = BankAccount("John Doe", 1000);
account.deposit(500);
account.withdraw(200);
account.withdraw(2000);  // Error: Insufficient funds
account.deposit(-100);   // Error: Deposit amount must be positive
account.getStatement();
```

---

## File Operations

### Reading and Writing Files

```claw
// Writing to a file
writeFile("notes.txt", "Hello, this is my first file!");
writeFile("data.csv", "Name,Age,City\nAlice,25,New York\nBob,30,London");

// Reading from a file
let content = readFile("notes.txt");
print content;

// Processing file line by line
let csvContent = readFile("data.csv");
let lines = split(csvContent, "\n");
print "CSV Data:";

for (let i = 0; i < lines.length; i = i + 1) {
    if (lines[i].length > 0) {  // Skip empty lines
        let fields = split(lines[i], ",");
        print "Line " + (i + 1) + ": " + fields[0] + ", " + fields[1] + ", " + fields[2];
    }
}
```

### File Management

```claw
// Check if file exists
if (fileExists("config.txt")) {
    print "Configuration file found";
    let config = readFile("config.txt");
    // Process configuration
} else {
    print "Configuration file not found, creating default";
    writeFile("config.txt", "setting1=value1\nsetting2=value2");
}

// Get file information
if (fileExists("data.txt")) {
    let size = fileSize("data.txt");
    print "File size: " + size + " bytes";
}

// Safe file operations
fn safeReadFile(filename) {
    if (!fileExists(filename)) {
        print "Error: File '" + filename + "' does not exist";
        return nil;
    }
    
    let size = fileSize(filename);
    if (size > 1000000) {  // 1MB limit
        print "Error: File too large (" + size + " bytes)";
        return nil;
    }
    
    return readFile(filename);
}

let data = safeReadFile("largefile.txt");
if (data !== nil) {
    print "File read successfully";
}
```

### Log File Example

```claw
class Logger {
    init(filename) {
        this.filename = filename;
        this.initLogFile();
    }
    
    initLogFile() {
        let timestamp = str(clock());
        let header = "Log started at: " + timestamp + "\n";
        writeFile(this.filename, header);
    }
    
    log(message, level) {
        if (level === nil) {
            level = "INFO";
        }
        let timestamp = str(clock());
        let entry = "[" + timestamp + "] [" + level + "] " + message + "\n";
        
        // Read existing content
        let existing = "";
        if (fileExists(this.filename)) {
            existing = readFile(this.filename);
        }
        
        // Append new entry
        writeFile(this.filename, existing + entry);
    }
    
    error(message) {
        this.log(message, "ERROR");
    }
    
    warning(message) {
        this.log(message, "WARNING");
    }
    
    info(message) {
        this.log(message, "INFO");
    }
}

// Usage
let logger = Logger("application.log");
logger.info("Application started");
logger.warning("Low memory detected");
logger.error("Failed to connect to database");
```

---

## Error Handling

### Common Error Types

```claw
// Division by zero
fn safeDivide(a, b) {
    if (b === 0) {
        print "Error: Cannot divide by zero";
        return nil;
    }
    return a / b;
}

print safeDivide(10, 2);  // 5
print safeDivide(10, 0);  // Error message and nil

// Array bounds checking
fn safeArrayGet(arr, index) {
    if (index < 0 || index >= arr.length) {
        print "Error: Array index out of bounds (0-" + (arr.length - 1) + ")";
        return nil;
    }
    return arr[index];
}

let arr = [1, 2, 3];
print safeArrayGet(arr, 1);  // 2
print safeArrayGet(arr, 5);  // Error message and nil

// Type checking
fn safeAdd(a, b) {
    if (type(a) !== "number" || type(b) !== "number") {
        print "Error: Both operands must be numbers";
        return nil;
    }
    return a + b;
}

print safeAdd(5, 3);       // 8
print safeAdd(5, "hello"); // Error message and nil
```

### Validation Functions

```claw
// Input validation
fn validateEmail(email) {
    if (type(email) !== "string") {
        return false;
    }
    
    // Basic email validation
    let hasAt = indexOf(email, "@") > 0;
    let hasDot = indexOf(email, ".") > indexOf(email, "@");
    let lengthValid = email.length > 5;
    
    return hasAt && hasDot && lengthValid;
}

fn validateAge(age) {
    if (type(age) !== "number") {
        return false;
    }
    return age >= 0 && age <= 150;
}

// Usage
let email = "user@example.com";
if (validateEmail(email)) {
    print "Valid email address";
} else {
    print "Invalid email address";
}

let age = 25;
if (validateAge(age)) {
    print "Valid age";
} else {
    print "Invalid age";
}
```

### Error Recovery

```claw
// Robust file processing
fn processFileRobust(filename) {
    // Check file exists
    if (!fileExists(filename)) {
        print "Warning: File not found, creating empty file";
        writeFile(filename, "");
        return 0;  // Return 0 lines processed
    }
    
    // Try to read file
    let content = readFile(filename);
    if (content === nil) {
        print "Error: Could not read file";
        return -1;  // Error code
    }
    
    // Process content
    let lines = split(content, "\n");
    let processed = 0;
    
    for (let i = 0; i < lines.length; i = i + 1) {
        if (lines[i].length > 0) {
            // Process line (would have more complex logic here)
            processed = processed + 1;
        }
    }
    
    print "Successfully processed " + processed + " lines";
    return processed;
}

let result = processFileRobust("data.txt");
if (result >= 0) {
    print "File processing completed successfully";
} else {
    print "File processing failed";
}
```

---

## Best Practices

### Code Organization

```claw
// Use meaningful variable names
let userAge = 25;           // Good
let a = 25;                // Bad

// Use consistent naming conventions
let userName = "Alice";     // camelCase for variables
let user_name = "Alice";    // snake_case (also acceptable)
let calculateTotal = fn() { ... }; // camelCase for functions

// Group related code
class UserManager {
    init() {
        this.users = [];
        this.nextId = 1;
    }
    
    addUser(name, email) {
        let user = {
            "id": this.nextId,
            "name": name,
            "email": email,
            "active": true
        };
        this.users.push(user);
        this.nextId = this.nextId + 1;
        return user;
    }
    
    getUser(id) {
        for (let i = 0; i < this.users.length; i = i + 1) {
            if (this.users[i]["id"] === id) {
                return this.users[i];
            }
        }
        return nil;
    }
    
    deactivateUser(id) {
        let user = this.getUser(id);
        if (user !== nil) {
            user["active"] = false;
            return true;
        }
        return false;
    }
}
```

### Performance Tips

```claw
// Avoid repeated calculations in loops
// Bad:
for (let i = 0; i < expensiveCalculation(); i = i + 1) {
    // expensiveCalculation() called every iteration
}

// Good:
let limit = expensiveCalculation();
for (let i = 0; i < limit; i = i + 1) {
    // Calculation done once
}

// Use built-in methods when possible
// Bad:
let sum = 0;
for (let i = 0; i < numbers.length; i = i + 1) {
    sum = sum + numbers[i];
}

// Good:
let sum = numbers.reduce(fn(acc, x) { return acc + x; }, 0);

// Pre-allocate arrays when size is known
// Bad:
let arr = [];
for (let i = 0; i < 1000; i = i + 1) {
    arr.push(i);  // May cause multiple reallocations
}

// Good (if supported):
let arr = new Array(1000);  // Pre-allocate
for (let i = 0; i < 1000; i = i + 1) {
    arr[i] = i;
}
```

### Security Considerations

```claw
// Input validation
fn processUserInput(input) {
    // Validate input type
    if (type(input) !== "string") {
        print "Invalid input type";
        return;
    }
    
    // Validate input length
    if (input.length > 1000) {
        print "Input too long";
        return;
    }
    
    // Sanitize input (remove potentially dangerous characters)
    let sanitized = replace(input, "<", "&lt;");
    sanitized = replace(sanitized, ">", "&gt;");
    
    // Process sanitized input
    print "Processed: " + sanitized;
}

// File operation safety
fn safeFileWrite(filename, content) {
    // Validate filename
    if (type(filename) !== "string" || filename.length === 0) {
        print "Invalid filename";
        return false;
    }
    
    // Prevent path traversal attacks
    if (indexOf(filename, "..") >= 0 || indexOf(filename, "/") === 0) {
        print "Invalid file path";
        return false;
    }
    
    // Validate content size
    if (type(content) === "string" && content.length > 1000000) {
        print "Content too large";
        return false;
    }
    
    writeFile(filename, content);
    return true;
}
```

---

## Common Patterns

### Factory Pattern

```claw
class Shape {
    draw() {
        print "Drawing a generic shape";
    }
}

class Circle extends Shape {
    init(radius) {
        this.radius = radius;
    }
    
    draw() {
        print "Drawing a circle with radius " + this.radius;
    }
}

class Rectangle extends Shape {
    init(width, height) {
        this.width = width;
        this.height = height;
    }
    
    draw() {
        print "Drawing a rectangle " + this.width + "x" + this.height;
    }
}

// Factory
fn createShape(type, params) {
    if (type === "circle") {
        return Circle(params[0]);
    } else if (type === "rectangle") {
        return Rectangle(params[0], params[1]);
    } else {
        return Shape();
    }
}

// Usage
let circle = createShape("circle", [5]);
let rect = createShape("rectangle", [10, 20]);

circle.draw();  // "Drawing a circle with radius 5"
rect.draw();    // "Drawing a rectangle 10x20"
```

### Observer Pattern

```claw
class EventEmitter {
    init() {
        this.listeners = {};
    }
    
    on(event, callback) {
        if (this.listeners[event] === nil) {
            this.listeners[event] = [];
        }
        this.listeners[event].push(callback);
    }
    
    emit(event, data) {
        if (this.listeners[event] !== nil) {
            for (let i = 0; i < this.listeners[event].length; i = i + 1) {
                this.listeners[event][i](data);
            }
        }
    }
}

class Button extends EventEmitter {
    init(label) {
        super.init();
        this.label = label;
    }
    
    click() {
        print "Button '" + this.label + "' clicked";
        this.emit("click", {"button": this.label});
    }
}

// Usage
let button = Button("Submit");

button.on("click", fn(data) {
    print "Handler 1: " + data["button"] + " was clicked";
});

button.on("click", fn(data) {
    print "Handler 2: Processing " + data["button"] + " click";
});

button.click();
// Output:
// Button 'Submit' clicked
// Handler 1: Submit was clicked
// Handler 2: Processing Submit click
```

### Singleton Pattern

```claw
class Database {
    init() {
        if (Database.instance !== nil) {
            return Database.instance;
        }
        
        this.connections = [];
        this.maxConnections = 10;
        Database.instance = this;
    }
    
    connect() {
        if (this.connections.length >= this.maxConnections) {
            print "Maximum connections reached";
            return false;
        }
        
        this.connections.push({"id": this.connections.length + 1});
        print "New connection established. Total: " + this.connections.length;
        return true;
    }
    
    disconnect(id) {
        for (let i = 0; i < this.connections.length; i = i + 1) {
            if (this.connections[i]["id"] === id) {
                this.connections.splice(i, 1);
                print "Connection " + id + " closed. Total: " + this.connections.length;
                return true;
            }
        }
        return false;
    }
    
    static getInstance() {
        if (Database.instance === nil) {
            Database.instance = Database();
        }
        return Database.instance;
    }
}

// Usage
let db1 = Database.getInstance();
let db2 = Database.getInstance();

print db1 === db2;  // true (same instance)

db1.connect();     // New connection established. Total: 1
db2.connect();     // New connection established. Total: 2
```

---

## Troubleshooting

### Common Issues

#### 1. "Division by zero" Error
```claw
// Problem:
let result = 10 / 0;

// Solution:
fn safeDivide(a, b) {
    if (b === 0) {
        print "Cannot divide by zero";
        return nil;
    }
    return a / b;
}
```

#### 2. "Array index out of bounds" Error
```claw
// Problem:
let arr = [1, 2, 3];
print arr[5];  // Error

// Solution:
fn safeGet(arr, index) {
    if (index >= 0 && index < arr.length) {
        return arr[index];
    }
    print "Index " + index + " out of bounds";
    return nil;
}
```

#### 3. Type Mismatch Errors
```claw
// Problem:
print "hello" + 42;  // Error

// Solution:
fn safeConcat(a, b) {
    if (type(a) !== "string") {
        a = str(a);
    }
    if (type(b) !== "string") {
        b = str(b);
    }
    return a + b;
}

print safeConcat("Value: ", 42);  // "Value: 42"
```

### Debugging Tips

```claw
// Use print statements for debugging
fn debugPrint(variableName, value) {
    print "DEBUG [" + variableName + "]: " + str(value) + " (type: " + type(value) + ")";
}

// Example usage
let result = someComplexFunction();
debugPrint("result", result);

// Check array contents
let arr = [1, 2, 3, 4, 5];
debugPrint("array", arr);
debugPrint("array length", arr.length);

// Check object properties
let obj = {"name": "Alice", "age": 25};
debugPrint("object keys", keys(obj));
debugPrint("object values", values(obj));
```

### Performance Issues

```claw
// Measure execution time
fn measureTime(func) {
    let start = clock();
    let result = func();
    let end = clock();
    print "Execution time: " + (end - start) + " ms";
    return result;
}

// Usage
let result = measureTime(fn() {
    // Some expensive operation
    let sum = 0;
    for (let i = 0; i < 100000; i = i + 1) {
        sum = sum + i;
    }
    return sum;
});
```

### Getting Help

1. **Check the documentation**: Review the API reference and this user guide
2. **Use the REPL**: Test small code snippets interactively
3. **Add debug output**: Use print statements to understand what's happening
4. **Simplify the problem**: Break down complex code into smaller, testable parts
5. **Check types**: Ensure variables contain the expected data types

---

This user guide provides a comprehensive introduction to ClawScript programming. For detailed API documentation, see the [API Reference](API_REFERENCE.md). For advanced topics and optimization techniques, refer to the main documentation.
