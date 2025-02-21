#pragma once
#include <string>

enum binop {
    PLUS, MINUS, TIMES, DIVIDE, BMOD,
    LMOVE, RMOVE, BITAND, BITOR, AND, OR, XOR,
    LT, GT, LEQ, GEQ, EQ, NEQ,
    CONN,
};

std::string binop_name(binop op);
std::string binop_action(binop op);