#pragma once
#include <string>

enum uniop {
    NOT, BITNOT, NEGATE
};

std::string uniop_name(uniop op);