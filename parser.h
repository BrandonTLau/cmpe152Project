#ifndef PARSER_H
#define PARSER_H

#include <vector>
#include <memory>
#include <string>
#include "tokens.h"
#include "ast.h"

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    std::unique_ptr<Program> parseProgram();
    bool hasErrors() const;
    void printErrors() const;

private:
    const std::vector<Token>& tokens;
    size_t pos;

    struct ParseError {
        std::string message;
        int line;
        int column;
    };
    std::vector<ParseError> errors;

    const Token& current() const;
    bool check(TokenType type) const;
    bool match(TokenType type);
    void advance();
    void error(const std::string& msg);
    void synchronize();

    SimpleType parseType();

    std::unique_ptr<FunctionDecl> parseFunctionDecl();
    std::unique_ptr<BlockStmt> parseBlock();

    std::unique_ptr<Stmt> parseStmt();
    std::unique_ptr<Stmt> parseDecl();
    std::unique_ptr<Stmt> parseReturnStmt();
    std::unique_ptr<Stmt> parseIfStmt();
    std::unique_ptr<Stmt> parseWhileStmt();
    std::unique_ptr<Stmt> parseForStmt();
    std::unique_ptr<Stmt> parseExprOrAssignStmt();
    std::unique_ptr<Stmt> parseIncrementStmt();

    // expressions
    std::unique_ptr<Expr> parseExpr();
    std::unique_ptr<Expr> parseOr();
    std::unique_ptr<Expr> parseAnd();
    std::unique_ptr<Expr> parseEquality();
    std::unique_ptr<Expr> parseRelational();
    std::unique_ptr<Expr> parseAdditive();
    std::unique_ptr<Expr> parseMultiplicative();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parsePrimary();
};

#endif
