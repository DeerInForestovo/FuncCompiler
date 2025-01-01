#include "prelude.hpp"

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
    Value *sign = ctx.builder.CreateICmpNE(
        isspace_call, 
        ConstantInt::get(ctx.builder.getInt32Ty(), 0)
    );

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
}
