#pragma once
#include <string>

enum binop {
    PLUS, MINUS, TIMES, DIVIDE, BMOD,
    LMOVE, RMOVE, BITAND, BITOR, AND, OR, XOR,
    LT, GT, LEQ, GEQ, EQ, NEQ,
};

std::string binop_name(binop op);