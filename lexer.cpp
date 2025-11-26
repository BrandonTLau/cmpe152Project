#include "lexer.h"
#include <cctype>

Lexer::Lexer(const std::string& src)
    : src(src), pos(0), line(1), column(1) {}

char Lexer::current() const {
    if (pos >= src.size()) return '\0';
    return src[pos];
}

char Lexer::peek(int offset) const {
    if (pos + offset >= src.size()) return '\0';
    return src[pos + offset];
}

void Lexer::advance() {
    if (current() == '\n') {
        line++;
        column = 1;
    } else {
        column++;
    }
    pos++;
}

bool Lexer::isAlpha(char c) const {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

bool Lexer::isDigit(char c) const {
    return std::isdigit(static_cast<unsigned char>(c));
}

/* ============================================================
   COMMENTS + WHITESPACE
   ============================================================ */

void Lexer::skipWhitespaceAndComments() {
    while (true) {
        char c = current();

        // whitespace
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            advance();
            continue;
        }

        // single-line comment: //
        if (c == '/' && peek(1) == '/') {
            advance(); // '/'
            advance(); // '/'
            while (current() != '\n' && current() != '\0') {
                advance();
            }
            continue;
        }

        // block comment: /* ... */
        if (c == '/' && peek(1) == '*') {
            advance(); // '/'
            advance(); // '*'

            while (true) {
                if (current() == '\0') {
                    // unterminated block comment — just stop
                    return;
                }
                if (current() == '*' && peek(1) == '/') {
                    advance(); // '*'
                    advance(); // '/'
                    break;
                }
                advance();
            }
            continue;
        }

        break; // nothing more to skip
    }
}

/* ============================================================
   TOKEN HELPERS
   ============================================================ */

Token Lexer::stringLiteral() {
    int startCol = column;
    advance(); // skip opening "

    std::string value;
    while (current() != '"' && current() != '\0') {
        // basic escape handling
        if (current() == '\\') {
            char n = peek(1);
            if (n == 'n') {
                value += '\n';
                advance();
                advance();
                continue;
            } else if (n == 't') {
                value += '\t';
                advance();
                advance();
                continue;
            }
        }
        value += current();
        advance();
    }

    if (current() == '"') {
        advance(); // skip closing "
    }

    return {TokenType::STRING_LITERAL, value, line, startCol};
}

Token Lexer::charLiteral() {
    int startCol = column;
    advance(); // skip opening '

    char value = current();
    if (value == '\0' || value == '\n') {
        return {TokenType::INVALID, "unterminated char literal", line, startCol};
    }
    advance(); // consume the character

    if (current() == '\'') {
        advance(); // skip closing '
        return {TokenType::CHAR_LITERAL, std::string(1, value), line, startCol};
    } else {
        return {TokenType::INVALID, "unterminated char literal", line, startCol};
    }
}

Token Lexer::identifier() {
    int startCol = column;
    std::string value;

    while (isAlpha(current()) || isDigit(current())) {
        value += current();
        advance();
    }

    // keywords
    if (value == "int")    return {TokenType::KW_INT,     value, line, startCol};
    if (value == "return") return {TokenType::KW_RETURN,  value, line, startCol};
    if (value == "if")     return {TokenType::KW_IF,      value, line, startCol};
    if (value == "else")   return {TokenType::KW_ELSE,    value, line, startCol};
    if (value == "while")  return {TokenType::KW_WHILE,   value, line, startCol};
    if (value == "for")    return {TokenType::KW_FOR,     value, line, startCol};
    if (value == "printf") return {TokenType::KW_PRINTF,  value, line, startCol};
    if (value == "char")   return {TokenType::KW_CHAR,    value, line, startCol};

    return {TokenType::IDENTIFIER, value, line, startCol};
}

Token Lexer::number() {
    int startCol = column;
    std::string value;

    while (isDigit(current())) {
        value += current();
        advance();
    }

    return {TokenType::NUMBER_INT, value, line, startCol};
}

/* ============================================================
   MAIN TOKENIZER
   ============================================================ */

std::vector<Token> Lexer::tokenize() {
    std::vector<Token> tokens;

    while (true) {
        skipWhitespaceAndComments();

        char c = current();

        if (c == '\0') {
            tokens.push_back({TokenType::END_OF_FILE, "", line, column});
            break;
        }

        if (isAlpha(c)) {
            tokens.push_back(identifier());
            continue;
        }

        if (isDigit(c)) {
            tokens.push_back(number());
            continue;
        }

        if (c == '"') {
            tokens.push_back(stringLiteral());
            continue;
        }

        if (c == '\'') {
            tokens.push_back(charLiteral());
            continue;
        }

        int col = column;

        switch (c) {
            case '+': tokens.push_back({TokenType::OP_PLUS, "+", line, col}); break;
            case '-': tokens.push_back({TokenType::OP_MINUS, "-", line, col}); break;
            case '*': tokens.push_back({TokenType::OP_MULTIPLY, "*", line, col}); break;
            case '/': tokens.push_back({TokenType::OP_DIVIDE, "/", line, col}); break;

            case '=':
                if (peek(1) == '=') {
                    advance();
                    tokens.push_back({TokenType::OP_EQUAL, "==", line, col});
                } else {
                    tokens.push_back({TokenType::OP_ASSIGN, "=", line, col});
                }
                break;

            case '!':
                if (peek(1) == '=') {
                    advance();
                    tokens.push_back({TokenType::OP_NOT_EQUAL, "!=", line, col});
                } else {
                    tokens.push_back({TokenType::OP_NOT, "!", line, col});
                }
                break;

            case '<':
                if (peek(1) == '=') {
                    advance();
                    tokens.push_back({TokenType::OP_LESS_EQUAL, "<=", line, col});
                } else {
                    tokens.push_back({TokenType::OP_LESS, "<", line, col});
                }
                break;

            case '>':
                if (peek(1) == '=') {
                    advance();
                    tokens.push_back({TokenType::OP_GREATER_EQUAL, ">=", line, col});
                } else {
                    tokens.push_back({TokenType::OP_GREATER, ">", line, col});
                }
                break;

            case '&':
                if (peek(1) == '&') {
                    advance();
                    tokens.push_back({TokenType::OP_AND, "&&", line, col});
                } else {
                    tokens.push_back({TokenType::INVALID, "&", line, col});
                }
                break;

            case '|':
                if (peek(1) == '|') {
                    advance();
                    tokens.push_back({TokenType::OP_OR, "||", line, col});
                } else {
                    tokens.push_back({TokenType::INVALID, "|", line, col});
                }
                break;

            case '(' : tokens.push_back({TokenType::PUNCT_LPAREN,    "(", line, col}); break;
            case ')' : tokens.push_back({TokenType::PUNCT_RPAREN,    ")", line, col}); break;
            case '{' : tokens.push_back({TokenType::PUNCT_LBRACE,    "{", line, col}); break;
            case '}' : tokens.push_back({TokenType::PUNCT_RBRACE,    "}", line, col}); break;
            case ';' : tokens.push_back({TokenType::PUNCT_SEMI,      ";", line, col}); break;
            case ',' : tokens.push_back({TokenType::PUNCT_COMMA,     ",", line, col}); break;
            case '[' : tokens.push_back({TokenType::PUNCT_LBRACKET,  "[", line, col}); break;
            case ']' : tokens.push_back({TokenType::PUNCT_RBRACKET,  "]", line, col}); break;

            default:
                tokens.push_back({TokenType::INVALID, std::string(1, c), line, col});
                break;
        }

        advance();
    }

    return tokens;
}
