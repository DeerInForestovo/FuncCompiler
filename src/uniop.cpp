#include "uniop.hpp"

std::string op_name(uniop op) {
    switch(op) {
        case NOT: return "!";
        case BITNOT: return "~";
        case NEGATE: return "-";
    }
    return "??";
}

std::string op_action(uniop op) {
    switch(op) {
        case NOT: return "not";
        case BITNOT: return "bitnot";
        case NEGATE: return "negate";
    }
    return "??";
}
