#include "lexer.h"
#include "parser.h"
#include "interpreter.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

void runFile(const std::string& path, volt::Interpreter& interpreter) {
    std::ifstream file(path);
    if (!file) {
        std::cerr << "Could not open file: " << path << "\n";
        exit(74);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    
    // Tokenize
    volt::Lexer lexer(source);
    auto tokens = lexer.tokenize();
    
    // Parse
    volt::Parser parser(tokens);
    auto statements = parser.parseProgram();
    
    if (parser.hadError()) {
        for (const auto& error : parser.getErrors()) {
            std::cerr << error << "\n";
        }
        exit(65);
    }
    
    // Execute
    try {
        interpreter.execute(statements);
    } catch (const volt::RuntimeError& e) {
        std::cerr << "Runtime Error [Line " << e.token.line << "]: " 
                  << e.what() << "\n";
        exit(70);
    }
}

void runPrompt() {
    volt::Interpreter interpreter;
    std::string line;
    
    std::cout << "VoltScript v0.5.0 REPL\n";
    std::cout << "Type 'exit' to quit\n\n";
    
    while (true) {
        std::cout << "> ";
        if (!std::getline(std::cin, line)) break;
        
        if (line == "exit" || line == "quit") break;
        if (line.empty()) continue;
        
        try {
            // Tokenize
            volt::Lexer lexer(line);
            auto tokens = lexer.tokenize();
            
            // Parse
            volt::Parser parser(tokens);
            auto statements = parser.parseProgram();
            
            if (parser.hadError()) {
                for (const auto& error : parser.getErrors()) {
                    std::cerr << error << "\n";
                }
                continue;
            }
            
            // Execute
            interpreter.execute(statements);
            
        } catch (const volt::RuntimeError& e) {
            std::cerr << "Runtime Error: " << e.what() << "\n";
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
}

int main(int argc, char** argv) {
    if (argc > 2) {
        std::cout << "Usage: volt [script]\n";
        return 64;
    }
    
    volt::Interpreter interpreter;
    
    if (argc == 2) {
        // Run file
        runFile(argv[1], interpreter);
    } else {
        // Interactive REPL
        runPrompt();
    }
    
    return 0;
}
