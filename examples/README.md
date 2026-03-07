# ClawScript Examples

This directory contains example programs demonstrating various features of ClawScript v3.0.0.

## Directory Structure

```
examples/
├── basic/              # Beginner-friendly examples
├── advanced/           # Advanced feature demonstrations
└── performance/        # Performance and optimization examples
```

## Basic Examples

### Getting Started
- **hello.claw** - Classic "Hello, World!" program
- **variables.claw** - Variable declarations and basic types
- **functions.claw** - Function definition and calling
- **control_flow.claw** - If statements, loops, and control structures
- **arrays.claw** - Array creation and manipulation
- **objects.claw** - Object creation and property access
- **strings.claw** - String manipulation and operations

### Data Structures
- **data_structures.claw** - Arrays, objects, and nested structures
- **sorting.claw** - Various sorting algorithms
- **searching.claw** - Linear and binary search implementations

### File Operations
- **file_io.claw** - Reading and writing files
- **file_processing.claw** - Processing text files line by line
- **json_handling.claw** - JSON parsing and generation

## Advanced Examples

### Object-Oriented Programming
- **classes.claw** - Class definitions and inheritance
- **polymorphism.claw** - Method overriding and polymorphism
- **design_patterns.claw** - Common design patterns in ClawScript

### Functional Programming
- **functional_basics.claw** - Higher-order functions and closures
- **function_composition.claw** - Function composition and piping
- **recursion.claw** - Recursive algorithms and patterns

### Algorithms
- **algorithms.claw** - Common algorithms (sorting, searching, etc.)
- **data_structures_advanced.claw** - Linked lists, trees, graphs
- **math_algorithms.claw** - Mathematical algorithms and computations

### System Integration
- **system_calls.claw** - System-level operations and APIs
- **networking.claw** - Network programming examples
- **concurrency.claw** - Multi-threading and parallel processing

## Performance Examples

### Benchmarking
- **benchmarking.claw** - Performance measurement and comparison
- **optimization.claw** - Code optimization techniques
- **profiling.claw** - Built-in profiler usage

### Memory Management
- **memory_efficiency.claw** - Memory usage optimization
- **garbage_collection.claw** - Understanding garbage collection
- **large_data_processing.claw** - Processing large datasets efficiently

## Running Examples

### Basic Execution
```bash
# Run a specific example
./clawscript examples/basic/hello.claw

# Run with JIT compilation
./clawscript --jit examples/advanced/algorithms.claw
```

### Interactive Testing
```bash
# Start REPL and load example
./clawscript
> load examples/basic/arrays.claw
> testArrayFunction()
```

### Batch Testing
```bash
# Run all basic examples
for file in examples/basic/*.claw; do
    echo "Running $file:"
    ./clawscript "$file"
    echo "---"
done
```

## Learning Path

### 1. Complete Beginner
Start with `basic/` examples to understand:
- Language syntax and basic constructs
- Variable types and operations
- Control flow and functions
- Basic data structures

### 2. Intermediate Concepts
Move to `advanced/` examples for:
- Object-oriented programming
- Functional programming patterns
- Algorithm implementation
- System integration

### 3. Performance Optimization
Use `performance/` examples to learn:
- Benchmarking techniques
- Memory optimization
- Algorithm efficiency
- Profiling and analysis

## Contributing Examples

When adding new examples:

1. **Choose appropriate directory** based on complexity
2. **Use descriptive filenames** that indicate the feature demonstrated
3. **Include comments** explaining the concepts
4. **Test thoroughly** before submitting
5. **Update this README** with new example descriptions

### Example Template

```claw
// example_name.claw
// Description: Brief description of what this example demonstrates
// Author: Your Name
// Date: YYYY-MM-DD

// Import statements (if needed)
// import "module.claw";

// Main demonstration
fn main() {
    // Example code here
    print "Example output";
}

// Run the example
main();
```

## Best Practices

### Code Style
- Use meaningful variable and function names
- Include comments for complex logic
- Follow consistent indentation (4 spaces)
- Add type hints where helpful

### Documentation
- Explain the purpose of each example
- Document any prerequisites or dependencies
- Include expected output in comments
- Provide usage instructions

### Testing
- Test examples with different inputs
- Verify error handling
- Check performance characteristics
- Ensure cross-platform compatibility

## Troubleshooting

### Common Issues
1. **File not found**: Ensure correct path and file extension
2. **Syntax errors**: Check for balanced brackets and proper syntax
3. **Runtime errors**: Verify logic and handle edge cases
4. **Performance issues**: Consider algorithmic complexity

### Getting Help
- Check the [User Guide](../docs/USER_GUIDE.md) for detailed explanations
- Review [API Reference](../docs/API_REFERENCE.md) for function documentation
- Search [GitHub Issues](https://github.com/your-org/clawscript/issues) for similar problems
- Join [GitHub Discussions](https://github.com/your-org/clawscript/discussions) for community support

## Additional Resources

### Documentation
- [Main Documentation](../docs/DOCUMENTATION.md)
- [API Reference](../docs/API_REFERENCE.md)
- [Migration Guide](../docs/MIGRATION_GUIDE.md)

### Community
- [GitHub Repository](https://github.com/your-org/clawscript)
- [Issue Tracker](https://github.com/your-org/clawscript/issues)
- [Discussion Forum](https://github.com/your-org/clawscript/discussions)

---

These examples are designed to help you learn ClawScript effectively. Start with the basic examples and gradually progress to more advanced concepts as you become comfortable with the language.
