#include "ast.hpp"
#include <iostream>

/*
 * Type Checking
*/

std::string op_name(binop op) {
    switch(op) {
        case PLUS: return "+";
        case MINUS: return "-";
        case TIMES: return "*";
        case DIVIDE: return "/";
    }
    throw 0;
}

type_ptr ast_int::typecheck(type_mgr& mgr, const type_env& env) const {
    return type_ptr(new type_base("Int"));
}

type_ptr ast_float::typecheck(type_mgr& mgr, const type_env& env) const {
    return type_ptr(new type_base("Float"));
}

type_ptr ast_bool::typecheck(type_mgr& mgr, const type_env& env) const {
    return type_ptr(new type_base("Bool"));
}

type_ptr ast_string::typecheck(type_mgr& mgr, const type_env& env) const {
    return type_ptr(new type_base("String"));
}

type_ptr ast_lid::typecheck(type_mgr& mgr, const type_env& env) const {
    return env.lookup(id);
}

type_ptr ast_uid::typecheck(type_mgr& mgr, const type_env& env) const {
    return env.lookup(id);
}

type_ptr ast_binop::typecheck(type_mgr& mgr, const type_env& env) const {
    type_ptr ltype = left->typecheck(mgr, env);
    type_ptr rtype = right->typecheck(mgr, env);
    type_ptr ftype = env.lookup(op_name(op));
    if(!ftype) throw 0;

    type_ptr return_type = mgr.new_type();
    type_ptr arrow_one = type_ptr(new type_arr(rtype, return_type));
    type_ptr arrow_two = type_ptr(new type_arr(ltype, arrow_one));

    mgr.unify(arrow_two, ftype);
    return return_type;
}

type_ptr ast_app::typecheck(type_mgr& mgr, const type_env& env) const {
    type_ptr ltype = left->typecheck(mgr, env);
    type_ptr rtype = right->typecheck(mgr, env);

    type_ptr return_type = mgr.new_type();
    type_ptr arrow = type_ptr(new type_arr(rtype, return_type));
    mgr.unify(arrow, ltype);
    return return_type;
}

type_ptr ast_case::typecheck(type_mgr& mgr, const type_env& env) const {
    type_ptr case_type = of->typecheck(mgr, env);
    type_ptr branch_type = mgr.new_type();

    for(auto& branch : branches) {
        type_env new_env = env.scope();
        branch->pat->match(case_type, mgr, new_env);
        type_ptr curr_branch_type = branch->expr->typecheck(mgr, new_env);
        mgr.unify(branch_type, curr_branch_type);
    }

    return branch_type;
}

void pattern_var::match(type_ptr t, type_mgr& mgr, type_env& env) const {
    env.bind(var, t);
}

void pattern_constr::match(type_ptr t, type_mgr& mgr, type_env& env) const {
    type_ptr constructor_type = env.lookup(constr);
    if(!constructor_type) throw 0;

    for(int i = 0; i < params.size(); i++) {
        type_arr* arr = dynamic_cast<type_arr*>(constructor_type.get());
        if(!arr) throw 0;

        env.bind(params[i], arr->left);
        constructor_type = arr->right;
    }

    mgr.unify(t, constructor_type);
    type_base* result_type = dynamic_cast<type_base*>(constructor_type.get());
    if(!result_type) throw 0;
}

/* 
 * Display
*/

void display_tabs(int tabs) {
    while (tabs--) std::cout << '\t';
}

void display_structure (const std::string &n) {  // white background + black characters
    std::cout << "\033[30;47m" << n << "\033[0m" << "";
}

void display_function_name (const std::string &n) {  // red characters
    std::cout << "\033[1;31m" << n << "\033[0m";
}

void display_string (const std::string &n) {  // orange characters
    std::cout << "\033[1;33m" << n << "\033[0m";
}

void branch::display(int tabs) const {
    display_tabs(tabs);
    display_structure("BRANCH: ");
    pat->display();
    std::cout << std::endl;

    display_tabs(tabs + 1);
    display_structure("EXPR: ");
    std::cout << std::endl;
    expr->display(tabs + 1);
    std::cout << std::endl;
}

void constructor::display(int tabs) const {
    display_tabs(tabs);
    display_structure("CONSTRUCTOR: ");
    display_function_name(name);
    for (std::string type: types)
        std::cout << ' ' << type;
    std::cout << std::endl;
}

void ast_int::display(int tabs) const {
    display_tabs(tabs);
    std::cout << value;
}

