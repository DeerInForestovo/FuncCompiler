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

void instruction_pushfloat::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "PushFloat(" << value << ")" << std::endl;
}

void instruction_pushfloat::gen_llvm(llvm_context& ctx, Function* f) const {
    ctx.create_push(f, ctx.create_float(f, ctx.create_f32(value)));
}

void instruction_pushchar::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "PushChar(" << value << ")" << std::endl;
}

void instruction_pushchar::gen_llvm(llvm_context& ctx, Function* f) const {
    ctx.create_pack(f, ctx.create_size(0), ctx.create_i8(value));
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
    if (op == CONN) {
        auto left_list = ctx.create_peek(f, ctx.create_size(0));
        auto tag_i8 = ctx.unwrap_data_tag(left_list);
        auto tag_i1 = ctx.builder.CreateICmpNE(tag_i8, ConstantInt::get(ctx.builder.getInt8Ty(), 0));

        auto safety_block = BasicBlock::Create(ctx.ctx, "safety", f);
        auto cons_block = BasicBlock::Create(ctx.ctx, "consBlock", f);
        auto nil_block = BasicBlock::Create(ctx.ctx, "nilBlock", f);

        ctx.builder.CreateCondBr(tag_i1, cons_block, nil_block);

        ctx.builder.SetInsertPoint(cons_block);
        ctx.create_split(f, ctx.create_size(2));
        ctx.create_disablegc(f);
        auto n_x = ctx.create_pop(f);
        auto recur_conn = ctx.create_global(f, f, ctx.create_i32(2));
        auto n_xs = ctx.create_pop(f);
        auto n_app_conn_xs = ctx.create_app(f, recur_conn, n_xs);
        auto right_list_cons = ctx.create_pop(f);
        auto n_app_conn = ctx.create_app(f, n_app_conn_xs, right_list_cons);
        auto n_cons = ctx.create_global(f, ctx.custom_functions.at("f_Cons")->function, ctx.create_i32(2));
        auto n_app_cons = ctx.create_app(f, n_cons, n_x);
        ctx.create_enablegc(f);
        ctx.create_push(f, ctx.create_app(f, n_app_cons, n_app_conn)); // gc issue?
        ctx.builder.CreateBr(safety_block);

        ctx.builder.SetInsertPoint(nil_block);
        ctx.create_popn(f, ctx.create_size(1));
        ctx.builder.CreateBr(safety_block);

        ctx.builder.SetInsertPoint(safety_block);

        return;
    }
    if (op == AND || op == OR) {
        auto left_bool_value = ctx.unwrap_data_tag(ctx.create_pop(f));
        auto right_bool_value = ctx.unwrap_data_tag(ctx.create_pop(f));
        llvm::Value* result;
        if (op == AND) {
            result = ctx.builder.CreateAnd(left_bool_value, right_bool_value);
        } else {  // OR
            result = ctx.builder.CreateOr(left_bool_value, right_bool_value);
        }
        ctx.create_pack(f, ctx.create_size(0),  // See comments below
                    ctx.builder.CreateSelect(ctx.builder.CreateICmpNE(result, ctx.create_i8(0)),  // result i8/i32 -> i1
                            ctx.create_i8(1), ctx.create_i8(0)));
    } else if (op == PLUS || op == MINUS || op == TIMES || op == DIVIDE) {
        auto left_value = ctx.create_pop(f);
        auto right_value = ctx.create_pop(f);
        auto left_tag = ctx.get_node_tag(left_value);
        auto right_tag = ctx.get_node_tag(right_value);

        auto is_left_float = ctx.builder.CreateICmpEQ(left_tag, ctx.create_i32(2));  // (enum) Tag == 2 -> float
        auto is_right_float = ctx.builder.CreateICmpEQ(right_tag, ctx.create_i32(2));
        auto is_any_float = ctx.builder.CreateOr(is_left_float, is_right_float);

        auto left_num = ctx.unwrap_num(left_value);
        auto right_num = ctx.unwrap_num(right_value);
        auto left_float = ctx.unwrap_float(left_value);
        auto right_float = ctx.unwrap_float(right_value);

        auto left_as_float = ctx.builder.CreateSelect(is_left_float,
                left_float, ctx.builder.CreateSIToFP(left_num, llvm::Type::getFloatTy(ctx.ctx)));
        auto right_as_float = ctx.builder.CreateSelect(is_right_float,
                right_float, ctx.builder.CreateSIToFP(right_num, llvm::Type::getFloatTy(ctx.ctx)));

        llvm::Value* num_result;
        llvm::Value* float_result;
        switch (op) {
            case PLUS:
                float_result = ctx.builder.CreateFAdd(left_as_float, right_as_float);
                num_result = ctx.builder.CreateAdd(left_num, right_num);
                break;
            case MINUS:
                float_result = ctx.builder.CreateFSub(left_as_float, right_as_float);
                num_result = ctx.builder.CreateSub(left_num, right_num);
                break;
            case TIMES:
                float_result = ctx.builder.CreateFMul(left_as_float, right_as_float);
                num_result = ctx.builder.CreateMul(left_num, right_num);
                break;
            case DIVIDE:
                float_result = ctx.builder.CreateFDiv(left_as_float, right_as_float);
                num_result = ctx.builder.CreateSDiv(left_num, right_num);
                break;
        }

        ctx.create_push(f, ctx.builder.CreateSelect(is_any_float,
                ctx.create_float(f, float_result), ctx.create_num(f, num_result)));
    } else {
        auto left_int = ctx.unwrap_num(ctx.create_pop(f));
        auto right_int = ctx.unwrap_num(ctx.create_pop(f));
        llvm::Value* result;
        switch(op) {
            case BMOD: result = ctx.builder.CreateSRem(left_int, right_int); break;
            case LMOVE: result = ctx.builder.CreateShl(left_int, right_int); break;
            case RMOVE: result = ctx.builder.CreateAShr(left_int, right_int); break;
            case BITAND: result = ctx.builder.CreateAnd(left_int, right_int); break;
            case BITOR: result = ctx.builder.CreateOr(left_int, right_int); break;
            case XOR: result = ctx.builder.CreateXor(left_int, right_int); break;
            case LT: result = ctx.builder.CreateICmpSLT(left_int, right_int); break;
            case GT: result = ctx.builder.CreateICmpSGT(left_int, right_int); break;
            case LEQ: result = ctx.builder.CreateICmpSLE(left_int, right_int); break;
            case GEQ: result = ctx.builder.CreateICmpSGE(left_int, right_int); break;
            case EQ: result = ctx.builder.CreateICmpEQ(left_int, right_int); break;
            case NEQ: result = ctx.builder.CreateICmpNE(left_int, right_int); break;
        }
        if (op == LT || op == GT || op == LEQ || op == GEQ || op == EQ || op == NEQ) {
            // For (num -> (num -> (Bool*))) operations, we need to simulate a Data constructor here.
            // See instruction.cpp - void instruction_pack::gen_llvm and definition.cpp - void definition_data::generate_llvm.
            ctx.create_pack(f, ctx.create_size(0),  // The constructor takes 0 elements in the stack (or, arity = 0).
                    ctx.builder.CreateSelect(result, ctx.create_i8(1), ctx.create_i8(0)));  // The constructor-tag is 1 (True) or 0 (False), depanded on result.
        } else {
            ctx.create_push(f, ctx.create_num(f, result));
        }
    }
}

