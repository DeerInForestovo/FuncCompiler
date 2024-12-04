#pragma once
#include <map>
#include "type.hpp"

struct type_env {
    std::map<std::string, type_ptr> names;
    type_env const* parent = nullptr;

    type_env(type_env const* p)
        : parent(p) {}
    type_env() : type_env(nullptr) {}

    type_ptr lookup(const std::string& name) const;
    void bind(const std::string& name, type_ptr t);
    type_env scope() const;
};