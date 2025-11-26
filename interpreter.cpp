#include "interpreter.h"
#include <iostream>
#include <stdexcept>

// =========================================================
//   SCOPE MANAGEMENT
// =========================================================

void Interpreter::pushScope() {
    scopes.emplace_back();
}

void Interpreter::popScope() {
    if (!scopes.empty()) scopes.pop_back();
}

bool Interpreter::hasVar(const std::string& name) const {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        if (it->vars.find(name) != it->vars.end()) return true;
    }
    return false;
}

Value Interpreter::getVar(const std::string& name) const {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto f = it->vars.find(name);
        if (f != it->vars.end()) return f->second;
    }
    throw std::runtime_error("Runtime error: variable '" + name + "' not declared");
}

void Interpreter::setVar(const std::string& name, const Value& v) {
    for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
        auto f = it->vars.find(name);
        if (f != it->vars.end()) {
            f->second = v;
            return;
        }
    }
    scopes.back().vars[name] = v;
}

void Interpreter::declareVar(const std::string& name) {
    Value v;
    v.isArray = false;
    v.i = 0;
    scopes.back().vars[name] = v;
}

// =========================================================
//   FUNCTION TABLE
// =========================================================

void Interpreter::initFunctions(const Program& program) {
    functions.clear();
    for (const auto& fn : program.functions) {
        functions[fn->name] = fn.get();
    }
}

int Interpreter::run(const Program& program) {
    initFunctions(program);

    auto it = functions.find("main");
    if (it == functions.end()) {
        std::cerr << "Runtime error: no 'main' function.\n";
        return 1;
    }

    pushScope();
    auto res = execFunction(*it->second, {});
    popScope();
    return res.returnValue.i;
}

// =========================================================
//   FUNCTION EXECUTION
// =========================================================

Interpreter::ExecResult
Interpreter::execFunction(const FunctionDecl& fn,
                          const std::vector<Value>& args) {
    pushScope();

    // bind parameters
    for (size_t i = 0; i < fn.params.size(); ++i) {
        const auto& p = fn.params[i];
        declareVar(p.name);
        if (i < args.size()) {
            setVar(p.name, args[i]);
        }
    }

    ExecResult res = execBlock(*fn.body);

    popScope();
    return res;
}

// =========================================================
//   BLOCK EXECUTION
// =========================================================

Interpreter::ExecResult Interpreter::execBlock(const BlockStmt& block) {
    pushScope();

    ExecResult res;
    for (const auto& stmt : block.statements) {
        res = execStmt(*stmt);
        if (res.hasReturn) {
            popScope();
            return res;
        }
    }

    popScope();
    return res;
}

// =========================================================
//   STATEMENT EXECUTION
// =========================================================

Interpreter::ExecResult Interpreter::execStmt(const Stmt& stmt) {
    ExecResult res;

    if (auto d = dynamic_cast<const DeclStmt*>(&stmt)) {
        declareVar(d->name);
        return res;
    }

    if (auto arr = dynamic_cast<const ArrayDeclStmt*>(&stmt)) {
        declareVar(arr->name);
        Value v;
        v.isArray = true;
        v.arr.resize(arr->size);
        v.i = 0;
        setVar(arr->name, v);
        return res;
    }

    if (auto a = dynamic_cast<const AssignStmt*>(&stmt)) {
        Value val = evalExpr(*a->value);

        if (a->index) {
            // array element assignment
            Value arrVal = getVar(a->name);
            if (!arrVal.isArray) {
                throw std::runtime_error("Runtime error: '" + a->name + "' is not an array");
            }
            Value idxVal = evalExpr(*a->index);
            int idx = idxVal.i;
            if (idx < 0 || idx >= static_cast<int>(arrVal.arr.size())) {
                throw std::runtime_error("Runtime error: array index out of bounds");
            }
            arrVal.arr[idx] = val.i;
            setVar(a->name, arrVal);
        } else {
            // scalar assignment
            if (!hasVar(a->name)) {
                throw std::runtime_error(
                    "Runtime error: assignment to undeclared variable '" + a->name + "'");
            }
            Value v;
            v.isArray = false;
            v.i = val.i;
            setVar(a->name, v);
        }
        return res;
    }

    if (auto e = dynamic_cast<const ExprStmt*>(&stmt)) {
        (void)evalExpr(*e->expr);
        return res;
    }

    if (auto r = dynamic_cast<const ReturnStmt*>(&stmt)) {
        Value v = evalExpr(*r->value);
        res.hasReturn = true;
        res.returnValue = v;
        return res;
    }

    if (auto b = dynamic_cast<const BlockStmt*>(&stmt)) {
        return execBlock(*b);
    }

    if (auto i = dynamic_cast<const IfStmt*>(&stmt)) {
        Value cond = evalExpr(*i->condition);
        if (cond.i != 0) {
            return execStmt(*i->thenBranch);
        } else if (i->elseBranch) {
            return execStmt(*i->elseBranch);
        }
        return res;
    }

    if (auto w = dynamic_cast<const WhileStmt*>(&stmt)) {
        while (true) {
            Value cond = evalExpr(*w->condition);
            if (cond.i == 0) break;

            auto bodyRes = execStmt(*w->body);
            if (bodyRes.hasReturn) return bodyRes;
        }
        return res;
    }

    if (auto f = dynamic_cast<const ForStmt*>(&stmt)) {
        if (f->init) execStmt(*f->init);

        while (true) {
            if (f->condition) {
                Value cond = evalExpr(*f->condition);
                if (cond.i == 0) break;
            }

            auto bodyRes = execStmt(*f->body);
            if (bodyRes.hasReturn) return bodyRes;

            if (f->increment) {
                auto incRes = execStmt(*f->increment);
                if (incRes.hasReturn) return incRes;
            }
        }

        return res;
    }

    return res;
}

