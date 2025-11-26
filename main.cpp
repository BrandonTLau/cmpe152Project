#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "lexer.h"
#include "parser.h"
#include "interpreter.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <source.c>\n";
        return 1;
    }

    std::ifstream in(argv[1]);
    if (!in) {
        std::cerr << "Could not open file: " << argv[1] << "\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string source = buffer.str();

    Lexer lexer(source);
    auto tokens = lexer.tokenize();

    std::cout << "\n=== TOKEN LIST ===\n";
    for (size_t i = 0; i < tokens.size(); ++i) {
        std::cout << static_cast<int>(tokens[i].type)
                  << "  '" << tokens[i].lexeme
                  << "'  (" << tokens[i].line
                  << "," << tokens[i].column << ")\n";
    }
    std::cout << "===================\n\n";

    Parser parser(tokens);
    auto program = parser.parseProgram();

    if (parser.hasErrors()) {
        std::cout << "\n=== PARSER ERRORS ===\n";
        parser.printErrors();
        std::cout << "======================\n";
        return 1;
    }

    Interpreter interp;
    try {
        int result = interp.run(*program);
        std::cout << "Program exited with code " << result << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Runtime error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}
