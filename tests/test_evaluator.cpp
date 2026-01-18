#include <gtest/gtest.h>
#include "../src/lexer.h"
#include "../src/parser.h"
#include "../src/evaluator.h"
#include "../src/value.h"

// Helper function
std::string eval(const std::string& source) {
    volt::Lexer lexer(source);
    auto tokens = lexer.tokenize();
    volt::Parser parser(tokens);
    auto ast = parser.parse();
    
    if (!ast || parser.hadError()) return "ERROR";
    
    volt::Evaluator evaluator;
    try {
        volt::Value result = evaluator.evaluate(ast.get());
        return volt::valueToString(result);
    } catch (...) {
        return "ERROR";
    }
}

// ===== LITERALS =====
TEST(Evaluator, Numbers) {
    EXPECT_EQ(eval("42"), "42");
    EXPECT_EQ(eval("3.14"), "3.140000");
}

TEST(Evaluator, Strings) {
    EXPECT_EQ(eval("\"hello\""), "hello");
    EXPECT_EQ(eval("\"world\""), "world");
}

TEST(Evaluator, Booleans) {
    EXPECT_EQ(eval("true"), "true");
    EXPECT_EQ(eval("false"), "false");
}

TEST(Evaluator, Nil) {
    EXPECT_EQ(eval("nil"), "nil");
}

// ===== ARITHMETIC =====
TEST(Evaluator, Addition) {
    EXPECT_EQ(eval("1 + 2"), "3");
    EXPECT_EQ(eval("10 + 5"), "15");
}

TEST(Evaluator, Subtraction) {
    EXPECT_EQ(eval("10 - 3"), "7");
    EXPECT_EQ(eval("5 - 10"), "-5");
}

TEST(Evaluator, Multiplication) {
    EXPECT_EQ(eval("2 * 3"), "6");
    EXPECT_EQ(eval("7 * 8"), "56");
}

TEST(Evaluator, Division) {
    EXPECT_EQ(eval("10 / 2"), "5");
    EXPECT_EQ(eval("15 / 3"), "5");
}

TEST(Evaluator, Modulo) {
    EXPECT_EQ(eval("10 % 3"), "1");
    EXPECT_EQ(eval("20 % 7"), "6");
}

TEST(Evaluator, Precedence) {
    EXPECT_EQ(eval("1 + 2 * 3"), "7");
    EXPECT_EQ(eval("10 - 4 / 2"), "8");
    EXPECT_EQ(eval("2 * 3 + 4 * 5"), "26");
}

TEST(Evaluator, Parentheses) {
    EXPECT_EQ(eval("(1 + 2) * 3"), "9");
    EXPECT_EQ(eval("(10 + 5) / 3"), "5");
}

// ===== UNARY =====
TEST(Evaluator, Negation) {
    EXPECT_EQ(eval("-5"), "-5");
    EXPECT_EQ(eval("-(10 + 5)"), "-15");
}

TEST(Evaluator, LogicalNot) {
    EXPECT_EQ(eval("!true"), "false");
    EXPECT_EQ(eval("!false"), "true");
    EXPECT_EQ(eval("!!true"), "true");
}

// ===== COMPARISON =====
TEST(Evaluator, LessThan) {
    EXPECT_EQ(eval("1 < 2"), "true");
    EXPECT_EQ(eval("5 < 3"), "false");
}

TEST(Evaluator, LessEqual) {
    EXPECT_EQ(eval("1 <= 2"), "true");
    EXPECT_EQ(eval("5 <= 5"), "true");
    EXPECT_EQ(eval("10 <= 5"), "false");
}

TEST(Evaluator, GreaterThan) {
    EXPECT_EQ(eval("5 > 3"), "true");
    EXPECT_EQ(eval("2 > 10"), "false");
}

TEST(Evaluator, GreaterEqual) {
    EXPECT_EQ(eval("10 >= 5"), "true");
    EXPECT_EQ(eval("5 >= 5"), "true");
}

TEST(Evaluator, Equality) {
    EXPECT_EQ(eval("5 == 5"), "true");
    EXPECT_EQ(eval("5 == 3"), "false");
    EXPECT_EQ(eval("true == true"), "true");
    EXPECT_EQ(eval("true == false"), "false");
}

