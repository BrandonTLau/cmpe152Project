#include "parser.h"
#include <iostream>
#include <cstdlib>

Parser::Parser(const std::vector<Token>& tokens)
    : tokens(tokens), pos(0) {}

const Token& Parser::current() const {
    return tokens[pos];
}

bool Parser::check(TokenType type) const {
    return current().type == type;
}

bool Parser::match(TokenType type) {
    if (check(type)) {
        advance();
        return true;
    }
    return false;
}

void Parser::advance() {
    if (pos < tokens.size()) pos++;
}

void Parser::error(const std::string& msg) {
    errors.push_back({msg, current().line, current().column});
}

void Parser::synchronize() {
    while (!check(TokenType::END_OF_FILE) &&
           !check(TokenType::PUNCT_SEMI) &&
           !check(TokenType::PUNCT_RBRACE)) {
        advance();
    }
    if (check(TokenType::PUNCT_SEMI)) advance();
}

bool Parser::hasErrors() const {
    return !errors.empty();
}

void Parser::printErrors() const {
    for (const auto& e : errors) {
        std::cout << "Error at line " << e.line
                  << ", col " << e.column
                  << ": " << e.message << "\n";
    }
}

SimpleType Parser::parseType() {
    if (match(TokenType::KW_INT))  return SimpleType::Int;
    if (match(TokenType::KW_CHAR)) return SimpleType::Char;

    error("Expected type (int or char)");
    return SimpleType::Int;
}

std::unique_ptr<Program> Parser::parseProgram() {
    std::unique_ptr<Program> prog(new Program());
    while (!check(TokenType::END_OF_FILE)) {
        auto fn = parseFunctionDecl();
        if (fn) prog->functions.push_back(std::move(fn));
        else break;
    }
    return std::move(prog);
}

std::unique_ptr<FunctionDecl> Parser::parseFunctionDecl() {
    SimpleType retType = parseType();

    if (!check(TokenType::IDENTIFIER)) {
        error("Expected function name");
        synchronize();
        return nullptr;
    }
    std::string name = current().lexeme;
    advance();

    if (!match(TokenType::PUNCT_LPAREN)) {
        error("Expected '(' after function name");
        synchronize();
        return nullptr;
    }

    std::vector<Param> params;
    if (!check(TokenType::PUNCT_RPAREN)) {
        do {
            SimpleType pt = parseType();
            if (!check(TokenType::IDENTIFIER)) {
                error("Expected parameter name");
                synchronize();
                return nullptr;
            }
            std::string pname = current().lexeme;
            advance();
            params.push_back({pt, pname});
        } while (match(TokenType::PUNCT_COMMA));
    }

    if (!match(TokenType::PUNCT_RPAREN)) {
        error("Expected ')' after parameters");
        synchronize();
        return nullptr;
    }

    auto body = parseBlock();
    if (!body) {
        error("Expected function body");
        return nullptr;
    }

    std::unique_ptr<FunctionDecl> fn(new FunctionDecl());
    fn->returnType = retType;
    fn->name = name;
    fn->params = std::move(params);
    fn->body = std::move(body);
    return std::move(fn);
}

std::unique_ptr<BlockStmt> Parser::parseBlock() {
    if (!match(TokenType::PUNCT_LBRACE)) {
        error("Expected '{'");
        synchronize();
        return nullptr;
    }

    std::unique_ptr<BlockStmt> block(new BlockStmt());
    while (!check(TokenType::PUNCT_RBRACE) &&
           !check(TokenType::END_OF_FILE)) {
        auto s = parseStmt();
        if (s) block->statements.push_back(std::move(s));
        else synchronize();
    }

    if (!match(TokenType::PUNCT_RBRACE)) {
        error("Expected '}'");
    }

    return std::move(block);
}

std::unique_ptr<Stmt> Parser::parseStmt() {
    if (check(TokenType::KW_INT) || check(TokenType::KW_CHAR))
        return parseDecl();
    if (check(TokenType::KW_RETURN))
        return parseReturnStmt();
    if (check(TokenType::KW_IF))
        return parseIfStmt();
    if (check(TokenType::KW_WHILE))
        return parseWhileStmt();
    if (check(TokenType::KW_FOR))
        return parseForStmt();
    if (check(TokenType::PUNCT_LBRACE))
        return parseBlock();

    return parseExprOrAssignStmt();
}

