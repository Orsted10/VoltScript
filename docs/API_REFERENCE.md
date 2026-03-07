# ClawScript API Reference

## ClawScript v3.0.0 API Documentation

## Table of Contents

1. [Core Language API](#core-language-api)
2. [Array Methods](#array-methods)
3. [Hash Map Methods](#hash-map-methods)
4. [String Methods](#string-methods)
5. [Mathematical Functions](#mathematical-functions)
6. [File I/O Functions](#file-io-functions)
7. [Type and Conversion Functions](#type-and-conversion-functions)
8. [Utility Functions](#utility-functions)
9. [Class System API](#class-system-api)
10. [Error Handling](#error-handling)

---

## Core Language API

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
exit(0);  // Successful exit
exit(1);  // Error exit
```

---

## Array Methods

### Creation and Access

#### `[item1, item2, ...]` → array
Creates a new array.

```claw
let numbers = [1, 2, 3, 4, 5];
let mixed = [42, "hello", true, nil];
```

### Properties

#### `array.length` → number
Returns the number of elements in the array.

```claw
let arr = [1, 2, 3];
print arr.length;  // 3
```

### Methods

#### `array.push(item)` → number
Adds an item to the end of the array and returns the new length.

```claw
let arr = [1, 2, 3];
arr.push(4);  // arr is now [1, 2, 3, 4]
```

#### `array.pop()` → value
Removes and returns the last element of the array.

```claw
let arr = [1, 2, 3];
let last = arr.pop();  // last is 3, arr is [1, 2]
```

#### `array.reverse()` → array
Reverses the array in place and returns it.

```claw
let arr = [1, 2, 3];
arr.reverse();  // arr is [3, 2, 1]
```

#### `array.map(function)` → array
Creates a new array by applying a function to each element.

```claw
let numbers = [1, 2, 3, 4, 5];
let doubled = numbers.map(fn(x) { return x * 2; });
// doubled is [2, 4, 6, 8, 10]
```

#### `array.filter(function)` → array
Creates a new array with elements that pass the test function.

```claw
let numbers = [1, 2, 3, 4, 5, 6];
let evens = numbers.filter(fn(x) { return x % 2 === 0; });
// evens is [2, 4, 6]
```

#### `array.reduce(function, initialValue)` → value
Reduces the array to a single value using a function.

```claw
let numbers = [1, 2, 3, 4, 5];
let sum = numbers.reduce(fn(acc, x) { return acc + x; }, 0);
// sum is 15
```

#### `array.join(separator)` → string
Joins all array elements into a string.

```claw
let words = ["Hello", "World", "ClawScript"];
let sentence = words.join(" ");
// sentence is "Hello World ClawScript"
```

---

## Hash Map Methods

### Creation and Access

#### `{key1: value1, key2: value2, ...}` → object
Creates a new hash map.

```claw
let person = {
    "name": "Alice",
    "age": 25,
    "active": true
};
```

### Properties

#### `object.size` → number
Returns the number of key-value pairs.

```claw
let obj = {"a": 1, "b": 2};
print obj.size;  // 2
```

### Methods

#### `keys(object)` → array
Returns an array of all keys.

```claw
let obj = {"name": "Alice", "age": 25};
let allKeys = keys(obj);  // ["name", "age"]
```

#### `values(object)` → array
Returns an array of all values.

```claw
let obj = {"name": "Alice", "age": 25};
let allValues = values(obj);  // ["Alice", 25]
```

#### `has(object, key)` → boolean
Checks if the object has the specified key.

```claw
let obj = {"name": "Alice"};
let hasName = has(obj, "name");     // true
let hasAge = has(obj, "age");       // false
```

#### `remove(object, key)` → void
Removes a key-value pair from the object.

```claw
let obj = {"name": "Alice", "age": 25};
remove(obj, "age");  // obj is now {"name": "Alice"}
```

---

## String Methods

### Properties

#### `string.length` → number
Returns the length of the string.

```claw
let text = "Hello";
print text.length;  // 5
```

### Methods

#### `substring(string, start, end)` → string
Extracts a substring from start to end (exclusive).

```claw
let text = "Hello, World!";
let sub = substring(text, 0, 5);  // "Hello"
```

#### `indexOf(string, searchValue)` → number
Returns the index of the first occurrence, or -1 if not found.

```claw
let text = "Hello, World!";
let index = indexOf(text, "World");  // 7
```

#### `toUpper(string)` → string
Converts the string to uppercase.

```claw
let text = "hello";
let upper = toUpper(text);  // "HELLO"
```

#### `toLower(string)` → string
Converts the string to lowercase.

```claw
let text = "HELLO";
let lower = toLower(text);  // "hello"
```

#### `trim(string)` → string
Removes whitespace from both ends.

```claw
let text = "  hello  ";
let trimmed = trim(text);  // "hello"
```

#### `split(string, separator)` → array
Splits the string into an array of substrings.

```claw
let text = "hello world test";
let words = split(text, " ");  // ["hello", "world", "test"]
```

#### `replace(string, searchValue, replaceValue)` → string
Replaces all occurrences of searchValue with replaceValue.

```claw
let text = "hello world";
let replaced = replace(text, "world", "ClawScript");  // "hello ClawScript"
```

#### `startsWith(string, prefix)` → boolean
Checks if the string starts with the specified prefix.

```claw
let text = "hello world";
let starts = startsWith(text, "hello");  // true
```

#### `endsWith(string, suffix)` → boolean
Checks if the string ends with the specified suffix.

```claw
let text = "hello world";
let ends = endsWith(text, "world");  // true
```

---

## Mathematical Functions

### Basic Arithmetic

#### `abs(number)` → number
Returns the absolute value.

```claw
print abs(-5);    // 5
print abs(3.14);  // 3.14
```

#### `sqrt(number)` → number
Returns the square root.

```claw
print sqrt(16);   // 4
print sqrt(2);    // 1.4142135623730951
```

#### `pow(base, exponent)` → number
Returns base raised to the power of exponent.

```claw
print pow(2, 3);  // 8
print pow(10, 2); // 100
```

#### `min(a, b)` → number
Returns the smaller of two numbers.

```claw
print min(5, 3);  // 3
print min(-1, 2); // -1
```

#### `max(a, b)` → number
Returns the larger of two numbers.

```claw
print max(5, 3);  // 5
print max(-1, 2); // 2
```

### Trigonometry

#### `sin(number)` → number
Returns the sine of an angle in radians.

```claw
print sin(0);     // 0
print sin(3.141592653589793); // ~0
```

#### `cos(number)` → number
Returns the cosine of an angle in radians.

```claw
print cos(0);     // 1
print cos(3.141592653589793); // -1
```

#### `tan(number)` → number
Returns the tangent of an angle in radians.

```claw
print tan(0);     // 0
```

### Rounding

#### `round(number)` → number
Rounds to the nearest integer.

```claw
print round(3.7);  // 4
print round(3.2);  // 3
```

#### `floor(number)` → number
Rounds down to the nearest integer.

```claw
print floor(3.7);  // 3
print floor(3.2);  // 3
```

#### `ceil(number)` → number
Rounds up to the nearest integer.

```claw
print ceil(3.7);   // 4
print ceil(3.2);   // 4
```

### Random

#### `random()` → number
Returns a random number between 0 (inclusive) and 1 (exclusive).

```claw
let r = random();  // Random number like 0.123456789
```

---

## File I/O Functions

#### `readFile(filename)` → string
Reads the entire contents of a file.

```claw
let content = readFile("data.txt");
print content;
```

#### `writeFile(filename, content)` → void
Writes content to a file (overwrites if exists).

```claw
writeFile("output.txt", "Hello, World!");
```

#### `fileExists(filename)` → boolean
Checks if a file exists.

```claw
if (fileExists("data.txt")) {
    print "File exists";
}
```

#### `fileSize(filename)` → number
Returns the file size in bytes.

```claw
let size = fileSize("data.txt");
print "File size: " + size + " bytes";
```

#### `deleteFile(filename)` → void
Deletes a file.

```claw
deleteFile("temp.txt");
```

---

## Type and Conversion Functions

#### `type(value)` → string
Returns the type of the value as a string.

```claw
print type("hello");  // "string"
print type(42);       // "number"
print type(true);     // "boolean"
print type(nil);      // "null"
print type([1, 2]);   // "array"
print type({});       // "object"
```

#### `str(value)` → string
Converts a value to a string.

```claw
print str(42);        // "42"
print str(3.14);      // "3.14"
print str(true);      // "true"
print str(nil);       // "null"
```

#### `num(string)` → number
Converts a string to a number.

```claw
print num("42");      // 42
print num("3.14");    // 3.14
```

---

## Utility Functions

#### `clock()` → number
Returns the current time in milliseconds.

```claw
let start = clock();
// ... some code ...
let elapsed = clock() - start;
print "Elapsed time: " + elapsed + " ms";
```

#### `sleep(milliseconds)` → void
Pauses execution for the specified number of milliseconds.

```claw
print "Starting...";
sleep(1000);  // Sleep for 1 second
print "Done!";
```

---

## Class System API

### Class Definition

```claw
class ClassName {
    init(param1, param2) {
        this.property1 = param1;
        this.property2 = param2;
    }
    
    methodName() {
        // Method implementation
        return this.property1;
    }
}
```

### Inheritance

```claw
class ChildClass extends ParentClass {
    init(param) {
        super.init();  // Call parent constructor
        this.childProperty = param;
    }
    
    overrideMethod() {
        // Override parent method
        return super.overrideMethod() + " (child)";
    }
}
```

### Object Creation

```claw
let obj = ClassName("value1", "value2");
let result = obj.methodName();
```

---

## Error Handling

### Error Types

#### Division by Zero
Attempting to divide by zero will throw a runtime error.

```claw
// This will cause an error
let result = 1 / 0;
```

#### Type Errors
Using incompatible types in operations will cause errors.

```claw
// This will cause an error
let result = "hello" + 42;
```

#### Out of Bounds Access
Accessing array indices that don't exist will cause errors.

```claw
let arr = [1, 2, 3];
// This will cause an error
let value = arr[5];
```

### Error Prevention

```claw
// Safe division
fn safeDivide(a, b) {
    if (b === 0) {
        print "Error: Division by zero";
        return nil;
    }
    return a / b;
}

// Safe array access
fn safeGet(arr, index) {
    if (index < 0 || index >= arr.length) {
        print "Error: Index out of bounds";
        return nil;
    }
    return arr[index];
}
```

---

## Usage Examples

### Complete API Usage Example

```claw
// File processing example
fn processFile(filename) {
    if (!fileExists(filename)) {
        print "Error: File not found: " + filename;
        return;
    }
    
    let content = readFile(filename);
    let lines = split(content, "\n");
    
    print "Processing " + lines.length + " lines...";
    
    let wordCount = 0;
    for (let i = 0; i < lines.length; i = i + 1) {
        let words = split(trim(lines[i]), " ");
        wordCount = wordCount + words.length;
    }
    
    print "Total words: " + wordCount;
}

// Data processing example
fn processData(data) {
    // Filter numeric values
    let numbers = data.filter(fn(x) { return type(x) === "number"; });
    
    // Calculate statistics
    let sum = numbers.reduce(fn(acc, x) { return acc + x; }, 0);
    let avg = sum / numbers.length;
    let max = numbers.reduce(fn(acc, x) { return max(acc, x); }, numbers[0]);
    let min = numbers.reduce(fn(acc, x) { return min(acc, x); }, numbers[0]);
    
    return {
        "count": numbers.length,
        "sum": sum,
        "average": avg,
        "max": max,
        "min": min
    };
}

// Example usage
let sampleData = [1, 2, 3, 4, 5, "hello", true, nil];
let stats = processData(sampleData);

print "Statistics:";
print "Count: " + stats.count;
print "Sum: " + stats.sum;
print "Average: " + stats.average;
print "Max: " + stats.max;
print "Min: " + stats.min;
```

---

This API reference covers all built-in functions and methods available in ClawScript v3.0.0. For more detailed examples and usage patterns, see the main documentation.
