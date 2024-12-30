#include "instruction.hpp"
#include "llvm_context.hpp"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>

using namespace llvm;

static void print_indent(int n, std::ostream& to) {
    while(n--) to << "  ";
}

void instruction_pushint::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "PushInt(" << value << ")" << std::endl;
}

void instruction_pushint::gen_llvm(llvm_context& ctx, Function* f) const {
    ctx.create_push(f, ctx.create_num(f, ctx.create_i32(value)));
}

void instruction_pushglobal::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "PushGlobal(" << name << ")" << std::endl;
}

void instruction_pushglobal::gen_llvm(llvm_context& ctx, Function* f) const {
    try {
        auto& global_f = ctx.custom_functions.at("f_" + name);
        auto arity = ctx.create_i32(global_f->arity);
        ctx.create_push(f, ctx.create_global(f, global_f->function, arity));
    } catch (std::out_of_range& err) {
        // This is only used during development: some functions/operations have not been implemented yet.
        // Remove this try-catch if all funcs/ops are ready.
    }
}

void instruction_push::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "Push(" << offset << ")" << std::endl;
}

void instruction_push::gen_llvm(llvm_context& ctx, Function* f) const {
    ctx.create_push(f, ctx.create_peek(f, ctx.create_size(offset)));
}

void instruction_pop::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "Pop(" << count << ")" << std::endl;
}

void instruction_pop::gen_llvm(llvm_context& ctx, Function* f) const {
    ctx.create_popn(f, ctx.create_size(count));
}

void instruction_mkapp::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "MkApp()" << std::endl;
}

void instruction_mkapp::gen_llvm(llvm_context& ctx, Function* f) const {
    auto left = ctx.create_pop(f);
    auto right = ctx.create_pop(f);
    ctx.create_push(f, ctx.create_app(f, left, right));
}

void instruction_update::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "Update(" << offset << ")" << std::endl;
}

void instruction_update::gen_llvm(llvm_context& ctx, Function* f) const {
    ctx.create_update(f, ctx.create_size(offset));
}

void instruction_pack::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "Pack(" << tag << ", " << size << ")" << std::endl;
}

void instruction_pack::gen_llvm(llvm_context& ctx, Function* f) const {
    ctx.create_pack(f, ctx.create_size(size), ctx.create_i8(tag));
}

void instruction_split::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "Split()" << std::endl;
}

void instruction_split::gen_llvm(llvm_context& ctx, Function* f) const {
    ctx.create_split(f, ctx.create_size(size));
}

void instruction_jump::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "Jump(" << std::endl;
    for(auto& instruction_set : branches) {
        for(auto& instruction : instruction_set) {
            instruction->print(indent + 2, to);
        }
        to << std::endl;
    }
    print_indent(indent, to);
    to << ")" << std::endl;
}

void instruction_jump::gen_llvm(llvm_context& ctx, Function* f) const {
    auto top_node = ctx.create_peek(f, ctx.create_size(0));
    auto tag = ctx.unwrap_data_tag(top_node);
    auto safety_block = BasicBlock::Create(ctx.ctx, "safety", f);
    auto switch_op = ctx.builder.CreateSwitch(tag, safety_block, tag_mappings.size());
    std::vector<BasicBlock*> blocks;

    for(auto& branch : branches) {
        auto branch_block = BasicBlock::Create(ctx.ctx, "branch", f);
        ctx.builder.SetInsertPoint(branch_block);
        for(auto& instruction : branch) {
            instruction->gen_llvm(ctx, f);
        }
        ctx.builder.CreateBr(safety_block);
        blocks.push_back(branch_block);
    }

    for(auto& mapping : tag_mappings) {
        switch_op->addCase(ctx.create_i8(mapping.first), blocks[mapping.second]);
    }

    ctx.builder.SetInsertPoint(safety_block);
}

void instruction_slide::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "Slide(" << offset << ")" << std::endl;
}

void instruction_slide::gen_llvm(llvm_context& ctx, Function* f) const {
    ctx.create_slide(f, ctx.create_size(offset));
}

void instruction_binop::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "BinOp(" << binop_name(op) << ")" << std::endl;
}

void instruction_binop::gen_llvm(llvm_context& ctx, Function* f) const {
    auto left_int = ctx.unwrap_num(ctx.create_pop(f));
    auto right_int = ctx.unwrap_num(ctx.create_pop(f));
    llvm::Value* result;
    switch(op) {
        case PLUS: result = ctx.builder.CreateAdd(left_int, right_int); break;
        case MINUS: result = ctx.builder.CreateSub(left_int, right_int); break;
        case TIMES: result = ctx.builder.CreateMul(left_int, right_int); break;
        case DIVIDE: result = ctx.builder.CreateSDiv(left_int, right_int); break;
    }
    ctx.create_push(f, ctx.create_num(f, result));
}

void instruction_eval::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "Eval()" << std::endl;
}

void instruction_eval::gen_llvm(llvm_context& ctx, Function* f) const {
    ctx.create_unwind(f);
}

void instruction_alloc::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "Alloc(" << amount << ")" << std::endl;
}

void instruction_alloc::gen_llvm(llvm_context& ctx, Function* f) const {
    ctx.create_alloc(f, ctx.create_size(amount));
}

void instruction_unwind::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "Unwind()" << std::endl;
}

void instruction_unwind::gen_llvm(llvm_context& ctx, Function* f) const {
    // Nothing
}