std::unique_ptr<Stmt> Parser::parseDecl() {
    SimpleType t = parseType();

    if (!check(TokenType::IDENTIFIER)) {
        error("Expected variable name");
        synchronize();
        return nullptr;
    }

    std::string name = current().lexeme;
    advance();

    // Array declaration: int a[5];
    if (match(TokenType::PUNCT_LBRACKET)) {
        if (!check(TokenType::NUMBER_INT)) {
            error("Expected array size");
            synchronize();
            return nullptr;
        }

        int size = std::stoi(current().lexeme);
        advance();

        if (!match(TokenType::PUNCT_RBRACKET)) {
            error("Expected ']'");
        }

        if (!match(TokenType::PUNCT_SEMI)) {
            error("Expected ';' after array declaration");
            synchronize();
        }

        return std::unique_ptr<Stmt>(new ArrayDeclStmt(t, name, size));
    }

    if (!match(TokenType::PUNCT_SEMI)) {
        error("Expected ';' after declaration");
        synchronize();
    }

    return std::unique_ptr<Stmt>(new DeclStmt(t, name));
}

std::unique_ptr<Stmt> Parser::parseReturnStmt() {
    match(TokenType::KW_RETURN);
    auto value = parseExpr();

    if (!match(TokenType::PUNCT_SEMI)) {
        error("Expected ';' after return");
        synchronize();
    }

    return std::unique_ptr<Stmt>(new ReturnStmt(std::move(value)));
}

std::unique_ptr<Stmt> Parser::parseIfStmt() {
    match(TokenType::KW_IF);

    if (!match(TokenType::PUNCT_LPAREN))
        error("Expected '(' after if");

    auto cond = parseExpr();

    if (!match(TokenType::PUNCT_RPAREN))
        error("Expected ')' after if condition");

    auto thenBr = parseStmt();
    std::unique_ptr<Stmt> elseBr;

    if (match(TokenType::KW_ELSE))
        elseBr = parseStmt();

    std::unique_ptr<IfStmt> node(new IfStmt());
    node->condition = std::move(cond);
    node->thenBranch = std::move(thenBr);
    node->elseBranch = std::move(elseBr);
    return std::unique_ptr<Stmt>(node.release());
}

std::unique_ptr<Stmt> Parser::parseWhileStmt() {
    match(TokenType::KW_WHILE);

    if (!match(TokenType::PUNCT_LPAREN))
        error("Expected '(' after while");

    auto cond = parseExpr();

    if (!match(TokenType::PUNCT_RPAREN))
        error("Expected ')' after while condition");

    auto body = parseStmt();

    std::unique_ptr<WhileStmt> node(new WhileStmt());
    node->condition = std::move(cond);
    node->body = std::move(body);
    return std::unique_ptr<Stmt>(node.release());
}

std::unique_ptr<Stmt> Parser::parseForStmt() {
    match(TokenType::KW_FOR);

    if (!match(TokenType::PUNCT_LPAREN))
        error("Expected '(' after for");

    // init
    std::unique_ptr<Stmt> init;
    if (!check(TokenType::PUNCT_SEMI)) {
        if (check(TokenType::KW_INT) || check(TokenType::KW_CHAR))
            init = parseDecl();
        else
            init = parseExprOrAssignStmt();
    } else {
        match(TokenType::PUNCT_SEMI);
    }

    // condition
    std::unique_ptr<Expr> cond;
    if (!check(TokenType::PUNCT_SEMI))
        cond = parseExpr();

    if (!match(TokenType::PUNCT_SEMI))
        error("Expected ';' after for condition");

    // increment (no trailing ';')
    std::unique_ptr<Stmt> incr;
    if (!check(TokenType::PUNCT_RPAREN))
        incr = parseIncrementStmt();

    if (!match(TokenType::PUNCT_RPAREN))
        error("Expected ')' after for clauses");

    auto body = parseStmt();

    std::unique_ptr<ForStmt> node(new ForStmt());
    node->init = std::move(init);
    node->condition = std::move(cond);
    node->increment = std::move(incr);
    node->body = std::move(body);
    return std::unique_ptr<Stmt>(node.release());
}

