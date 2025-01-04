#pragma once
#include "llvm_context.hpp"

void generate_read_llvm(llvm_context& ctx);
void generate_print_llvm(llvm_context &ctx);

void generate_charToNum_llvm(llvm_context& ctx);
void generate_numToChar_llvm(llvm_context& ctx);
void generate_floatToNum_llvm(llvm_context& ctx);
void generate_intToFloat_llvm(llvm_context& ctx);