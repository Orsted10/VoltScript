#pragma once
#include <cstdint>

namespace claw {

enum class OpCode : uint8_t {
    Constant,    // Load constant from chunk
    Nil,         // Load nil
    True,        // Load true
    False,       // Load false
    Pop,         // Pop value from stack
    
    GetGlobal,   // Get global variable
    DefineGlobal,// Define global variable
    SetGlobal,   // Set global variable
    GetLocal,    // Get local variable
    SetLocal,    // Set local variable
    GetUpvalue,  // Get upvalue (for closures)
    SetUpvalue,  // Set upvalue
    CloseUpvalue,// Close upvalue
    
    Equal,       // ==
    Greater,     // >
    Less,        // <
    Add,         // +
    Subtract,    // -
    Multiply,    // *
    Divide,      // /
    BitAnd,      // &
    BitOr,       // |
    BitXor,      // ^
    ShiftLeft,   // <<
    ShiftRight,  // >>
    Not,         // !
    Negate,      // - (unary)
    
    Print,       // print
    Jump,        // Jump forward
    JumpIfFalse, // Jump forward if false
    Loop,        // Jump backward
    
    Call,        // Call function
    Closure,     // Create closure
    Return,      // Return from function
    
    Class,       // Define class
    Inherit,     // Set up inheritance
    Method,      // Define method
    Invoke,      // Call method directly
    SuperInvoke, // Call super method
    GetProperty, // Get instance property
    SetProperty, // Set instance property
    GetIndex,    // Get array/map element by index/key
    SetIndex,    // Set array/map element by index/key
    EnsureIndexDefault,    // Ensure hash key exists with default for compound ops
    EnsurePropertyDefault, // Ensure instance field exists with default for compound ops

    // Extended opcodes for new language features
    Dup,           // Duplicate top of stack
    JumpIfTrue,    // Jump forward if top is truthy (does NOT pop)
    JumpIfNil,     // Jump forward if top is nil (does NOT pop)
    JumpIfNotNil,  // Jump forward if top is not nil (does NOT pop)
    ToString,      // Convert top of stack to string in-place
    Spread,        // Spread iterable onto stack
    NewArray,      // Push new empty ClawArray
    ArrayPush,     // Pop value, push onto array at stack[-1]
    ArraySlice,    // Slice array from index (operand byte = start index)
    NewHashMap,    // Push new empty ClawHashMap
    HashMapSet,    // Pop value+key, set in hash map at stack[-1]
    GetIter,       // Get iterator from iterable on top of stack
    IterNext,      // Advance iterator: push next value (truthy) or false (done)
    Construct,     // Construct new class instance (like Call but runs __init__)
    GetMeta,       // Get metatable of instance on top of stack
    SetMeta,       // Set metatable: stack[-1]=instance, stack[-0]=meta
    Await,         // Await async value (no-op in sync mode)
    Yield,         // Yield value from generator
};

} // namespace claw