std::unique_ptr<Stmt> Parser::parseIncrementStmt() {
    if (check(TokenType::IDENTIFIER) && pos + 1 < tokens.size()) {
        std::string name = current().lexeme;

        if (tokens[pos + 1].type == TokenType::PUNCT_LBRACKET) {
            // a[i] = expr
            advance(); // name
            match(TokenType::PUNCT_LBRACKET);
            auto idx = parseExpr();
            if (!match(TokenType::PUNCT_RBRACKET)) {
                error("Expected ']' after array index");
            }
            if (!match(TokenType::OP_ASSIGN)) {
                error("Expected '=' after array index in for increment");
                synchronize();
                return nullptr;
            }
            auto value = parseExpr();
            return std::unique_ptr<Stmt>(new AssignStmt(name, std::move(idx), std::move(value)));
        } else if (tokens[pos + 1].type == TokenType::OP_ASSIGN) {
            // scalar assignment x = expr
            advance(); // name
            advance(); // '='
            auto value = parseExpr();
            return std::unique_ptr<Stmt>(new AssignStmt(name, std::move(value)));
        }
    }

    // fallback: expression (like i = i + 1 or ++i desugared)
    auto e = parseExpr();
    return std::unique_ptr<Stmt>(new ExprStmt(std::move(e)));
}

std::unique_ptr<Stmt> Parser::parseExprOrAssignStmt() {
    if (check(TokenType::IDENTIFIER) && pos + 1 < tokens.size()) {
        std::string name = current().lexeme;

        // array element assignment: a[i] = expr;
        if (tokens[pos + 1].type == TokenType::PUNCT_LBRACKET) {
            advance(); // name
            match(TokenType::PUNCT_LBRACKET);
            auto idx = parseExpr();
            if (!match(TokenType::PUNCT_RBRACKET)) {
                error("Expected ']' after array index");
            }
            if (!match(TokenType::OP_ASSIGN)) {
                error("Expected '=' after array index");
                synchronize();
                return nullptr;
            }
            auto value = parseExpr();
            if (!match(TokenType::PUNCT_SEMI)) {
                error("Expected ';' after assignment");
                synchronize();
            }
            return std::unique_ptr<Stmt>(new AssignStmt(name, std::move(idx), std::move(value)));
        }

        // scalar assignment: x = expr;
        if (tokens[pos + 1].type == TokenType::OP_ASSIGN) {
            advance(); // name
            advance(); // '='
            auto value = parseExpr();
            if (!match(TokenType::PUNCT_SEMI)) {
                error("Expected ';' after assignment");
                synchronize();
            }
            return std::unique_ptr<Stmt>(new AssignStmt(name, std::move(value)));
        }
    }

    // expression statement
    auto e = parseExpr();
    if (!match(TokenType::PUNCT_SEMI)) {
        error("Expected ';' after expression");
        synchronize();
    }
    return std::unique_ptr<Stmt>(new ExprStmt(std::move(e)));
}

// ===== EXPRESSION PARSING =====

std::unique_ptr<Expr> Parser::parseExpr() {
    return parseOr();
}

