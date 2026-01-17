# VoltScript - Milestone 1: Tokenizer

## Features
- Tokenize numbers (integers and floats)
- Tokenize operators: `+`, `-`, `*`, `/`
- Tokenize parentheses: `(`, `)`
- Tokenize identifiers: `x`, `myVar`, `_test`
- Skip whitespace and newlines
- Track line numbers for errors

## Build Instructions

### Linux/macOS
```bash
# Create build directory
mkdir build
cd build
# Configure
cmake ..
# Build
make
# Run
./bin/volt "1 + 2 * 3"

### Windows (Visual Studio) 
mkdir build
cd build
cmake ..
cmake --build . --config Release
bin\Release\volt.exe "1 + 2 * 3"

### Usage
# Basic expression
./bin/volt "42"
# With operators
./bin/volt "1 + 2 * 3"
# With parentheses
./bin/volt "(10 + 5) / 3"
# With variables
./bin/volt "x * 2 + y"

### Expected Output
$ ./bin/volt "1 + 2 * 3"
Tokens:
  NUMBER '1'
  PLUS '+'
  NUMBER '2'
  STAR '*'
  NUMBER '3'
  EOF ''

###Testing
If Google Test is installed:
cd build
ctest --output-on-failure
# Or run directly
./bin/volt_tests