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
    type_ptr int_type = type_ptr(new type_base("Int")); 
    env->bind_type("Int", int_type);
    type_ptr int_type_app = type_ptr(new type_app(int_type));

    type_ptr binop_type = type_ptr(new type_arr(
                int_type_app,
                type_ptr(new type_arr(int_type_app, int_type_app))));
    env->bind("+", binop_type);
    env->bind("-", binop_type);
    env->bind("*", binop_type);
    env->bind("/", binop_type);

    for(auto& def_data : defs_data) {
        def_data.second->insert_types(env);
    }
    for(auto& def_data : defs_data) {
        def_data.second->insert_constructors();
    }

    function_graph dependency_graph;

    for(auto& def_defn : defs_defn) {
        def_defn.second->find_free(mgr, env);
        dependency_graph.add_function(def_defn.second->name);

        for(auto& dependency : def_defn.second->free_variables) {
            if(defs_defn.find(dependency) == defs_defn.end())
                throw 0;
            dependency_graph.add_edge(def_defn.second->name, dependency);
        }
    }

    std::vector<group_ptr> groups = dependency_graph.compute_order();
    for(auto it = groups.rbegin(); it != groups.rend(); it++) {
        auto& group = *it;
        for(auto& def_defnn_name : group->members) {
            auto& def_defn = defs_defn.find(def_defnn_name)->second;
            def_defn->insert_types(mgr);
        }
        for(auto& def_defnn_name : group->members) {
            auto& def_defn = defs_defn.find(def_defnn_name)->second;
            def_defn->typecheck(mgr);
        }
        for(auto& def_defnn_name : group->members) {
            env->generalize(def_defnn_name, mgr);
        }
    }

    for(auto& pair : env->names) {
        std::cout << pair.first << ": ";
        pair.second->print(mgr, std::cout);
        std::cout << std::endl;
    }
}

int main() {
    yy::parser parser;
    type_mgr mgr;
    type_env_ptr env(new type_env);

    parser.parse();
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
}
