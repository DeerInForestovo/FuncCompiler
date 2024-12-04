#include "ast.hpp"
#include <ostream>
#include "binop.hpp"
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
    return type_ptr(new type_app(env->lookup_type("Int")));
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
    return env->lookup(id)->instantiate(mgr);
}

void ast_binop::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "BINOP: " << op_name(op) << std::endl;
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
    type_ptr ftype = env->lookup(op_name(op))->instantiate(mgr);
    if(!ftype) throw type_error(std::string("unknown binary operator ") + op_name(op));

    type_ptr return_type = mgr.new_type();
    type_ptr arrow_one = type_ptr(new type_arr(rtype, return_type));
    type_ptr arrow_two = type_ptr(new type_arr(ltype, arrow_one));

    mgr.unify(arrow_two, ftype);
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
