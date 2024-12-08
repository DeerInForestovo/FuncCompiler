#include "binop.hpp"

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
    }
    return "??";
}