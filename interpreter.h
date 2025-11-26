#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "ast.h"
#include <unordered_map>
#include <string>
#include <vector>

struct Value {
    bool isArray = false;
    std::vector<int> arr;
    int i = 0; // scalar value if !isArray
};

struct Scope {
    std::unordered_map<std::string, Value> vars;
};

class Interpreter {
public:
    // Run the whole program, return main()'s int result
    int run(const Program& program);

private:
    struct ExecResult {
        bool hasReturn = false;
        Value returnValue;
    };

    std::vector<Scope> scopes;
    std::unordered_map<std::string, const FunctionDecl*> functions;

    // scope management
    void pushScope();
    void popScope();

    // variables
    bool hasVar(const std::string& name) const;
    Value getVar(const std::string& name) const;
    void setVar(const std::string& name, const Value& v);
    void declareVar(const std::string& name);

    // functions
    void initFunctions(const Program& program);
    ExecResult execFunction(const FunctionDecl& fn,
                            const std::vector<Value>& args);

    // execution
    ExecResult execBlock(const BlockStmt& block);
    ExecResult execStmt(const Stmt& stmt);

    // expressions
    Value evalExpr(const Expr& expr);
};

#endif
