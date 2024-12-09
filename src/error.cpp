#include "error.hpp"

const char* type_error::what() const noexcept {
    return "an error occured while checking the types of the program";
}
