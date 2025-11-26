#ifndef LEXER_H
#define LEXER_H

#include <string>
#include <vector>
#include "tokens.h"

class Lexer {
public:
    explicit Lexer(const std::string& src);

    std::vector<Token> tokenize();

private:
    std::string src;
    size_t pos;
    int line, column;

    char current() const;
    char peek(int offset) const;
    void advance();

    bool isAlpha(char c) const;
    bool isDigit(char c) const;

    void skipWhitespaceAndComments();

    Token stringLiteral();
    Token identifier();
    Token number();
    Token charLiteral();
};

#endif