TEST(Evaluator, Inequality) {
    EXPECT_EQ(eval("5 != 3"), "true");
    EXPECT_EQ(eval("5 != 5"), "false");
}

// ===== LOGICAL =====
TEST(Evaluator, LogicalAnd) {
    EXPECT_EQ(eval("true && true"), "true");
    EXPECT_EQ(eval("true && false"), "false");
    EXPECT_EQ(eval("false && true"), "false");
}

TEST(Evaluator, LogicalOr) {
    EXPECT_EQ(eval("true || false"), "true");
    EXPECT_EQ(eval("false || true"), "true");
    EXPECT_EQ(eval("false || false"), "false");
}

TEST(Evaluator, ShortCircuit) {
    // AND: stop if first is false
    EXPECT_EQ(eval("false && true"), "false");
    
    // OR: stop if first is true
    EXPECT_EQ(eval("true || false"), "true");
}

// ===== STRING OPERATIONS =====
TEST(Evaluator, StringConcatenation) {
    EXPECT_EQ(eval("\"hello\" + \" world\""), "hello world");
    EXPECT_EQ(eval("\"foo\" + \"bar\""), "foobar");
}

// ===== VARIABLES =====
TEST(Evaluator, VariableAssignment) {
    volt::Lexer lexer("x = 10");
    auto tokens = lexer.tokenize();
    volt::Parser parser(tokens);
    auto ast = parser.parse();
    
    volt::Evaluator evaluator;
    volt::Value result = evaluator.evaluate(ast.get());
    
    EXPECT_EQ(volt::valueToString(result), "10");
}

TEST(Evaluator, VariableReference) {
    volt::Evaluator evaluator;
    
    // Define variable
    volt::Lexer lexer1("x = 42");
    auto tokens1 = lexer1.tokenize();
    volt::Parser parser1(tokens1);
    auto ast1 = parser1.parse();
    evaluator.evaluate(ast1.get());
    
    // Use variable
    volt::Lexer lexer2("x");
    auto tokens2 = lexer2.tokenize();
    volt::Parser parser2(tokens2);
    auto ast2 = parser2.parse();
    volt::Value result = evaluator.evaluate(ast2.get());
    
    EXPECT_EQ(volt::valueToString(result), "42");
}

TEST(Evaluator, VariableArithmetic) {
    volt::Evaluator evaluator;
    
    // x = 10
    volt::Lexer lexer1("x = 10");
    auto tokens1 = lexer1.tokenize();
    volt::Parser parser1(tokens1);
    auto ast1 = parser1.parse();
    evaluator.evaluate(ast1.get());
    
    // x * 2 + 5
    volt::Lexer lexer2("x * 2 + 5");
    auto tokens2 = lexer2.tokenize();
    volt::Parser parser2(tokens2);
    auto ast2 = parser2.parse();
    volt::Value result = evaluator.evaluate(ast2.get());
    
    EXPECT_EQ(volt::valueToString(result), "25");
}

// ===== COMPLEX EXPRESSIONS =====
TEST(Evaluator, ComplexArithmetic) {
    EXPECT_EQ(eval("(5 + 3) * 2 - 10 / 5"), "14");
    EXPECT_EQ(eval("((10 + 5) * 2) / 3"), "10");
}

TEST(Evaluator, ComplexLogic) {
    EXPECT_EQ(eval("(5 > 3) && (10 < 20)"), "true");
    EXPECT_EQ(eval("(5 > 10) || (3 < 7)"), "true");
    EXPECT_EQ(eval("!(5 == 3) && (10 != 5)"), "true");
}

// ===== ERROR CASES =====
TEST(Evaluator, DivisionByZero) {
    EXPECT_EQ(eval("10 / 0"), "ERROR");
}

TEST(Evaluator, UndefinedVariable) {
    EXPECT_EQ(eval("unknownVar"), "ERROR");
}

TEST(Evaluator, TypeMismatch) {
    EXPECT_EQ(eval("5 + \"hello\""), "ERROR");
    EXPECT_EQ(eval("\"hello\" - 5"), "ERROR");
}
