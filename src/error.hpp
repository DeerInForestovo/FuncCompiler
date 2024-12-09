#pragma once
#include <exception>
#include "type.hpp"

struct type_error : std::exception {
    std::string description;

    type_error(std::string d)
        : description(std::move(d)) {}

    const char* what() const noexcept override;
};

struct unification_error : public type_error {
    type_ptr left;
    type_ptr right;

    unification_error(type_ptr l, type_ptr r)
        : left(std::move(l)), right(std::move(r)), 
            type_error("failed to unify types") {}
};
