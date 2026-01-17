#include "lexer.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "VoltScript Milestone 1: Tokenizer\n";
        std::cout << "Usage: volt \"expression\"\n";
        std::cout << "Example: volt \"1 + 2 * 3\"\n";
        return 0;
    }
    
    std::string input = argv[1];
    volt::Lexer lexer(input);
    
    auto tokens = lexer.tokenize();
    
    std::cout << "Tokens:\n";
    for (const auto& token : tokens) {
        std::cout << "  " << volt::tokenName(token.type) 
                  << " '" << token.lexeme << "'\n";
    }
    
    return 0;
}
