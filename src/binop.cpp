#include "binop.hpp"
#include "error.hpp"

std::string binop_name(binop op) {
    switch(op) {
        case PLUS: return "+";
        case MINUS: return "-";
        case TIMES: return "*";
        case DIVIDE: return "/";
        case BMOD: return "%";
        case LMOVE: return "<<";
        case RMOVE: return ">>";
        case BITAND: return "&";
        case BITOR: return "|";
        case AND: return "&&";
        case OR: return "||";
        case XOR: return "^";
        case LT: return "<";
        case GT: return ">";
        case LEQ: return "<=";
        case GEQ: return ">=";
        case EQ: return "==";
        case NEQ: return "!=";
        case INDEX: return "_";
        case CONN: return "++";
    }
    return "??";
}

std::string binop_action(binop op) {
    switch(op) {
        case PLUS: return "plus";
        case MINUS: return "minus";
        case TIMES: return "times";
        case DIVIDE: return "divide";
        case BMOD: return "mod";
        case LMOVE: return "lshift";
        case RMOVE: return "rshift";
        case BITAND: return "bitand";
        case BITOR: return "bitor";
        case AND: return "and";
        case OR: return "or";
        case XOR: return "xor";
        case LT: return "lt";
        case GT: return "gt";
        case LEQ: return "leq";
        case GEQ: return "geq";
        case EQ: return "eq";
        case NEQ: return "neq";
        case INDEX: return "index";
        case CONN: return "concat";
    }
    return "??";
}