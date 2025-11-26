#ifndef TOKENS_H
#define TOKENS_H

#include <string>

enum class TokenType {
    // Keywords
    KW_INT,
    KW_RETURN,
    KW_IF,
    KW_ELSE,
    KW_WHILE,
    KW_FOR,
    KW_PRINTF,
    KW_CHAR,

    // Identifiers & literals
    IDENTIFIER,
    NUMBER_INT,
    STRING_LITERAL,
    CHAR_LITERAL,

    // Operators
    OP_PLUS,
    OP_MINUS,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_ASSIGN,
    OP_EQUAL,
    OP_NOT_EQUAL,
    OP_LESS,
    OP_GREATER,
    OP_LESS_EQUAL,
    OP_GREATER_EQUAL,
    OP_AND,
    OP_OR,
    OP_NOT,   // '!'

    // Punctuation
    PUNCT_LPAREN,
    PUNCT_RPAREN,
    PUNCT_LBRACE,
    PUNCT_RBRACE,
    PUNCT_SEMI,
    PUNCT_COMMA,
    PUNCT_LBRACKET,
    PUNCT_RBRACKET,

    // Special
    END_OF_FILE,
    INVALID
};

struct Token {
    TokenType type;
    std::string lexeme;
    int line;
    int column;
};

#endif

