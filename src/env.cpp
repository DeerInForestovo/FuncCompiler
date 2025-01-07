#include "env.hpp"
#include "error.hpp"

int env_var::get_offset(const std::string& name) const {
    if(name == this->name) return 0;
    if(parent) return parent->get_offset(name) + 1;
    throw unexpected_error("env_var::get_offset: name not found");
}

bool env_var::has_variable(const std::string& name) const {
    if(name == this->name) return true;
    if(parent) return parent->has_variable(name);
    return false;
}

int env_offset::get_offset(const std::string& name) const {
    if(parent) return parent->get_offset(name) + offset;
    throw unexpected_error("env_offset::get_offset: name not found");
}

bool env_offset::has_variable(const std::string& name) const {
    if(parent) return parent->has_variable(name);
    return false;
}
