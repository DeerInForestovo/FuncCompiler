#pragma once
#include <string>

enum binop {
    PLUS, MINUS, TIMES, DIVIDE, BMOD,
    LMOVE, RMOVE, BITAND, BITOR, AND, OR, XOR,
    LT, GT, LEQ, GEQ, EQ, NEQ,
};

std::string op_name(binop op);
std::string op_action(binop op);
