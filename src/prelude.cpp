#include "prelude.hpp"
#include "instruction.hpp"

using namespace llvm;

void generate_read_llvm(llvm_context &ctx) {
    Function *f = ctx.create_custom_function("read", 0);
    ctx.builder.SetInsertPoint(&f->getEntryBlock());

    FunctionType *getchar_type = FunctionType::get(ctx.builder.getInt32Ty(), false);
    FunctionCallee getchar_func = ctx.module.getOrInsertFunction("getchar", getchar_type);
    Value *getchar_call = ctx.builder.CreateCall(getchar_func, {});

    FunctionType* isspace_type = FunctionType::get(
        ctx.builder.getInt32Ty(), 
        { ctx.builder.getInt32Ty() }, 
        false
    );
    FunctionCallee isspace_func = ctx.module.getOrInsertFunction("isspace", isspace_type);
    Value *isspace_call = ctx.builder.CreateCall(isspace_func, { getchar_call });
    Value *sign = ctx.builder.CreateICmpNE(isspace_call, ConstantInt::get(ctx.builder.getInt32Ty(), 0));

    Value *ret_char = ctx.builder.CreateTrunc(getchar_call, ctx.builder.getInt8Ty());

    BasicBlock *safety_block = BasicBlock::Create(ctx.ctx, "safety", f);
    BasicBlock *true_block = BasicBlock::Create(ctx.ctx, "trueBlock", f);
    BasicBlock *false_block = BasicBlock::Create(ctx.ctx, "falseBlock", f);

    ctx.builder.CreateCondBr(sign, true_block, false_block);
    
    ctx.builder.SetInsertPoint(true_block);
    ctx.create_push(f, ctx.create_global(f, ctx.custom_functions.at("f_Nil")->function, ctx.create_i32(0)));
    ctx.builder.CreateBr(safety_block);

    ctx.builder.SetInsertPoint(false_block);
    ctx.create_push(f, ctx.create_global(f, f, ctx.create_i32(0)));
    ctx.create_unwind(f);
    ctx.create_pack(f, ctx.create_size(0), ret_char);
    Value *n_cons = ctx.create_global(f, ctx.custom_functions.at("f_Cons")->function, ctx.create_i32(2));
    Value *n_char = ctx.create_pop(f);
    Value *n_app = ctx.create_app(f, n_cons, n_char);
    Value *n_branch = ctx.create_pop(f);
    ctx.create_push(f, ctx.create_app(f, n_app, n_branch));
    ctx.builder.CreateBr(safety_block);

    ctx.builder.SetInsertPoint(safety_block);

    ctx.create_unwind(f);
    ctx.create_update(f, ctx.create_size(0));

    ctx.builder.CreateRetVoid();
}

void generate_print_llvm(llvm_context &ctx) {
    Function *f = ctx.create_custom_function("print", 1);
    ctx.builder.SetInsertPoint(&f->getEntryBlock());

    FunctionType *putchar_type = FunctionType::get(ctx.builder.getInt32Ty(), { ctx.builder.getInt32Ty() }, false);
    FunctionCallee putchar_func = ctx.module.getOrInsertFunction("putchar", putchar_type);

    ctx.create_unwind(f);

    Value *top_node = ctx.create_peek(f, ctx.create_size(0));
    Value *tag_i8 = ctx.unwrap_data_tag(top_node);
    Value *tag_i1 = ctx.builder.CreateICmpNE(tag_i8, ConstantInt::get(ctx.builder.getInt8Ty(), 0));

    BasicBlock *safety_block = BasicBlock::Create(ctx.ctx, "safety", f);
    BasicBlock *cons_block = BasicBlock::Create(ctx.ctx, "consBlock", f);
    BasicBlock *nil_block = BasicBlock::Create(ctx.ctx, "nilBlock", f);

    ctx.builder.CreateCondBr(tag_i1, cons_block, nil_block);

    ctx.builder.SetInsertPoint(cons_block);
    ctx.create_split(f, ctx.create_size(2));
    ctx.create_unwind(f);

    Value *top_char_node = ctx.create_pop(f);
    Value *char_val_i8 = ctx.unwrap_data_tag(top_char_node);
    Value *char_val_i32 = ctx.builder.CreateZExt(char_val_i8, ctx.builder.getInt32Ty());
    ctx.builder.CreateCall(putchar_func, { char_val_i32 });

    Value *recur_print = ctx.create_global(f, f, ctx.create_i32(1));
    Value *n_xs = ctx.create_pop(f);
    ctx.create_push(f, ctx.create_app(f, recur_print, n_xs));
    ctx.create_unwind(f);

    ctx.builder.CreateBr(safety_block);

    ctx.builder.SetInsertPoint(nil_block);

    Value *newline_val = ctx.builder.getInt32('\n');
    ctx.builder.CreateCall(putchar_func, { newline_val });
    
    ctx.builder.CreateBr(safety_block);

    ctx.builder.SetInsertPoint(safety_block);

    ctx.create_update(f, ctx.create_size(0));

    ctx.builder.CreateRetVoid();
}

void generate_charToNum_llvm(llvm_context& ctx) {
    auto f = ctx.create_custom_function("charToNum", 1);
    ctx.builder.SetInsertPoint(&f->getEntryBlock());

    (new instruction_push(0))->gen_llvm(ctx, f);
    (new instruction_eval())->gen_llvm(ctx, f);
    ctx.create_push(f, ctx.create_num(f, ctx.builder.CreateZExt(  // i8 -> i32
            ctx.unwrap_data_tag(ctx.create_pop(f)), llvm::Type::getInt32Ty(ctx.ctx))));
    (new instruction_update(1))->gen_llvm(ctx, f);
    (new instruction_pop(1))->gen_llvm(ctx, f);

    ctx.builder.CreateRetVoid();
}

void generate_numToChar_llvm(llvm_context& ctx) {
    auto f = ctx.create_custom_function("numToChar", 1);
    ctx.builder.SetInsertPoint(&f->getEntryBlock());

    (new instruction_push(0))->gen_llvm(ctx, f);
    (new instruction_eval())->gen_llvm(ctx, f);
    ctx.create_pack(f, ctx.create_size(0), ctx.builder.CreateTrunc(  // i32 -> i8
            ctx.unwrap_num(ctx.create_pop(f)), llvm::Type::getInt8Ty(ctx.ctx)));
    (new instruction_update(1))->gen_llvm(ctx, f);
    (new instruction_pop(1))->gen_llvm(ctx, f);

    ctx.builder.CreateRetVoid();
}

void generate_floatToNum_llvm(llvm_context& ctx) {
    auto f = ctx.create_custom_function("floatToNum", 1);
    ctx.builder.SetInsertPoint(&f->getEntryBlock());

    (new instruction_push(0))->gen_llvm(ctx, f);
    (new instruction_eval())->gen_llvm(ctx, f);
    ctx.create_push(f, ctx.create_num(f, ctx.builder.CreateFPToSI(
            ctx.unwrap_float(ctx.create_pop(f)), llvm::Type::getInt32Ty(ctx.ctx))));
    (new instruction_update(1))->gen_llvm(ctx, f);
    (new instruction_pop(1))->gen_llvm(ctx, f);

    ctx.builder.CreateRetVoid();
}