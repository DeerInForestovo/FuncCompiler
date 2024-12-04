#pragma once
#include <string>

enum binop {
    PLUS,
    MINUS,
    TIMES,
    DIVIDE
};

std::string op_name(binop op);
std::string op_action(binop op);