void instruction_uniop::print(int indent, std::ostream& to) const {
    print_indent(indent, to);
    to << "UniOp(" << uniop_name(op) << ")" << std::endl;
}

void instruction_uniop::gen_llvm(llvm_context& ctx, Function* f) const {
    if (op == NOT) {
        auto bool_value = ctx.unwrap_data_tag(ctx.create_pop(f));  // i8
        auto result = ctx.builder.CreateICmpEQ(bool_value, ctx.create_i8(0));  // !a <--> (a == False) for (bool)a
        ctx.create_pack(f, ctx.create_size(0),
                ctx.builder.CreateSelect(result, ctx.create_i8(1), ctx.create_i8(0)));
    } else if (op == BITNOT) {
        auto int_value = ctx.unwrap_num(ctx.create_pop(f));
        auto result = ctx.builder.CreateNot(int_value);
        ctx.create_push(f, ctx.create_num(f, result));
    } else {  // op == NEGATE
        auto value = ctx.create_pop(f);
        ctx.create_push(f, ctx.builder.CreateSelect(
                ctx.builder.CreateICmpEQ(ctx.get_node_tag(value), ctx.create_i32(2)),  // is_float
                        ctx.create_float(f, ctx.builder.CreateFNeg(ctx.unwrap_float(value))),
                        ctx.create_num(f, ctx.builder.CreateNeg(ctx.unwrap_num(value)))));
    }
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