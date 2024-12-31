#include "type_env.hpp"
#include "type.hpp"
#include "error.hpp"
#include <iostream>

type_scheme_ptr type_env::lookup(const std::string& name) const {
    auto it = names.find(name);
    if(it != names.end()) return it->second;
    if(parent) return parent->lookup(name);
    return nullptr;
}

type_ptr type_env::lookup_type(const std::string& name) const {
    auto it = type_names.find(name);
    if(it != type_names.end()) return it->second;
    if(parent) return parent->lookup_type(name);
    return nullptr;
}

void type_env::bind(const std::string& name, type_ptr t) {
    names[name] = type_scheme_ptr(new type_scheme(t));
}

void type_env::bind(const std::string& name, type_scheme_ptr t) {
    names[name] = t;
}

void type_env::bind_type(const std::string& type_name, type_ptr t) {
    if(lookup_type(type_name) != nullptr)
        throw type_error("Duplicate definition of " + type_name);
    type_names[type_name] = t;
}

void type_env::generalize(const std::string& name, type_mgr& mgr) {
    auto names_it = names.find(name);
    if(names_it == names.end()) throw 0;
    if(names_it->second->forall.size() > 0) throw 0;

    std::set<std::pair<std::string, bool>> free_variables;
    std::vector<type_ptr> ptr_stack;
    mgr.find_free(names_it->second->monotype, free_variables, ptr_stack);
    for(auto& free : free_variables) {
        names_it->second->forall.push_back(free);
    }
}

type_env_ptr type_scope(type_env_ptr parent) {
    return type_env_ptr(new type_env(std::move(parent)));
}
