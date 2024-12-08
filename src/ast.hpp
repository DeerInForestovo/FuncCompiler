#pragma once
#include <memory>
#include <vector>
#include <set>
#include "type.hpp"
#include "type_env.hpp"
#include "binop.hpp"
#include "uniop.hpp"
#include "env.hpp"

struct ast {
    type_env_ptr env;

    virtual ~ast() = default;

    virtual void print(int indent, std::ostream& to) const = 0;
    virtual void find_free(type_mgr& mgr,
        type_env_ptr& env, std::set<std::string>& into) = 0;
    virtual type_ptr typecheck(type_mgr& mgr) = 0;
};

using ast_ptr = std::unique_ptr<ast>;

struct pattern {
    virtual ~pattern() = default;

    virtual void print(std::ostream& to) const = 0;
    virtual void insert_bindings(type_mgr& mgr, type_env_ptr& env) const = 0;
    virtual void typecheck(type_ptr t, type_mgr& mgr, type_env_ptr& env) const = 0;
};

using pattern_ptr = std::unique_ptr<pattern>;

struct branch {
    pattern_ptr pat;
    ast_ptr expr;

    branch(pattern_ptr p, ast_ptr a)
        : pat(std::move(p)), expr(std::move(a)) {}
};

using branch_ptr = std::unique_ptr<branch>;

struct ast_int : public ast {
    int value;

    explicit ast_int(int v)
        : value(v) {}

    void print(int indent, std::ostream& to) const;
    void find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into);
    type_ptr typecheck(type_mgr& mgr);
};

struct ast_float : public ast {
    float value;

    explicit ast_float(float v)
        : value(v) {}

    void print(int indent, std::ostream& to) const;
    void find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into);
    type_ptr typecheck(type_mgr& mgr);
};

// struct ast_bool : public ast {
//     bool value;

//     explicit ast_bool(bool v)
//         : value(v) {}

//     void print(int indent, std::ostream& to) const;
//     void find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into);
//     type_ptr typecheck(type_mgr& mgr);
// };

struct ast_char : public ast {
    char value;

    explicit ast_char(char v)
        : value(v) {}

    void print(int indent, std::ostream& to) const;
    void find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into);
    type_ptr typecheck(type_mgr& mgr);
};

struct ast_list : public ast {
    std::vector<ast_ptr> arr;

    explicit ast_list(std::vector<ast_ptr> a)
        : arr(std::move(a)) {}

    explicit ast_list(std::string s) {
        arr = std::vector<ast_ptr>();
        for (char c : s) arr.push_back(ast_ptr(new ast_char(c)));
    }

    void print(int indent, std::ostream& to) const;
    void find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into);
    type_ptr typecheck(type_mgr& mgr);
};

struct ast_connect : public ast {
    ast_ptr left;
    ast_ptr right;

    ast_connect(ast_ptr l, ast_ptr r)
        : left(std::move(l)), right(std::move(r)) {}

    void print(int indent, std::ostream& to) const;
    void find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into);
    type_ptr typecheck(type_mgr& mgr);
};

struct ast_index : public ast {
    ast_ptr arr;
    ast_ptr ind;

    ast_index(ast_ptr a, ast_ptr i)
        : arr(std::move(a)), ind(std::move(i)) {}

    void print(int indent, std::ostream& to) const;
    void find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into);
    type_ptr typecheck(type_mgr& mgr);
};

struct ast_lid : public ast {
    std::string id;

    explicit ast_lid(std::string i)
        : id(std::move(i)) {}

    void print(int indent, std::ostream& to) const;
    void find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into);
    type_ptr typecheck(type_mgr& mgr);
};

struct ast_uid : public ast {
    std::string id;

    explicit ast_uid(std::string i)
        : id(std::move(i)) {}

    void print(int indent, std::ostream& to) const;
    void find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into);
    type_ptr typecheck(type_mgr& mgr);
};

struct ast_binop : public ast {
    binop op;
    ast_ptr left;
    ast_ptr right;

    ast_binop(binop o, ast_ptr l, ast_ptr r)
        : op(o), left(std::move(l)), right(std::move(r)) {}

    void print(int indent, std::ostream& to) const;
    void find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into);
    type_ptr typecheck(type_mgr& mgr);
};

struct ast_uniop : public ast {
    uniop op;
    ast_ptr opd;

    ast_uniop(uniop o, ast_ptr od)
        : op(o), opd(std::move(od)) {}

    void print(int indent, std::ostream& to) const;
    void find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into);
    type_ptr typecheck(type_mgr& mgr);
};

struct ast_app : public ast {
    ast_ptr left;
    ast_ptr right;

    ast_app(ast_ptr l, ast_ptr r)
        : left(std::move(l)), right(std::move(r)) {}

    void print(int indent, std::ostream& to) const;
    void find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into);
    type_ptr typecheck(type_mgr& mgr);
};

struct ast_case : public ast {
    ast_ptr of;
    type_ptr input_type;
    std::vector<branch_ptr> branches;

    ast_case(ast_ptr o, std::vector<branch_ptr> b)
        : of(std::move(o)), branches(std::move(b)) {}

    void print(int indent, std::ostream& to) const;
    void find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into);
    type_ptr typecheck(type_mgr& mgr);
};

struct pattern_var : public pattern {
    std::string var;

    pattern_var(std::string v)
        : var(std::move(v)) {}

    void print(std::ostream &to) const;
    void insert_bindings(type_mgr& mgr, type_env_ptr& env) const;
    void typecheck(type_ptr t, type_mgr& mgr, type_env_ptr& env) const;
};

struct pattern_constr : public pattern {
    std::string constr;
    std::vector<std::string> params;

    pattern_constr(std::string c, std::vector<std::string> p)
        : constr(std::move(c)), params(std::move(p)) {}

    void print(std::ostream &to) const;
    virtual void insert_bindings(type_mgr& mgr, type_env_ptr& env) const;
    virtual void typecheck(type_ptr t, type_mgr& mgr, type_env_ptr& env) const;
};
