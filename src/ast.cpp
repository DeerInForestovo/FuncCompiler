#include "ast.hpp"
#include <iostream>
#include "binop.hpp"
#include "uniop.hpp"
#include "error.hpp"
#include "type_env.hpp"

static void print_indent(int n, std::ostream& to) {
    while(n--) to << "  ";
}

void ast_int::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "INT: " << value << std::endl;
}

void ast_int::find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into) {
    this->env = env;
}

type_ptr ast_int::typecheck(type_mgr& mgr) {
    // return type_ptr(new type_app(env->lookup_type("Int")));  // Do NOT use this
    return type_ptr(new type_app(mgr.new_num_type()));  // An Int instance is num-taged-var type
}

void ast_float::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "FLOAT: " << value << std::endl;
}

void ast_float::find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into) {
    this->env = env;
}

type_ptr ast_float::typecheck(type_mgr& mgr) {
    return type_ptr(new type_app(env->lookup_type("Float")));
}

void ast_list::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "LIST:" << std::endl;
    for (auto &a : arr) a->print(indent + 1, to);
}

void ast_list::find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into) {
    this->env = env;
    for (auto &a : arr) a->find_free(mgr, env, into);
}

type_ptr ast_list::typecheck(type_mgr& mgr) {
    type_ptr arg_type = mgr.new_type();
    for (auto &a : arr)
        mgr.unify(a->typecheck(mgr), arg_type);
    type_ptr list_type = env->lookup_type("List");
    type_app* list_app = new type_app(list_type);
    list_app->arguments.emplace_back(arg_type);
    type_ptr list_app_type = type_ptr(list_app);
    return list_app_type;
}

void ast_char::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    char unParsedChar = '\0';
    switch (value) {
        case '\0':  unParsedChar = '0'; break;
        case '\n':  unParsedChar = 'n'; break;
        case '\t':  unParsedChar = 't'; break;
        case '\r':  unParsedChar = 'r'; break;
    }
    if (unParsedChar) to << "CHAR: \\" << unParsedChar << std::endl;
        else to << "CHAR: " << value << std::endl;
}

void ast_char::find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into) {
    this->env = env;
}

type_ptr ast_char::typecheck(type_mgr& mgr) {
    return type_ptr(new type_app(env->lookup_type("Char")));
}

void ast_lid::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "LID: " << id << std::endl;
}

void ast_lid::find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into) {
    this->env = env;
    if(env->lookup(id) == nullptr) into.insert(id);
}

type_ptr ast_lid::typecheck(type_mgr& mgr) {
    return env->lookup(id)->instantiate(mgr);
}

void ast_uid::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "UID: " << id << std::endl;
}

void ast_uid::find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into) {
    this->env = env;
}

type_ptr ast_uid::typecheck(type_mgr& mgr) {
    std::cout << "uid typecheck id = " << id << std::endl;
    return env->lookup(id)->instantiate(mgr);
}

void ast_binop::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "BINOP: " << binop_name(op) << std::endl;
    left->print(indent + 1, to);
    right->print(indent + 1, to);
}

void ast_binop::find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into) {
    this->env = env;
    left->find_free(mgr, env, into);
    right->find_free(mgr, env, into);
}

type_ptr ast_binop::typecheck(type_mgr& mgr) {
    type_ptr ltype = left->typecheck(mgr);
    type_ptr rtype = right->typecheck(mgr);
    type_ptr ftype = env->lookup(binop_name(op))->instantiate(mgr);
    if(!ftype) throw type_error(std::string("unknown binary operator ") + binop_name(op));

    type_ptr return_type = mgr.new_type();
    type_ptr arrow_one = type_ptr(new type_arr(rtype, return_type));
    type_ptr arrow_two = type_ptr(new type_arr(ltype, arrow_one));

    mgr.unify(arrow_two, ftype);
    return return_type;
}

void ast_uniop::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "UNIOP: " << uniop_name(op) << std::endl;
    opd->print(indent + 1, to);
}

void ast_uniop::find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into) {
    this->env = env;
    opd->find_free(mgr, env, into);
}

type_ptr ast_uniop::typecheck(type_mgr& mgr) {
    type_ptr otype = opd->typecheck(mgr);
    type_ptr ftype = env->lookup(uniop_name(op))->instantiate(mgr);
    if(!ftype) throw type_error(std::string("unknown binary operator ") + uniop_name(op));
    type_ptr return_type = mgr.new_type();
    type_ptr arrow_type = type_ptr(new type_arr(otype, return_type));
    mgr.unify(arrow_type, ftype);
    return return_type;
}

void ast_app::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "APP: " << std::endl;
    left->print(indent + 1, to);
    right->print(indent + 1, to);
}

void ast_app::find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into) {
    this->env = env;
    left->find_free(mgr, env, into);
    right->find_free(mgr, env, into);
}

type_ptr ast_app::typecheck(type_mgr& mgr) {
    type_ptr ltype = left->typecheck(mgr);
    type_ptr rtype = right->typecheck(mgr);

    type_ptr return_type = mgr.new_type();
    type_ptr arrow = type_ptr(new type_arr(rtype, return_type));
    mgr.unify(arrow, ltype);
    return return_type;
}

void ast_do::print(int indent, std::ostream &to) const {
    print_indent(indent, to);
    to << "DO: " << std::endl;
    for (auto& action: actions) {
        action->print(indent + 1, to);
    }
}

