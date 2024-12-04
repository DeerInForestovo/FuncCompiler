#pragma once
#include <string>

enum uniop {
    NOT, BITNOT, NEGATE
};

std::string op_name(uniop op);
std::string op_action(uniop op);
