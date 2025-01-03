#pragma once
#include <llvm/IR/Function.h>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <ostream>
#include "binop.hpp"
#include "llvm_context.hpp"

struct instruction {
    virtual ~instruction() = default;

    virtual void print(int indent, std::ostream& to) const = 0;
    virtual void gen_llvm(llvm_context& ctx, llvm::Function* f) const = 0;
};

using instruction_ptr = std::unique_ptr<instruction>;

struct instruction_pushint : public instruction {
    int value;

    instruction_pushint(int v)
        : value(v) {}

    void print(int indent, std::ostream& to) const;
    void gen_llvm(llvm_context& ctx, llvm::Function* f) const;
};

struct instruction_pushfloat : public instruction {
    float value;

    instruction_pushfloat(float v)
        : value(v) {}

    void print(int indent, std::ostream& to) const;
    void gen_llvm(llvm_context& ctx, llvm::Function* f) const;
};

struct instruction_pushchar : public instruction {
    char value;

    instruction_pushchar(char v)
        : value(v) {}

    void print(int indent, std::ostream& to) const;
    void gen_llvm(llvm_context& ctx, llvm::Function* f) const;
};

struct instruction_pushglobal : public instruction {
    std::string name;

    instruction_pushglobal(std::string n)
        : name(std::move(n)) {}

    void print(int indent, std::ostream& to) const;
    void gen_llvm(llvm_context& ctx, llvm::Function* f) const;
};

struct instruction_push : public instruction {
    int offset;

    instruction_push(int o)
        : offset(o) {}

    void print(int indent, std::ostream& to) const;
    void gen_llvm(llvm_context& ctx, llvm::Function* f) const;
};

struct instruction_pop : public instruction {
    int count;

    instruction_pop(int c)
        : count(c) {}

    void print(int indent, std::ostream& to) const;
    void gen_llvm(llvm_context& ctx, llvm::Function* f) const;
};

struct instruction_mkapp : public instruction {
    void print(int indent, std::ostream& to) const;
    void gen_llvm(llvm_context& ctx, llvm::Function* f) const;
};

struct instruction_update : public instruction {
    int offset;

    instruction_update(int o)
        : offset(o) {}

    void print(int indent, std::ostream& to) const;
    void gen_llvm(llvm_context& ctx, llvm::Function* f) const;
};

struct instruction_pack : public instruction {
    int tag;
    int size;

    instruction_pack(int t, int s)
        : tag(t), size(s) {}

    void print(int indent, std::ostream& to) const;
    void gen_llvm(llvm_context& ctx, llvm::Function* f) const;
};

struct instruction_split : public instruction {
    int size;

    instruction_split(int s)
        : size(s) {}

    void print(int indent, std::ostream& to) const;
    void gen_llvm(llvm_context& ctx, llvm::Function* f) const;
};

struct instruction_jump : public instruction {
    std::vector<std::vector<instruction_ptr>> branches;
    std::map<int, int> tag_mappings;

    void print(int indent, std::ostream& to) const;
    void gen_llvm(llvm_context& ctx, llvm::Function* f) const;
};

struct instruction_slide : public instruction {
    int offset;

    instruction_slide(int o)
        : offset(o) {}

    void print(int indent, std::ostream& to) const;
    void gen_llvm(llvm_context& ctx, llvm::Function* f) const;
};

struct instruction_binop : public instruction {
    binop op;

    instruction_binop(binop o)
        : op(o) {}

    void print(int indent, std::ostream& to) const;
    void gen_llvm(llvm_context& ctx, llvm::Function* f) const;
};

struct instruction_eval : public instruction {
    void print(int indent, std::ostream& to) const;
    void gen_llvm(llvm_context& ctx, llvm::Function* f) const;
};

struct instruction_alloc : public instruction {
    int amount;

    instruction_alloc(int a)
        : amount(a) {}

    void print(int indent, std::ostream& to) const;
    void gen_llvm(llvm_context& ctx, llvm::Function* f) const;
};

struct instruction_unwind : public instruction {
    void print(int indent, std::ostream& to) const;
    void gen_llvm(llvm_context& ctx, llvm::Function* f) const;
};