void ast_float::display(int tabs) const {
    display_tabs(tabs);
    std::cout << value;
}

void ast_bool::display(int tabs) const {
    display_tabs(tabs);
    std::cout << (value ? "TRUE" : "FALSE");
}

void ast_string::display(int tabs) const {
    display_tabs(tabs);
    std::cout << '"';
    display_string(str);
    std::cout << '"';
}

void ast_lid::display(int tabs) const {
    display_tabs(tabs);
    std::cout << id;
}

void ast_uid::display(int tabs) const {
    display_tabs(tabs);
    std::cout << id;
}

void ast_binop::display(int tabs) const {
    display_tabs(tabs);
    display_structure("BINOP");
    std::cout << std::endl;

    display_tabs(tabs + 1);
    display_structure("BINOP-LEFT: ");
    std::cout << std::endl;
    
    left->display(tabs + 1);
    std::cout << std::endl;
    
    display_tabs(tabs + 1);
    display_structure("BINOP-RIGHT: ");
    std::cout << std::endl;

    right->display(tabs + 1);
    std::cout << std::endl;
}

void ast_uniop::display(int tabs) const {
    display_tabs(tabs);
    display_structure("UNIOP");
}

void ast_app::display(int tabs) const {
    display_tabs(tabs);
    left->display(0);
    std::cout << ' ';
    right->display(0);
}

void ast_case::display(int tabs) const {
    display_tabs(tabs);
    display_structure("CASE");
    std::cout << ' ' << (int)branches.size() << " branch(es)" << std::endl;

    display_tabs(tabs + 1);
    display_structure("OF: ");
    std::cout << std::endl;

    of->display(tabs + 1);
    std::cout << std::endl;
    
    for (const auto& branch: branches)
        branch->display(tabs + 1);
}

void ast_tuple::display(int tabs) const {
    display_tabs(tabs);
    display_structure("TUPLE");
    std::cout << ' ' << (int)terms.size() << " term(s)" << std::endl;

    for (const auto& term: terms) {
        display_tabs(tabs + 1);
        display_structure("TERM");
        std::cout << std::endl;
        term->display(tabs + 1);
        std::cout << std::endl;
    }
}

void ast_list::display(int tabs) const {
    display_tabs(tabs);
    display_structure("LIST");
    std::cout << ' ' << (int)terms.size() << " term(s)" << std::endl;

    for (const auto& term: terms) {
        display_tabs(tabs + 1);
        display_structure("TERM");
        std::cout << std::endl;
        term->display(tabs + 1);
        std::cout << std::endl;
    }
}

void ast_index::display(int tabs) const {
    display_tabs(tabs);
    display_structure("INDEX");
    std::cout << ' ';
    index->display(0);
    std::cout << std::endl;
}

void action_exec::display(int tabs) const {
    display_tabs(tabs);
    display_structure("EXEC: ");
    std::cout << std::endl;

    body->display(tabs + 1);
    std::cout << std::endl;
}

void action_return::display(int tabs) const {
    display_tabs(tabs);
    display_structure("RETURN: ");
    std::cout << std::endl;

    body->display(tabs + 1);
    std::cout << std::endl;
}

void action_bind::display(int tabs) const {
    display_tabs(tabs);
    display_structure("BIND ");
    display_function_name(name);
    std::cout << ' ';

    act->display(tabs);
    std::cout << std::endl;
}

void pattern_var::display() const {
    display_structure("PATTERN_VAR: ");
    std::cout << var;
}

void pattern_constr::display() const {
    display_structure("PATTERN_CONSTR: ");
    display_function_name(constr);
    for (const auto& param: params)
        std::cout << ' ' << param;
}

void definition_defn::display(int tabs) const {
    display_tabs(tabs);
    display_structure("DEFN: ");
    display_function_name(name);
    for (const auto& param: params)
        std::cout << ' ' << param;
    std::cout << std::endl;

    body->display(tabs + 1);
    std::cout << std::endl;
}

void definition_defn_action::display(int tabs) const {
    display_tabs(tabs);
    display_structure("DEFN: ");
    display_function_name(name);
    for (const auto& param: params)
        std::cout << ' ' << param;
    std::cout << std::endl;

    for (const auto& action: body)
        action->display(tabs + 1);
}

void definition_data::display(int tabs) const {
    display_tabs(tabs);
    display_structure("DATA: ");
    std::cout << name << std::endl;

    for (const auto& constructor: constructors)
        constructor->display(tabs + 1);
}