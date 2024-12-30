#include "uniop.hpp"

std::string uniop_name(uniop op) {
    switch(op) {
        case NOT: return "!";
        case BITNOT: return "~";
        case NEGATE: return "--";  // diff minus
    }
    return "??";
}

std::string uniop_action(uniop op) {
    switch(op) {
        case NOT: return "not";
        case BITNOT: return "bitnot";
        case NEGATE: return "negate";
    }
    return "??";
}