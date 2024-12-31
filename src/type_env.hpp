#pragma once
#include <map>
#include <string>
#include "type.hpp"

struct type_env;
using type_env_ptr = std::shared_ptr<type_env>;

struct type_env {
    type_env_ptr parent;
    std::map<std::string, type_scheme_ptr> names;
    std::map<std::string, type_scheme_ptr> private_names;
    std::map<std::string, type_ptr> type_names;

    type_env(type_env_ptr p) : parent(std::move(p)) {}
    type_env() : type_env(nullptr) {}

    type_scheme_ptr lookup(const std::string& name) const;
    type_scheme_ptr lookup_private(const std::string& name) const;
    type_ptr lookup_type(const std::string& name) const;
    void bind(const std::string& name, type_ptr t);
    void bind(const std::string& name, type_scheme_ptr t);
    void bind_private(const std::string& name, type_ptr t);
    void bind_private(const std::string& name, type_scheme_ptr t);
    void bind_type(const std::string& type_name, type_ptr t);
    void generalize(const std::string& name, type_mgr& mgr);
};


type_env_ptr type_scope(type_env_ptr parent);
