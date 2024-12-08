#include "ast.hpp"
#include <iostream>
#include "binop.hpp"
#include "definition.hpp"
#include "graph.hpp"
#include "parser.hpp"
#include "error.hpp"
#include "type.hpp"

void yy::parser::error(const std::string& msg) {
    std::cout << "An error occured: " << msg << std::endl;
}

extern std::map<std::string, definition_data_ptr> defs_data;
extern std::map<std::string, definition_defn_ptr> defs_defn;

void typecheck_program(
        const std::map<std::string, definition_data_ptr>& defs_data,
        const std::map<std::string, definition_defn_ptr>& defs_defn,
        type_mgr& mgr, type_env_ptr& env) {
    /*
        Insert types
    */
    // basic types
    type_var* num_type_var = new type_var("Num");
    num_type_var->set_num_type();
    type_ptr num_type = type_ptr(num_type_var);
    type_ptr num_type_app = type_ptr(new type_app(num_type));

    type_ptr int_type = type_ptr(new type_base("Int"));
    type_ptr int_type_app = type_ptr(new type_app(int_type));
    env->bind_type("Int", int_type);

    type_ptr float_type = type_ptr(new type_base("Float"));
    env->bind_type("Float", float_type);

    type_ptr char_type = type_ptr(new type_base("Char"));
    env->bind_type("Char", char_type);

    type_ptr list_type = type_ptr(new type_base("List"));
    env->bind_type("List", list_type);

    // data types
    if (defs_data.find("Bool") != defs_data.end()) {
        std::cout << "error: User self-defined Bool type." << std::endl;
        throw 0;
    }
    constructor_ptr true_constructor = constructor_ptr(new constructor("True", std::vector<parsed_type_ptr>()));
    constructor_ptr false_constructor = constructor_ptr(new constructor("False", std::vector<parsed_type_ptr>()));
    std::vector<constructor_ptr> bool_constructors;
    bool_constructors.push_back(std::move(true_constructor));
    bool_constructors.push_back(std::move(false_constructor));
    definition_data_ptr bool_type = definition_data_ptr(new 
            definition_data("Bool", std::vector<std::string>(), std::move(bool_constructors)));
    bool_type->insert_types(env);
    bool_type->insert_constructors();
    type_ptr bool_type_app = type_ptr(new type_app(
            type_ptr(new type_data("Bool"))));
    // insert all data definitions
    for(auto& def_data : defs_data) {
        def_data.second->insert_types(env);
    }
    std::cout << "insert_types, finished." << std::endl;
    for(auto& def_data : defs_data) {
        def_data.second->insert_constructors();
    }
    std::cout << "insert_constructors, finished." << std::endl;

    /*
        Bind op types
    */
    // std::cout << "Bind num_op types:" << std::endl;
    type_ptr num_op_type = type_ptr(new type_arr(num_type_app, type_ptr(
            new type_arr(num_type_app, num_type_app))));
    type_scheme_ptr num_op_type_ptr = type_scheme_ptr(new type_scheme(std::move(num_op_type)));
    num_op_type_ptr->forall.push_back("Num");
    num_op_type_ptr->set_num_type();
    env->bind("+", num_op_type_ptr);
    env->bind("-", num_op_type_ptr);
    env->bind("*", num_op_type_ptr);
    env->bind("/", num_op_type_ptr);

    type_ptr num_cmp_type = type_ptr(new type_arr(num_type_app, type_ptr(
            new type_arr(num_type_app, bool_type_app))));
    type_scheme_ptr num_cmp_type_ptr = type_scheme_ptr(new type_scheme(std::move(num_cmp_type)));
    num_cmp_type_ptr->forall.push_back("Num");
    num_cmp_type_ptr->set_num_type();
    env->bind("<", num_cmp_type_ptr);
    env->bind(">", num_cmp_type_ptr);
    env->bind("<=", num_cmp_type_ptr);
    env->bind(">=", num_cmp_type_ptr);
    env->bind("==", num_cmp_type_ptr);
    env->bind("!=", num_cmp_type_ptr);

    // std::cout << "Bind int_op types:" << std::endl;
    type_ptr int_op_type = type_ptr(new type_arr(int_type_app, type_ptr(
            new type_arr(int_type_app, int_type_app))));
    env->bind("%", int_op_type);
    env->bind("|", int_op_type);
    env->bind("&", int_op_type);
    env->bind("^", int_op_type);
    env->bind("<<", int_op_type);
    env->bind(">>", int_op_type);

    // std::cout << "Bind bool_op types:" << std::endl;
    type_ptr bool_op_type = type_ptr(new type_arr(bool_type_app, type_ptr(
            new type_arr(bool_type_app, bool_type_app))));
    env->bind("||", bool_op_type);
    env->bind("&&", bool_op_type);

    // std::cout << "Bind num_uniop types:" << std::endl;
    type_ptr num_uniop_type = type_ptr(new type_arr(num_type_app, num_type_app));
    type_scheme_ptr num_uniop_type_ptr = type_scheme_ptr(new type_scheme(std::move(num_uniop_type)));
    num_uniop_type_ptr->forall.push_back("Num");
    num_uniop_type_ptr->set_num_type();
    env->bind("--", num_uniop_type_ptr);

    // std::cout << "Bind int_uniop types:" << std::endl;
    type_ptr int_uniop_type = type_ptr(new type_arr(int_type_app, int_type_app));
    env->bind("~", int_uniop_type);
    
    // std::cout << "Bind bool_uniop types:" << std::endl;
    type_ptr bool_uniop_type = type_ptr(new type_arr(bool_type_app, bool_type_app));
    env->bind("!", bool_uniop_type);

    // std::cout << "Bind base types and op types, finished." << std::endl;

    function_graph dependency_graph;

    for(auto& def_defn : defs_defn) {
        def_defn.second->find_free(mgr, env);
        dependency_graph.add_function(def_defn.second->name);

        for(auto& dependency : def_defn.second->free_variables) {
            if(defs_defn.find(dependency) == defs_defn.end()) {
                std::cout << "defs_defn cannot find dependency: " << dependency << std::endl;
                throw 0;
            }
            dependency_graph.add_edge(def_defn.second->name, dependency);
            // std::cout << "add_edge " << def_defn.second->name << " " << dependency << std::endl;
        }
    }

    std::vector<group_ptr> groups = dependency_graph.compute_order();
    std::cout << "compute_order, finished." << std::endl;
    for(auto it = groups.rbegin(); it != groups.rend(); it++) {
        auto& group = *it;
        for(auto& def_defnn_name : group->members) {
            auto& def_defn = defs_defn.find(def_defnn_name)->second;
            // std::cout << "begin insert_types " << def_defnn_name << std::endl;
            def_defn->insert_types(mgr);
            // std::cout << "finish insert_types " << def_defnn_name << std::endl;
        }
        for(auto& def_defnn_name : group->members) {
            auto& def_defn = defs_defn.find(def_defnn_name)->second;
            std::cout << "begin typecheck " << def_defnn_name << std::endl;
            def_defn->typecheck(mgr);
            std::cout << "finish typecheck " << def_defnn_name << std::endl;
        }
        for(auto& def_defnn_name : group->members) {
            // std::cout << "begin generalize " << def_defnn_name << std::endl;
            env->generalize(def_defnn_name, mgr);
            // std::cout << "finish generalize " << def_defnn_name << std::endl;
        }
    }

    // std::cout << "Type Checking Result:" << std::endl;
    for(auto& pair : env->names) {
        char fi_letter = pair.first[0];
        if (!('a' <= fi_letter && fi_letter <= 'z'
                || 'A' <= fi_letter && fi_letter <= 'Z')) continue;
        std::cout << pair.first << ": ";
        pair.second->print(mgr, std::cout);
        std::cout << std::endl;
    }
}

int main() {
    yy::parser parser;
    type_mgr mgr;
    type_env_ptr env(new type_env);

    std::cout << "Parsing begin:" << std::endl;
    parser.parse();
    std::cout << "Parsing finished." << std::endl;

    std::cout << "Type checking begin:" << std::endl;
    for(auto& def_defn : defs_defn) {
        std::cout << def_defn.second->name;
        for(auto& param : def_defn.second->params) std::cout << " " << param;
        std::cout << ":" << std::endl;
        def_defn.second->body->print(1, std::cout);
    }
    try {
        typecheck_program(defs_data, defs_defn, mgr, env);
    } catch(unification_error& err) {
        std::cout << "failed to unify types: " << std::endl;
        std::cout << "  (1) \033[34m";
        err.left->print(mgr, std::cout);
        std::cout << "\033[0m" << std::endl;
        std::cout << "  (2) \033[32m";
        err.right->print(mgr, std::cout);
        std::cout << "\033[0m" << std::endl;
    } catch(type_error& err) {
        std::cout << "failed to type check program: " << err.description << std::endl;
    }
    std::cout << "Type checking finished." << std::endl;
}
