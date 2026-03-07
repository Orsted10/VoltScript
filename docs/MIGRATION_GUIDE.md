# ClawScript Migration Guide

## Migrating to ClawScript v3.0.0

This guide helps you migrate your code from previous versions of ClawScript to v3.0.0.

## Table of Contents

1. [Overview of Changes](#overview-of-changes)
2. [Breaking Changes](#breaking-changes)
3. [New Features](#new-features)
4. [Deprecated Features](#deprecated-features)
5. [Step-by-Step Migration](#step-by-step-migration)
6. [Common Migration Issues](#common-migration-issues)
7. [Code Examples](#code-examples)
8. [Testing Your Migration](#testing-your-migration)

---

## Overview of Changes

ClawScript v3.0.0 introduces significant improvements while maintaining backward compatibility where possible. Key changes include:

- **Professional Documentation**: Complete overhaul of all documentation
- **Enhanced API**: Improved function naming and consistency
- **Better Error Handling**: More descriptive error messages
- **Performance Improvements**: Optimized core runtime
- **New Standard Library Functions**: Expanded built-in functionality
- **Improved Type System**: Better type checking and conversion

---

## Breaking Changes

### 1. File Extension Changes

**Old**: `.volt` files  
**New**: `.claw` files

```claw
// Old filename: program.volt
// New filename: program.claw
```

### 2. Removed Emojis from CLI Output

**Old**: `⚡ ClawScript v2.0.0`  
**New**: `ClawScript v3.0.0`

This affects REPL output and help messages.

### 3. Function Name Changes

Some functions have been renamed for consistency:

| Old Name | New Name | Status |
|----------|----------|---------|
| `len()` | `length` property | Changed |
| `size()` | `size` property | Changed |
| `substring()` | `substring()` | Unchanged |
| `indexOf()` | `indexOf()` | Unchanged |

### 4. Array Method Changes

Array methods are now called on the array instance:

```claw
// Old way (if applicable)
let length = len(array);

// New way
let length = array.length;
```

---

## New Features

### 1. Enhanced Class System

```claw
// New inheritance support
class Animal {
    init(name) {
        this.name = name;
    }
    
    speak() {
        return this.name + " makes a sound";
    }
}

class Dog extends Animal {
    speak() {
        return this.name + " barks!";
    }
}
```

### 2. Improved String Methods

```claw
let text = "Hello, World!";

// New methods
print text.startsWith("Hello");   // true
print text.endsWith("World!");    // true
print text.replace("World", "ClawScript"); // "Hello, ClawScript!"
```

### 3. Better Object Methods

```claw
let obj = {"key1": "value1", "key2": "value2"};

// New utility functions
let keys = keys(obj);        // ["key1", "key2"]
let values = values(obj);    // ["value1", "value2"]
let hasKey = has(obj, "key1"); // true
```

### 4. Enhanced Error Messages

Error messages are now more descriptive:

```claw
// Old: Generic error
// New: "Division by zero prevented"
// New: "Array index out of bounds (0-2)"
// New: "Both operands must be numbers"
```

---

## Deprecated Features

### 1. Old Array Functions

The following functions are deprecated in favor of property access:

- `len(array)` → use `array.length`
- `size(object)` → use `object.size`

### 2. Legacy Comment Styles

While still supported, the following comment style is discouraged:

```claw
// Still works but not recommended:
/******/ Legacy comment style /******/

// Recommended:
// Standard single-line comment
/* Standard multi-line comment */
```

---

## Step-by-Step Migration

### Step 1: Backup Your Code

Before starting the migration, create a backup of your existing code:

```bash
cp -r /path/to/your/project /path/to/your/project.backup
```

### Step 2: Update File Extensions

Rename all `.volt` files to `.claw`:

```bash
# On Unix systems
find . -name "*.volt" -exec rename 's/\.volt$/.claw/' {} \;

# Manually on any system
mv program.volt program.claw
mv script.volt script.claw
```

### Step 3: Update Function Calls

Replace deprecated function calls:

```claw
// Find and replace in your code:
// len(array) → array.length
// size(object) → object.size
```

### Step 4: Update Import Statements

If you have any import statements, update them:

```claw
// Old (if applicable)
import "module.volt"

// New
import "module.claw"
```

### Step 5: Test Basic Functionality

Create a simple test to ensure basic functionality works:

```claw
// test_migration.claw
print "Testing migration...";

// Test basic operations
let arr = [1, 2, 3];
print "Array length: " + arr.length;

let obj = {"key": "value"};
print "Object size: " + obj.size;

print "Migration test completed successfully!";
```

### Step 6: Run Your Tests

Execute your existing test suite:

```bash
./clawscript test_migration.claw
./clawscript --run your_existing_tests.claw
```

---

## Common Migration Issues

### Issue 1: Array Length Access

**Problem**: Code using `len(array)` fails

**Solution**: Replace with property access:

```claw
// Old
let length = len(myArray);

// New
let length = myArray.length;
```

### Issue 2: Object Size Access

**Problem**: Code using `size(object)` fails

**Solution**: Replace with property access:

```claw
// Old
let size = size(myObject);

// New
let size = myObject.size;
```

### Issue 3: File Extension References

**Problem**: Code references old `.volt` files

**Solution**: Update all file references:

```claw
// Old
let content = readFile("config.volt");

// New
let content = readFile("config.claw");
```

### Issue 4: CLI Script Changes

**Problem**: Build scripts or automation using old CLI output

**Solution**: Update scripts to handle new output format:

```bash
# Old grep pattern
clawscript --version | grep "⚡"

# New grep pattern
clawscript --version | grep "ClawScript"
```

---

## Code Examples

### Before and After Migration

#### Example 1: Basic Array Operations

```claw
// Before (v2.0.0)
let numbers = [1, 2, 3, 4, 5];
let arrayLength = len(numbers);
print "Array has " + arrayLength + " elements";

for (let i = 0; i < len(numbers); i = i + 1) {
    print numbers[i];
}

// After (v3.0.0)
let numbers = [1, 2, 3, 4, 5];
let arrayLength = numbers.length;
print "Array has " + arrayLength + " elements";

for (let i = 0; i < numbers.length; i = i + 1) {
    print numbers[i];
}
```

#### Example 2: Object Operations

```claw
// Before (v2.0.0)
let person = {"name": "Alice", "age": 25};
let objectSize = size(person);
print "Object has " + objectSize + " properties";

// After (v3.0.0)
let person = {"name": "Alice", "age": 25};
let objectSize = person.size;
print "Object has " + objectSize + " properties";

// Additional new functionality
let keys = keys(person);
let values = values(person);
print "Keys: " + join(keys, ", ");
print "Values: " + join(values, ", ");
```

#### Example 3: String Operations

```claw
// Before (v2.0.0)
let text = "Hello, World!";
let sub = substring(text, 0, 5);
print "Substring: " + sub;

// After (v3.0.0) - same functionality plus new methods
let text = "Hello, World!";
let sub = substring(text, 0, 5);
print "Substring: " + sub;

// New string methods
print "Starts with 'Hello': " + text.startsWith("Hello");
print "Ends with 'World!': " + text.endsWith("World!");
print "Replaced: " + text.replace("World", "ClawScript");
```

#### Example 4: Class Usage

```claw
// Before (v2.0.0) - basic classes
class Person {
    init(name, age) {
        this.name = name;
        this.age = age;
    }
    
    introduce() {
        return "Hi, I'm " + this.name;
    }
}

// After (v3.0.0) - enhanced with inheritance
class Person {
    init(name, age) {
        this.name = name;
        this.age = age;
    }
    
    introduce() {
        return "Hi, I'm " + this.name + " and I'm " + this.age;
    }
}

class Student extends Person {
    init(name, age, grade) {
        super.init(name, age);
        this.grade = grade;
    }
    
    introduce() {
        return super.introduce() + ". I'm in grade " + this.grade;
    }
}

let student = Student("Alice", 15, 10);
print student.introduce();
```

---

## Testing Your Migration

### Migration Test Suite

Create a comprehensive test to verify your migration:

```claw
// migration_test.claw
print "Running migration tests...";

let testResults = [];
let totalTests = 0;
let passedTests = 0;

fn runTest(testName, testFunction) {
    totalTests = totalTests + 1;
    try {
        let result = testFunction();
        if (result) {
            passedTests = passedTests + 1;
            print "✓ " + testName;
        } else {
            print "✗ " + testName + " (test failed)";
        }
    } catch (error) {
        print "✗ " + testName + " (error: " + error + ")";
    }
}

// Test 1: Array operations
runTest("Array length property", fn() {
    let arr = [1, 2, 3, 4, 5];
    return arr.length === 5;
});

// Test 2: Object operations
runTest("Object size property", fn() {
    let obj = {"a": 1, "b": 2, "c": 3};
    return obj.size === 3;
});

// Test 3: String operations
runTest("String new methods", fn() {
    let text = "Hello, World!";
    return text.startsWith("Hello") && 
           text.endsWith("World!") &&
           text.replace("World", "ClawScript") === "Hello, ClawScript!";
});

// Test 4: Object utility functions
runTest("Object utility functions", fn() {
    let obj = {"name": "Alice", "age": 25};
    let keys = keys(obj);
    let values = values(obj);
    return keys.length === 2 && 
           values.length === 2 &&
           has(obj, "name");
});

// Test 5: Class inheritance
runTest("Class inheritance", fn() {
    class Animal {
        init(name) {
            this.name = name;
        }
        speak() {
            return this.name + " makes a sound";
        }
    }
    
    class Dog extends Animal {
        speak() {
            return this.name + " barks!";
        }
    }
    
    let dog = Dog("Buddy");
    return dog.speak() === "Buddy barks!";
});

// Test 6: File operations
runTest("File operations", fn() {
    let testContent = "Hello, Migration Test!";
    writeFile("test_migration.txt", testContent);
    let readContent = readFile("test_migration.txt");
    deleteFile("test_migration.txt");
    return readContent === testContent;
});

// Results
print "\nMigration Test Results:";
print "Tests run: " + totalTests;
print "Tests passed: " + passedTests;
print "Success rate: " + ((passedTests / totalTests) * 100) + "%";

if (passedTests === totalTests) {
    print "✓ All migration tests passed!";
} else {
    print "✗ Some tests failed. Review migration.";
}
```

### Performance Comparison Test

```claw
// performance_test.claw
print "Running performance comparison...";

fn measureTime(description, testFunction) {
    let start = clock();
    testFunction();
    let end = clock();
    let duration = end - start;
    print description + ": " + duration + " ms";
    return duration;
}

// Test array operations
measureTime("Array creation", fn() {
    let arr = [];
    for (let i = 0; i < 10000; i = i + 1) {
        arr.push(i);
    }
});

measureTime("Array iteration", fn() {
    let arr = [1, 2, 3, 4, 5];
    let sum = 0;
    for (let i = 0; i < arr.length; i = i + 1) {
        sum = sum + arr[i];
    }
});

// Test string operations
measureTime("String concatenation", fn() {
    let result = "";
    for (let i = 0; i < 1000; i = i + 1) {
        result = result + "x";
    }
});

print "Performance testing completed.";
```

---

## Automated Migration Script

For large projects, you can use this script to help with migration:

```claw
// migrate.claw - Automated migration helper
print "ClawScript Migration Helper v3.0.0";

fn migrateFile(filename) {
    if (!fileExists(filename)) {
        print "File not found: " + filename;
        return false;
    }
    
    let content = readFile(filename);
    let migrated = content;
    let changes = 0;
    
    // Replace len(array) with array.length
    migrated = replace(migrated, "len(", ".length");
    changes = changes + 1;
    
    // Replace size(object) with object.size  
    migrated = replace(migrated, "size(", ".size");
    changes = changes + 1;
    
    // Update file extensions in content
    migrated = replace(migrated, ".volt", ".claw");
    changes = changes + 1;
    
    if (changes > 0) {
        let newFilename = replace(filename, ".volt", ".claw");
        writeFile(newFilename, migrated);
        print "Migrated: " + filename + " → " + newFilename + " (" + changes + " changes)";
        return true;
    } else {
        print "No changes needed for: " + filename;
        return false;
    }
}

// Usage
let filesToMigrate = ["program.volt", "utils.volt", "tests.volt"];
let migratedCount = 0;

for (let i = 0; i < filesToMigrate.length; i = i + 1) {
    if (migrateFile(filesToMigrate[i])) {
        migratedCount = migratedCount + 1;
    }
}

print "\nMigration completed. Files migrated: " + migratedCount;
```

---

## Getting Help with Migration

If you encounter issues during migration:

1. **Check the Documentation**: Review the API Reference and User Guide
2. **Run Tests**: Use the migration test suite to identify problems
3. **Check Error Messages**: New error messages are more descriptive
4. **Simplify**: Test problematic code in isolation
5. **Community Support**: Use GitHub Issues for migration-specific questions

### Migration Checklist

- [ ] Backup all source code
- [ ] Rename `.volt` files to `.claw`
- [ ] Update `len()` calls to `.length` property
- [ ] Update `size()` calls to `.size` property
- [ ] Update file references in code
- [ ] Update build scripts and automation
- [ ] Run migration test suite
- [ ] Test all existing functionality
- [ ] Update documentation
- [ ] Update deployment scripts

---

This migration guide should help you transition smoothly to ClawScript v3.0.0. The changes are designed to improve the language while minimizing disruption to existing code.
