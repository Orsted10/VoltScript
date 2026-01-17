#include <gtest/gtest.h>
#include "../src/lexer.h"

TEST(Lexer, Numbers) {
    volt::Lexer lexer("42");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(tokens.size(), 2); // NUMBER + EOF
    EXPECT_EQ(tokens[0].type, volt::TokenType::Number);
    EXPECT_EQ(tokens[0].lexeme, "42");
}

TEST(Lexer, Operators) {
    volt::Lexer lexer("+ - * /");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(tokens.size(), 5); // 4 operators + EOF
    EXPECT_EQ(tokens[0].type, volt::TokenType::Plus);
    EXPECT_EQ(tokens[1].type, volt::TokenType::Minus);
    EXPECT_EQ(tokens[2].type, volt::TokenType::Star);
    EXPECT_EQ(tokens[3].type, volt::TokenType::Slash);
}

TEST(Lexer, Expression) {
    volt::Lexer lexer("1 + 2 * 3");
    auto tokens = lexer.tokenize();
    
    ASSERT_EQ(tokens.size(), 6); // 3 numbers + 2 ops + EOF
    EXPECT_EQ(tokens[0].lexeme, "1");
    EXPECT_EQ(tokens[1].type, volt::TokenType::Plus);
    EXPECT_EQ(tokens[2].lexeme, "2");
    EXPECT_EQ(tokens[3].type, volt::TokenType::Star);
    EXPECT_EQ(tokens[4].lexeme, "3");
}
