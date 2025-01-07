#include "error.hpp"

const char* type_error::what() const noexcept {
    return "an error occured while checking the types of the program";
}

const char* unexpected_error::what() const noexcept {
    return "an unexpected error occured";
}