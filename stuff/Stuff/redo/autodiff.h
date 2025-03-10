#ifndef SOGLFW_AUTODIFF_H
#define SOGLFW_AUTODIFF_H

#include <functional>
#include <string>
#include <vector>
#include <unordered_map>


namespace Simple {
    namespace AutoDiff {
        typedef float BType;
        const char *_vars_inner_ops_pfix = "__lastval";


        struct Expr {
            std::unordered_map<std::string, BType> vars;
            std::unordered_map<std::string, std::vector<std::function<BType(const std::unordered_map<std::string, BType> &)>>> _derivatives;
            std::vector<std::function<BType(const std::unordered_map<std::string, BType> &)>> expression;

            Expr(const std::string &_n, const BType &_val)
                    : vars{{_n, _val}}, _derivatives{{_n, {[](const std::unordered_map<std::string, BType> &){ return 1.f; }}}},
                      expression{[_n](const std::unordered_map<std::string, BType> &_vars){ return _vars.at(_n); }} {}

            Expr(const Expr &_expr, const std::string &_n = "")
                    : vars{_expr.vars}, _derivatives{_expr._derivatives}, expression{_expr.expression} {}

            BType derivative(const char *_n) { return _derivatives[_n].back()(vars); }
        };

        const Expr &operator+(BType _val, const Expr &_expr) {}
        const Expr &operator-(BType _val, const Expr &_expr) {}
        const Expr &operator*(BType _val, const Expr &_expr) {}
        const Expr &operator/(BType _val, const Expr &_expr) {}

        const Expr &operator+(const Expr &_expr, BType _val) {}
        const Expr &operator-(const Expr &_expr, BType _val) {}
        const Expr &operator*(const Expr &_expr, BType _val) {}
        const Expr &operator/(const Expr &_expr, BType _val) {}



// Useless except for having the name of operation
        struct Op {
            static bool upd_ops_name;
            std::string _n{"BaseOp"};
            std::function<BType(const std::unordered_map<std::string, BType> &)> _func;
            std::string &name() { return _n; }

            BType operator()(const std::unordered_map<std::string, BType> &vars) { return _func(vars); }
        };
    };
};


#endif