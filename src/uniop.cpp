#include "uniop.hpp"

std::string uniop_name(uniop op) {
    switch(op) {
        case NOT: return "!";
        case BITNOT: return "~";
        case NEGATE: return "--";  // diff minus
    }
    return "??";
}