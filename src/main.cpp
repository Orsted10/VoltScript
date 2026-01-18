#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "evaluator.h"
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cout << "VoltScript Milestone 4: Evaluator\n";
        std::cout << "Usage: volt \"expression\"\n\n";
        std::cout << "Examples:\n";
        std::cout << "  volt \"1 + 2 * 3\"           # Arithmetic\n";
        std::cout << "  volt \"(10 + 5) / 3\"        # Grouping\n";
        std::cout << "  volt \"x = 5\"               # Assignment\n";
        std::cout << "  volt \"x = 10; x * 2\"       # Variables\n";
        std::cout << "  volt \"5 > 3\"               # Comparison\n";
        std::cout << "  volt \"true && false\"       # Logic\n";
        std::cout << "  volt '\"hello\" + \" world\"' # Strings\n";
        return 0;
    }
    
    std::string input = argv[1];
    
    // Create evaluator (persists variables)
    volt::Evaluator evaluator;
    
    // Split by semicolon for multiple expressions
    size_t start = 0;
    size_t pos = 0;
    volt::Value lastResult = nullptr;
    
    while (pos <= input.length()) {
        if (pos == input.length() || input[pos] == ';') {
            if (pos > start) {
                std::string expr = input.substr(start, pos - start);
                
                // Skip empty expressions
                bool isEmpty = true;
                for (char c : expr) {
                    if (!std::isspace(c)) {
                        isEmpty = false;
                        break;
                    }
                }
                
                if (!isEmpty) {
                    try {
                        // Tokenize
                        volt::Lexer lexer(expr);
                        auto tokens = lexer.tokenize();
                        
                        // Parse
                        volt::Parser parser(tokens);
                        auto ast = parser.parse();
                        
                        if (parser.hadError()) {
                            for (const auto& error : parser.getErrors()) {
                                std::cerr << error << "\n";
                            }
                            return 1;
                        }
                        
                        // Evaluate
                        if (ast) {
                            lastResult = evaluator.evaluate(ast.get());
                        }
                        
                    } catch (const volt::RuntimeError& e) {
                        std::cerr << "Runtime Error [Line " << e.token.line << "]: " 
                                  << e.what() << "\n";
                        return 1;
                    } catch (const std::exception& e) {
                        std::cerr << "Error: " << e.what() << "\n";
                        return 1;
                    }
                }
            }
            start = pos + 1;
        }
        pos++;
    }
    
    // Print final result
    std::cout << volt::valueToString(lastResult) << "\n";
    
    return 0;
}
