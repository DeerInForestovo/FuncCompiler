#pragma once
#include <memory>
#include <vector>
#include <set>
#include "parsed_type.hpp"
#include "type_env.hpp"

struct ast;
using ast_ptr = std::unique_ptr<ast>;

struct constructor {
    std::string name;
    std::vector<parsed_type_ptr> types;
    int8_t tag;

    constructor(std::string n, std::vector<parsed_type_ptr> ts)
        : name(std::move(n)), types(std::move(ts)) {}
};

using constructor_ptr = std::unique_ptr<constructor>;

struct definition_defn {
    std::string name;
    std::vector<std::string> params;
    ast_ptr body;

    type_env_ptr env;
    type_env_ptr var_env;
    std::set<std::string> free_variables;
    type_ptr full_type;
    type_ptr return_type;

    definition_defn(std::string n, std::vector<std::string> p, ast_ptr b)
        : name(std::move(n)), params(std::move(p)), body(std::move(b)) {

    }

    void find_free(type_mgr& mgr, type_env_ptr& env);
    void insert_types(type_mgr& mgr);
    void typecheck(type_mgr& mgr);
    void compile();
};

using definition_defn_ptr = std::unique_ptr<definition_defn>;

struct definition_data {
    std::string name;
    std::vector<std::string> vars;
    std::vector<constructor_ptr> constructors;

    type_env_ptr env;

    definition_data(
            std::string n,
            std::vector<std::string> vs,
            std::vector<constructor_ptr> cs)
        : name(std::move(n)), vars(std::move(vs)), constructors(std::move(cs)) {}

    void insert_types(type_env_ptr& env);
    void insert_constructors() const;
};

using definition_data_ptr = std::unique_ptr<definition_data>;
