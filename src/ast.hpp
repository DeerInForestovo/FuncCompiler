#include <memory>
#include <vector>
#include <string>

struct ast {
    virtual ~ast() = default;
};

using ast_ptr = std::unique_ptr<ast>;

struct pattern {
    virtual ~pattern() = default;
};

using pattern_ptr = std::unique_ptr<pattern>;

struct branch {
    pattern_ptr pat;
    ast_ptr expr;

    branch(pattern_ptr p, ast_ptr a)
        : pat(std::move(p)), expr(std::move(a)) {}
};

using branch_ptr = std::unique_ptr<branch>;

struct constructor {
    std::string name;
    std::vector<std::string> types;

    constructor(std::string n, std::vector<std::string> ts)
        : name(std::move(n)), types(std::move(ts)) {}
};

using constructor_ptr = std::unique_ptr<constructor>;

struct definition {
    virtual ~definition() = default;
};

using definition_ptr = std::unique_ptr<definition>;

enum binop {
    PLUS,
    MINUS,
    TIMES,
    DIVIDE
};

struct ast_int : public ast {
    int value;

    explicit ast_int(int v)
        : value(v) {}
};

struct ast_lid : public ast {
    std::string id;

    explicit ast_lid(std::string i)
        : id(std::move(i)) {}
};

struct ast_uid : public ast {
    std::string id;

    explicit ast_uid(std::string i)
        : id(std::move(i)) {}
};

struct ast_binop : public ast {
    binop op;  
    ast_ptr left;
    ast_ptr right;

    ast_binop(binop o, ast_ptr l, ast_ptr r)
        : op(o), left(std::move(l)), right(std::move(r)) {}
};

struct ast_app : public ast {
    ast_ptr left;
    ast_ptr right;

    ast_app(ast_ptr l, ast_ptr r)
        : left(std::move(l)), right(std::move(r)) {}
};

struct ast_case : public ast {
    ast_ptr of;
    std::vector<branch_ptr> branches;

    ast_case(ast_ptr o, std::vector<branch_ptr> b)
        : of(std::move(o)), branches(std::move(b)) {}
};

struct pattern_var : public pattern {
    std::string var;

    pattern_var(std::string v)
        : var(std::move(v)) {}
};

struct pattern_constr : public pattern {
    std::string constr;
    std::vector<std::string> params;

    pattern_constr(std::string c, std::vector<std::string> p)
        : constr(std::move(c)), params(std::move(p)) {}
};

struct definition_defn : public definition {
    std::string name;
    std::vector<std::string> params;
    ast_ptr body;

    definition_defn(std::string n, std::vector<std::string> p, ast_ptr b)
        : name(std::move(n)), params(std::move(p)), body(std::move(b)) {}
};

struct definition_data : public definition {
    std::string name;
    std::vector<constructor_ptr> constructors;

    definition_data(std::string n, std::vector<constructor_ptr> cs)
        : name(std::move(n)), constructors(std::move(cs)) {}
};