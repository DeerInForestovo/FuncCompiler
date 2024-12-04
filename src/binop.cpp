#include "binop.hpp"

std::string op_name(binop op) {
    switch(op) {
        case PLUS: return "+";
        case MINUS: return "-";
        case TIMES: return "*";
        case DIVIDE: return "/";
    }
    return "??";
}

std::string op_action(binop op) {
    switch(op) {
        case PLUS: return "plus";
        case MINUS: return "minus";
        case TIMES: return "times";
        case DIVIDE: return "divide";
    }
    return "??";
}