void ast_do::find_free(type_mgr &mgr, type_env_ptr &env, std::set<std::string> &into) {
    this->env = env;

    type_env_ptr curr_env = env;
    for (auto& action: actions) {
        type_env_ptr new_env = type_scope(curr_env);
        action->expr->find_free(mgr, new_env, into);
        action->insert_binding(mgr, new_env);
        curr_env = new_env;
    }
}

type_ptr ast_do::typecheck(type_mgr &mgr) {
    type_ptr return_type = mgr.new_type();
    mgr.unify(return_type, env->lookup("IOSimpleCons")->instantiate(mgr));
    
    type_ptr last_type;
    for (auto& action: actions) {
        last_type = action->typecheck(mgr);
    }

    mgr.unify(return_type, last_type);
    return return_type;
}

void action::insert_binding(type_mgr &mgr, type_env_ptr &env) {
    this->env = env;
    if (!bind_name.empty()) {
        env->bind(bind_name, mgr.new_type());
    }
}

void action_exec::print(int indent, std::ostream &to) const {
    print_indent(indent, to);
    if (bind_name.empty()) {
        to << "EXEC: " << std::endl;
    } else {
        to << "BIND " << bind_name << " TO EXEC: " << std::endl;
    }
    expr->print(indent + 1, to);
}

type_ptr action_exec::typecheck(type_mgr &mgr) {
    type_ptr body_type = expr->typecheck(mgr);
    mgr.unify(body_type, expr->env->lookup("IOSimpleCons")->instantiate(mgr));

    if (!bind_name.empty()) {
        type_ptr io_bind_ptr = env->lookup("IOBindCons")->instantiate(mgr);
        type_arr* io_bind = dynamic_cast<type_arr*>(io_bind_ptr.get());
        mgr.unify(env->lookup(bind_name)->instantiate(mgr), io_bind->left);
    }

    return body_type;
}

void action_return::print(int indent, std::ostream &to) const {
    print_indent(indent, to);
    if (bind_name.empty()) {
        to << "RETURN: " << std::endl;
    } else {
        to << "BIND " << bind_name << " TO RETURN: " << std::endl;
    }
    expr->print(indent + 1, to);
}

type_ptr action_return::typecheck(type_mgr &mgr) {
    type_ptr body_type = expr->typecheck(mgr);
    type_ptr return_type = mgr.new_type();
    mgr.unify(type_ptr(new type_arr(body_type, return_type)), env->lookup("IOBindCons")->instantiate(mgr));

    if (!bind_name.empty()) {
        mgr.unify(env->lookup(bind_name)->instantiate(mgr), body_type);
    }

    return return_type;
}

void ast_case::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "CASE: " << std::endl;
    for(auto& branch : branches) {
        print_indent(indent + 1, to);
        branch->pat->print(to);
        to << std::endl;
        branch->expr->print(indent + 2, to);
    }
}

void ast_case::find_free(type_mgr& mgr, type_env_ptr& env, std::set<std::string>& into) {
    this->env = env;
    of->find_free(mgr, env, into);
    for(auto& branch : branches) {
        type_env_ptr new_env = type_scope(env);
        branch->pat->insert_bindings(mgr, new_env);
        branch->expr->find_free(mgr, new_env, into);
    }
}

type_ptr ast_case::typecheck(type_mgr& mgr) {
    type_var* var;
    type_ptr case_type = mgr.resolve(of->typecheck(mgr), var);
    type_ptr branch_type = mgr.new_type();

    for(auto& branch : branches) {
        branch->pat->typecheck(case_type, mgr, branch->expr->env);
        type_ptr curr_branch_type = branch->expr->typecheck(mgr);
        mgr.unify(branch_type, curr_branch_type);
    }

    input_type = mgr.resolve(case_type, var);
    type_app* app_type;
    if(!(app_type = dynamic_cast<type_app*>(input_type.get())) ||
            !dynamic_cast<type_data*>(app_type->constructor.get())) {
        throw type_error("attempting case analysis of non-data type");
    }

    return branch_type;
}

void pattern_var::print(std::ostream& to) const {
    to << var;
}

void pattern_var::insert_bindings(type_mgr& mgr, type_env_ptr& env) const {
    env->bind(var, mgr.new_type());
}

void pattern_var::typecheck(type_ptr t, type_mgr& mgr, type_env_ptr& env) const {
    mgr.unify(env->lookup(var)->instantiate(mgr), t);
}

void pattern_constr::print(std::ostream& to) const {
    to << constr;
    for(auto& param : params) {
        to << " " << param;
    }
}

void pattern_constr::insert_bindings(type_mgr& mgr, type_env_ptr& env) const {
    for(auto& param : params) {
        env->bind(param, mgr.new_type());
    }
}

void pattern_constr::typecheck(type_ptr t, type_mgr& mgr, type_env_ptr& env) const {
    type_ptr constructor_type = env->lookup(constr)->instantiate(mgr);
    if(!constructor_type) {
        throw type_error(std::string("pattern using unknown constructor ") + constr);
    }

    for(auto& param : params) {
        type_arr* arr = dynamic_cast<type_arr*>(constructor_type.get());
        if(!arr) throw type_error("too many parameters in constructor pattern");

        mgr.unify(env->lookup(param)->instantiate(mgr), arr->left);
        constructor_type = arr->right;
    }

    mgr.unify(t, constructor_type);
}
