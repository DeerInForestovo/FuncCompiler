#pragma once
#include <memory>
#include <set>
#include <string>
#include "type_env.hpp"

struct parsed_type {
    virtual type_ptr to_type(
            const std::set<std::string>& vars,
            const type_env& env) const = 0;
};

using parsed_type_ptr = std::unique_ptr<parsed_type>;

struct parsed_type_app : parsed_type {
    std::string name;
    std::vector<parsed_type_ptr> arguments;

    parsed_type_app(
            std::string n,
            std::vector<parsed_type_ptr> as)
        : name(std::move(n)), arguments(std::move(as)) {}

    type_ptr to_type(const std::set<std::string>& vars, const type_env& env) const;
};

struct parsed_type_var : parsed_type {
    std::string var;

    parsed_type_var(std::string v) : var(std::move(v)) {}

    type_ptr to_type(const std::set<std::string>& vars, const type_env& env) const;
};

struct parsed_type_arr : parsed_type {
    parsed_type_ptr left;
    parsed_type_ptr right;

    parsed_type_arr(parsed_type_ptr l, parsed_type_ptr r)
        : left(std::move(l)), right(std::move(r)) {}

    type_ptr to_type(const std::set<std::string>& vars, const type_env& env) const;
};
