#include <string>
#include "env.hpp"

type_ptr type_env::lookup(const std::string& name) const {
    auto it = names.find(name);
    if(it != names.end()) return it->second;
    if(parent) return parent->lookup(name);
    return nullptr;
}

void type_env::bind(const std::string& name, type_ptr t) {
    names[name] = t;
}

type_env type_env::scope() const {
    return type_env(this);
}