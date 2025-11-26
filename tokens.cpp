#include "tokens.h"

std::string tokenTypeToString(TokenType t) {
    switch(t) {
        case TokenType::KEYWORD_INT: return "KEYWORD_INT";
        case TokenType::KEYWORD_RETURN: return "KEYWORD_RETURN";
        case TokenType::KEYWORD_IF: return "KEYWORD_IF";
        case TokenType::KEYWORD_ELSE: return "KEYWORD_ELSE";
        case TokenType::KEYWORD_WHILE: return "KEYWORD_WHILE";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER_INT: return "NUMBER_INT";
        case TokenType::NUMBER_FLOAT: return "NUMBER_FLOAT";
        case TokenType::OP_ASSIGN: return "OP_ASSIGN";
        case TokenType::OP_PLUS: return "OP_PLUS";
        case TokenType::OP_MINUS: return "OP_MINUS";
        case TokenType::OP_MULTIPLY: return "OP_MULTIPLY";
        case TokenType::OP_DIVIDE: return "OP_DIVIDE";
        case TokenType::OP_MODULO: return "OP_MODULO";
        case TokenType::OP_EQUAL: return "OP_EQUAL";
        case TokenType::OP_NOT_EQUAL: return "OP_NOT_EQUAL";
        case TokenType::OP_LESS: return "OP_LESS";
        case TokenType::OP_GREATER: return "OP_GREATER";
        case TokenType::OP_LESS_EQUAL: return "OP_LESS_EQUAL";
        case TokenType::OP_GREATER_EQUAL: return "OP_GREATER_EQUAL";
        case TokenType::PUNCT_LPAREN: return "PUNCT_LPAREN";
        case TokenType::PUNCT_RPAREN: return "PUNCT_RPAREN";
        case TokenType::PUNCT_LBRACE: return "PUNCT_LBRACE";
        case TokenType::PUNCT_RBRACE: return "PUNCT_RBRACE";
        case TokenType::PUNCT_SEMICOLON: return "PUNCT_SEMICOLON";
        case TokenType::COMMA: return "COMMA";
        case TokenType::UNKNOWN: return "UNKNOWN";
        case TokenType::TOKEN_EOF: return "TOKEN_EOF";
        default: return "UNKNOWN";
    }
}
