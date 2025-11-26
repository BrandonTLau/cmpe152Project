#ifndef AST_H
#define AST_H

#include <memory>
#include <string>
#include <vector>

// ---- Expressions ----
struct Expr {
    virtual ~Expr() = default;
};

struct IntLiteralExpr : Expr {
    int value;
    explicit IntLiteralExpr(int v) : value(v) {}
};

struct StringLiteralExpr : Expr {
    std::string value;
    explicit StringLiteralExpr(std::string v) : value(std::move(v)) {}
};

struct VarExpr : Expr {
    std::string name;
    explicit VarExpr(std::string n) : name(std::move(n)) {}
};

struct ArrayAccessExpr : Expr {
    std::string arrayName;
    std::unique_ptr<Expr> index;
    ArrayAccessExpr(std::string n, std::unique_ptr<Expr> idx)
        : arrayName(std::move(n)), index(std::move(idx)) {}
};

enum class UnaryOp {
    Negate, // -x
    Plus,   // +x
    Not     // !x
};

struct UnaryExpr : Expr {
    UnaryOp op;
    std::unique_ptr<Expr> operand;
    UnaryExpr(UnaryOp o, std::unique_ptr<Expr> e)
        : op(o), operand(std::move(e)) {}
};

enum class BinaryOp {
    Add, Sub, Mul, Div,
    Equal, NotEqual,
    Less, Greater, LessEq, GreaterEq,
    And, Or
};

struct BinaryExpr : Expr {
    BinaryOp op;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    BinaryExpr(BinaryOp o, std::unique_ptr<Expr> l, std::unique_ptr<Expr> r)
        : op(o), left(std::move(l)), right(std::move(r)) {}
};

struct CallExpr : Expr {
    std::string callee;
    std::vector<std::unique_ptr<Expr>> args;
    explicit CallExpr(std::string c) : callee(std::move(c)) {}
};

// ---- Statements ----
struct Stmt {
    virtual ~Stmt() = default;
};

enum class SimpleType {
    Int,
    Char
};

struct DeclStmt : Stmt {
    SimpleType type;
    std::string name;
    explicit DeclStmt(SimpleType t, std::string n)
        : type(t), name(std::move(n)) {}
};

struct ArrayDeclStmt : Stmt {
    SimpleType type;
    std::string name;
    int size;
    ArrayDeclStmt(SimpleType t, std::string n, int s)
        : type(t), name(std::move(n)), size(s) {}
};

struct AssignStmt : Stmt {
    std::string name;
    std::unique_ptr<Expr> index; // null if scalar
    std::unique_ptr<Expr> value;

    AssignStmt(std::string n, std::unique_ptr<Expr> v)
        : name(std::move(n)), value(std::move(v)) {}

    AssignStmt(std::string n, std::unique_ptr<Expr> idx, std::unique_ptr<Expr> v)
        : name(std::move(n)), index(std::move(idx)), value(std::move(v)) {}
};

struct ExprStmt : Stmt {
    std::unique_ptr<Expr> expr;
    explicit ExprStmt(std::unique_ptr<Expr> e) : expr(std::move(e)) {}
};

struct IfStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> thenBranch;
    std::unique_ptr<Stmt> elseBranch; // may be null
};

struct WhileStmt : Stmt {
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Stmt> body;
};

struct ForStmt : Stmt {
    std::unique_ptr<Stmt> init;       // may be null
    std::unique_ptr<Expr> condition;  // may be null
    std::unique_ptr<Stmt> increment;  // may be null
    std::unique_ptr<Stmt> body;
};

struct ReturnStmt : Stmt {
    std::unique_ptr<Expr> value;
    explicit ReturnStmt(std::unique_ptr<Expr> v) : value(std::move(v)) {}
};

struct BlockStmt : Stmt {
    std::vector<std::unique_ptr<Stmt>> statements;
};

// ---- Functions / Program ----
struct Param {
    SimpleType type;
    std::string name;
};

struct FunctionDecl {
    SimpleType returnType;
    std::string name;
    std::vector<Param> params;
    std::unique_ptr<BlockStmt> body;
};

struct Program {
    std::vector<std::unique_ptr<FunctionDecl>> functions;
};

#endif