std::unique_ptr<Expr> Parser::parseOr() {
    auto left = parseAnd();
    while (check(TokenType::OP_OR)) {
        match(TokenType::OP_OR);
        auto right = parseAnd();
        std::unique_ptr<Expr> tmp(
            new BinaryExpr(BinaryOp::Or, std::move(left), std::move(right))
        );
        left = std::move(tmp);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseAnd() {
    auto left = parseEquality();
    while (check(TokenType::OP_AND)) {
        match(TokenType::OP_AND);
        auto right = parseEquality();
        std::unique_ptr<Expr> tmp(
            new BinaryExpr(BinaryOp::And, std::move(left), std::move(right))
        );
        left = std::move(tmp);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseEquality() {
    auto left = parseRelational();
    while (check(TokenType::OP_EQUAL) || check(TokenType::OP_NOT_EQUAL)) {
        BinaryOp op = BinaryOp::Equal;

        if (check(TokenType::OP_EQUAL)) {
            match(TokenType::OP_EQUAL);
        } else {
            match(TokenType::OP_NOT_EQUAL);
            op = BinaryOp::NotEqual;
        }

        auto right = parseRelational();
        std::unique_ptr<Expr> tmp(
            new BinaryExpr(op, std::move(left), std::move(right))
        );
        left = std::move(tmp);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseRelational() {
    auto left = parseAdditive();
    while (check(TokenType::OP_LESS) || check(TokenType::OP_GREATER) ||
           check(TokenType::OP_LESS_EQUAL) || check(TokenType::OP_GREATER_EQUAL)) {

        BinaryOp op = BinaryOp::Less;

        if (check(TokenType::OP_LESS)) {
            match(TokenType::OP_LESS);
        } else if (check(TokenType::OP_GREATER)) {
            match(TokenType::OP_GREATER);
            op = BinaryOp::Greater;
        } else if (check(TokenType::OP_LESS_EQUAL)) {
            match(TokenType::OP_LESS_EQUAL);
            op = BinaryOp::LessEq;
        } else {
            match(TokenType::OP_GREATER_EQUAL);
            op = BinaryOp::GreaterEq;
        }

        auto right = parseAdditive();
        std::unique_ptr<Expr> tmp(
            new BinaryExpr(op, std::move(left), std::move(right))
        );
        left = std::move(tmp);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseAdditive() {
    auto left = parseMultiplicative();
    while (check(TokenType::OP_PLUS) || check(TokenType::OP_MINUS)) {
        BinaryOp op = BinaryOp::Add;

        if (check(TokenType::OP_PLUS)) {
            match(TokenType::OP_PLUS);
        } else {
            match(TokenType::OP_MINUS);
            op = BinaryOp::Sub;
        }

        auto right = parseMultiplicative();
        std::unique_ptr<Expr> tmp(
            new BinaryExpr(op, std::move(left), std::move(right))
        );
        left = std::move(tmp);
    }
    return left;
}

std::unique_ptr<Expr> Parser::parseMultiplicative() {
    auto left = parseUnary();
    while (check(TokenType::OP_MULTIPLY) || check(TokenType::OP_DIVIDE)) {
        BinaryOp op = BinaryOp::Mul;

        if (check(TokenType::OP_MULTIPLY)) {
            match(TokenType::OP_MULTIPLY);
        } else {
            match(TokenType::OP_DIVIDE);
            op = BinaryOp::Div;
        }

        auto right = parseUnary();
        std::unique_ptr<Expr> tmp(
            new BinaryExpr(op, std::move(left), std::move(right))
        );
        left = std::move(tmp);
    }

    return left;
}

std::unique_ptr<Expr> Parser::parseUnary() {
    if (match(TokenType::OP_MINUS)) {
        auto operand = parseUnary();
        return std::unique_ptr<Expr>(
            new UnaryExpr(UnaryOp::Negate, std::move(operand))
        );
    }
    if (match(TokenType::OP_PLUS)) {
        auto operand = parseUnary();
        return std::unique_ptr<Expr>(
            new UnaryExpr(UnaryOp::Plus, std::move(operand))
        );
    }
    if (match(TokenType::OP_NOT)) {
        auto operand = parseUnary();
        return std::unique_ptr<Expr>(
            new UnaryExpr(UnaryOp::Not, std::move(operand))
        );
    }

    return parsePrimary();
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    if (check(TokenType::NUMBER_INT)) {
        int v = std::stoi(current().lexeme);
        advance();
        return std::unique_ptr<Expr>(new IntLiteralExpr(v));
    }

    if (check(TokenType::CHAR_LITERAL)) {
        char ch = current().lexeme.empty() ? '\0' : current().lexeme[0];
        int v = static_cast<int>(ch);
        advance();
        return std::unique_ptr<Expr>(new IntLiteralExpr(v));
    }

    if (check(TokenType::STRING_LITERAL)) {
        std::string s = current().lexeme;
        advance();
        return std::unique_ptr<Expr>(new StringLiteralExpr(s));
    }

    if (check(TokenType::IDENTIFIER) || check(TokenType::KW_PRINTF)) {
        std::string name = current().lexeme;
        advance();

        // array access: a[expr]
        if (match(TokenType::PUNCT_LBRACKET)) {
            auto idx = parseExpr();
            if (!match(TokenType::PUNCT_RBRACKET)) {
                error("Expected ']' after array index");
            }
            return std::unique_ptr<Expr>(new ArrayAccessExpr(name, std::move(idx)));
        }

        // function call?
        if (match(TokenType::PUNCT_LPAREN)) {
            std::unique_ptr<CallExpr> call(new CallExpr(name));

            if (!check(TokenType::PUNCT_RPAREN)) {
                do {
                    call->args.push_back(parseExpr());
                } while (match(TokenType::PUNCT_COMMA));
            }

            if (!match(TokenType::PUNCT_RPAREN))
                error("Expected ')' after function call");

            return std::unique_ptr<Expr>(call.release());
        }

        return std::unique_ptr<Expr>(new VarExpr(name));
    }

    if (match(TokenType::PUNCT_LPAREN)) {
        auto e = parseExpr();
        if (!match(TokenType::PUNCT_RPAREN))
            error("Expected ')'");
        return e;
    }

    error("Unexpected token in expression");
    synchronize();
    return std::unique_ptr<Expr>(new IntLiteralExpr(0));
}