// =========================================================
//   EXPRESSION EVALUATION
// =========================================================

Value Interpreter::evalExpr(const Expr& expr) {
    // int literal
    if (auto lit = dynamic_cast<const IntLiteralExpr*>(&expr)) {
        Value v;
        v.isArray = false;
        v.i = lit->value;
        return v;
    }

    // string literal (used by printf)
    if (auto s = dynamic_cast<const StringLiteralExpr*>(&expr)) {
        Value v;
        v.isArray = false;
        v.i = 0;
        return v;
    }

    // variable
    if (auto vexpr = dynamic_cast<const VarExpr*>(&expr)) {
        Value v = getVar(vexpr->name);
        if (v.isArray) {
            throw std::runtime_error("Runtime error: cannot use array '" + vexpr->name + "' as scalar");
        }
        return v;
    }

    // array access
    if (auto aexpr = dynamic_cast<const ArrayAccessExpr*>(&expr)) {
        Value arrVal = getVar(aexpr->arrayName);
        if (!arrVal.isArray) {
            throw std::runtime_error("Runtime error: '" + aexpr->arrayName + "' is not an array");
        }
        Value idxVal = evalExpr(*aexpr->index);
        int idx = idxVal.i;
        if (idx < 0 || idx >= static_cast<int>(arrVal.arr.size())) {
            throw std::runtime_error("Runtime error: array index out of bounds");
        }
        Value v;
        v.isArray = false;
        v.i = arrVal.arr[idx];
        return v;
    }

    // unary
    if (auto un = dynamic_cast<const UnaryExpr*>(&expr)) {
        Value val = evalExpr(*un->operand);
        Value out;
        out.isArray = false;
        switch (un->op) {
            case UnaryOp::Negate: out.i = -val.i; break;
            case UnaryOp::Plus:   out.i = +val.i; break;
            case UnaryOp::Not:    out.i = (val.i == 0); break;
        }
        return out;
    }

    // binary
    if (auto bin = dynamic_cast<const BinaryExpr*>(&expr)) {
        Value lv = evalExpr(*bin->left);
        Value rv = evalExpr(*bin->right);
        Value out;
        out.isArray = false;

        switch (bin->op) {
            case BinaryOp::Add: out.i = lv.i + rv.i; break;
            case BinaryOp::Sub: out.i = lv.i - rv.i; break;
            case BinaryOp::Mul: out.i = lv.i * rv.i; break;
            case BinaryOp::Div:
                if (rv.i == 0) throw std::runtime_error("Division by zero");
                out.i = lv.i / rv.i;
                break;

            case BinaryOp::Equal:      out.i = (lv.i == rv.i); break;
            case BinaryOp::NotEqual:   out.i = (lv.i != rv.i); break;
            case BinaryOp::Less:       out.i = (lv.i <  rv.i); break;
            case BinaryOp::Greater:    out.i = (lv.i >  rv.i); break;
            case BinaryOp::LessEq:     out.i = (lv.i <= rv.i); break;
            case BinaryOp::GreaterEq:  out.i = (lv.i >= rv.i); break;
            case BinaryOp::And:        out.i = (lv.i && rv.i); break;
            case BinaryOp::Or:         out.i = (lv.i || rv.i); break;
        }

        return out;
    }

    // call expression (printf or user function)
    if (auto call = dynamic_cast<const CallExpr*>(&expr)) {

        // ---------- built-in printf ----------
        if (call->callee == "printf") {
            if (call->args.empty()) {
                throw std::runtime_error("printf expects at least a format string");
            }

            auto fmtNode = dynamic_cast<StringLiteralExpr*>(call->args[0].get());
            if (!fmtNode) {
                throw std::runtime_error("printf first argument must be a string literal");
            }
            const std::string& fmt = fmtNode->value;

            // pre-evaluate all non-format arguments as ints
            std::vector<Value> argVals;
            for (size_t i = 1; i < call->args.size(); ++i) {
                argVals.push_back(evalExpr(*call->args[i]));
            }

            size_t argIndex = 0;

            for (size_t i = 0; i < fmt.size(); ++i) {
                char c = fmt[i];

                // escape sequences
                if (c == '\\' && i + 1 < fmt.size()) {
                    char n = fmt[i + 1];
                    if (n == 'n') {
                        std::cout << '\n';
                        ++i;
                        continue;
                    } else if (n == 't') {
                        std::cout << '\t';
                        ++i;
                        continue;
                    }
                    std::cout << '\\';
                    continue;
                }

                // %d
                if (c == '%' && i + 1 < fmt.size() && fmt[i + 1] == 'd') {
                    if (argIndex >= argVals.size()) {
                        throw std::runtime_error("printf: not enough arguments for %d");
                    }
                    std::cout << argVals[argIndex].i;
                    ++argIndex;
                    ++i; // skip 'd'
                    continue;
                }

                std::cout << c;
            }

            // ignore extra args (like C UB; we just don't crash)
            Value v;
            v.isArray = false;
            v.i = 0;
            return v;
        }

        // user-defined function
        auto it = functions.find(call->callee);
        if (it == functions.end()) {
            throw std::runtime_error("Runtime error: unknown function '" + call->callee + "'");
        }

        std::vector<Value> argVals;
        for (const auto& a : call->args) {
            argVals.push_back(evalExpr(*a));
        }

        auto res = execFunction(*it->second, argVals);
        if (!res.hasReturn) {
            Value v;
            v.isArray = false;
            v.i = 0;
            return v;
        }
        return res.returnValue;
    }

    throw std::runtime_error("Unknown expression at runtime");
}
