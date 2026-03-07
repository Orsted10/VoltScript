// parser.cpp — DEPRECATED
// All parser implementations have been split into:
//   parser_core.cpp    — constructor, parseProgram, parseExpression, token helpers
//   parser_stmts.cpp   — statement(), printStatement(), letStatement(), fnStatement(),
//                        classStatement(), switchStatement(), matchStatement(), etc.
//   parser_stmts2.cpp  — enumStatement(), interfaceStatement(), deferStatement(),
//                        withStatement(), labeledOrExprStatement(), expressionStatement(),
//                        decoratorStatement()
//   parser_exprs.cpp   — expression(), assignment(), ternary(), all binary/unary/postfix,
//                        primary(), arrayLiteral(), hashMapOrBlock(), fstringExpression(), etc.
//
// This file is intentionally empty to avoid duplicate symbol errors.
// The CMakeLists.txt glob picks up all .cpp files in src/, so the split files
// provide all implementations.
