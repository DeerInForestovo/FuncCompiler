#include "llvm_context.hpp"
#include <llvm/IR/DerivedTypes.h>

using namespace llvm;

void llvm_context::create_types() {
    stack_type = StructType::create(ctx, "stack");
    gmachine_type = StructType::create(ctx, "gmachine");
    stack_ptr_type = PointerType::getUnqual(stack_type);
    gmachine_ptr_type = PointerType::getUnqual(gmachine_type);
    tag_type = IntegerType::getInt8Ty(ctx);
    struct_types["node_base"] = StructType::create(ctx, "node_base");
    struct_types["node_app"] = StructType::create(ctx, "node_app");
    struct_types["node_num"] = StructType::create(ctx, "node_num");
    struct_types["node_float"] = StructType::create(ctx, "node_float");
    struct_types["node_global"] = StructType::create(ctx, "node_global");
    struct_types["node_ind"] = StructType::create(ctx, "node_ind");
    struct_types["node_data"] = StructType::create(ctx, "node_data");
    node_ptr_type = PointerType::getUnqual(struct_types.at("node_base"));
    function_type = FunctionType::get(Type::getVoidTy(ctx), { gmachine_ptr_type }, false);

    gmachine_type->setBody(
            stack_ptr_type,
            node_ptr_type,
            IntegerType::getInt64Ty(ctx),
            IntegerType::getInt64Ty(ctx)
    );
    struct_types.at("node_base")->setBody(
            IntegerType::getInt32Ty(ctx),
            IntegerType::getInt8Ty(ctx),
            node_ptr_type
    );
    struct_types.at("node_app")->setBody(
            struct_types.at("node_base"),
            node_ptr_type,
            node_ptr_type
    );
    struct_types.at("node_num")->setBody(
            struct_types.at("node_base"),
            IntegerType::getInt32Ty(ctx)
    );
    struct_types.at("node_float")->setBody(
            struct_types.at("node_base"),
            Type::getFloatTy(ctx)
    );
    struct_types.at("node_global")->setBody(
            struct_types.at("node_base"),
            FunctionType::get(Type::getVoidTy(ctx), { stack_ptr_type }, false)
    );
    struct_types.at("node_ind")->setBody(
            struct_types.at("node_base"),
            node_ptr_type
    );
    struct_types.at("node_data")->setBody(
            struct_types.at("node_base"),
            IntegerType::getInt8Ty(ctx),
            PointerType::getUnqual(node_ptr_type)
    );
}

void llvm_context::create_functions() {
    auto void_type = Type::getVoidTy(ctx);
    auto sizet_type = IntegerType::get(ctx, sizeof(size_t) * 8);
    functions["stack_init"] = Function::Create(
            FunctionType::get(void_type, { stack_ptr_type }, false),
            Function::LinkageTypes::ExternalLinkage,
            "stack_init",
            &module
    );
    functions["stack_free"] = Function::Create(
            FunctionType::get(void_type, { stack_ptr_type }, false),
            Function::LinkageTypes::ExternalLinkage,
            "stack_free",
            &module
    );
    functions["stack_push"] = Function::Create(
            FunctionType::get(void_type, { stack_ptr_type, node_ptr_type }, false),
            Function::LinkageTypes::ExternalLinkage,
            "stack_push",
            &module
    );
    functions["stack_pop"] = Function::Create(
            FunctionType::get(node_ptr_type, { stack_ptr_type }, false),
            Function::LinkageTypes::ExternalLinkage,
            "stack_pop",
            &module
    );
    functions["stack_peek"] = Function::Create(
            FunctionType::get(node_ptr_type, { stack_ptr_type, sizet_type }, false),
            Function::LinkageTypes::ExternalLinkage,
            "stack_peek",
            &module
    );
    functions["stack_popn"] = Function::Create(
            FunctionType::get(void_type, { stack_ptr_type, sizet_type }, false),
            Function::LinkageTypes::ExternalLinkage,
            "stack_popn",
            &module
    );
    functions["gmachine_slide"] = Function::Create(
            FunctionType::get(void_type, { gmachine_ptr_type, sizet_type }, false),
            Function::LinkageTypes::ExternalLinkage,
            "gmachine_slide",
            &module
    );
    functions["gmachine_update"] = Function::Create(
            FunctionType::get(void_type, { gmachine_ptr_type, sizet_type }, false),
            Function::LinkageTypes::ExternalLinkage,
            "gmachine_update",
            &module
    );
    functions["gmachine_alloc"] = Function::Create(
            FunctionType::get(void_type, { gmachine_ptr_type, sizet_type }, false),
            Function::LinkageTypes::ExternalLinkage,
            "gmachine_alloc",
            &module
    );
    functions["gmachine_pack"] = Function::Create(
            FunctionType::get(void_type, { gmachine_ptr_type, sizet_type, tag_type }, false),
            Function::LinkageTypes::ExternalLinkage,
            "gmachine_pack",
            &module
    );
    functions["gmachine_split"] = Function::Create(
            FunctionType::get(void_type, { gmachine_ptr_type, sizet_type }, false),
            Function::LinkageTypes::ExternalLinkage,
            "gmachine_split",
            &module
    );
    functions["gmachine_enablegc"] = Function::Create(
            FunctionType::get(node_ptr_type, { gmachine_ptr_type }, false),
            Function::LinkageTypes::ExternalLinkage,
            "gmachine_enablegc",
            &module
    );
    functions["gmachine_disablegc"] = Function::Create(
            FunctionType::get(node_ptr_type, { gmachine_ptr_type }, false),
            Function::LinkageTypes::ExternalLinkage,
            "gmachine_disablegc",
            &module
    );
    functions["gmachine_track"] = Function::Create(
            FunctionType::get(node_ptr_type, { gmachine_ptr_type, node_ptr_type }, false),
            Function::LinkageTypes::ExternalLinkage,
            "gmachine_track",
            &module
    );

    auto int32_type = IntegerType::getInt32Ty(ctx);
    auto float32_type = llvm::Type::getFloatTy(ctx);
    functions["alloc_app"] = Function::Create(
            FunctionType::get(node_ptr_type, { node_ptr_type, node_ptr_type }, false),
            Function::LinkageTypes::ExternalLinkage,
            "alloc_app",
            &module
    );
    functions["alloc_num"] = Function::Create(
            FunctionType::get(node_ptr_type, { int32_type }, false),
            Function::LinkageTypes::ExternalLinkage,
            "alloc_num",
            &module
    );
    functions["alloc_float"] = Function::Create(
            FunctionType::get(node_ptr_type, { float32_type }, false),
            Function::LinkageTypes::ExternalLinkage,
            "alloc_float",
            &module
    );
    functions["alloc_data"] = Function::Create(
            FunctionType::get(node_ptr_type, { IntegerType::getInt8Ty(ctx), IntegerType::get(ctx, sizeof(size_t) * 8) }, false),
            Function::LinkageTypes::ExternalLinkage,
            "alloc_data",
            &module
    );
    functions["alloc_global"] = Function::Create(
            FunctionType::get(node_ptr_type, { function_type, int32_type }, false),
            Function::LinkageTypes::ExternalLinkage,
            "alloc_global",
            &module
    );
    functions["alloc_ind"] = Function::Create(
            FunctionType::get(node_ptr_type, { node_ptr_type }, false),
            Function::LinkageTypes::ExternalLinkage,
            "alloc_ind",
            &module
    );

    functions["unwind"] = Function::Create(
            FunctionType::get(void_type, { gmachine_ptr_type }, false),
            Function::LinkageTypes::ExternalLinkage,
            "unwind",
            &module
    );
}

ConstantInt* llvm_context::create_i8(int8_t i) {
    return ConstantInt::get(ctx, APInt(8, i));
}
ConstantInt* llvm_context::create_i32(int32_t i) {
    return ConstantInt::get(ctx, APInt(32, i));
}
ConstantInt* llvm_context::create_size(size_t i) {
    return ConstantInt::get(ctx, APInt(sizeof(size_t) * 8, i));
}

ConstantFP* llvm_context::create_f32(float_t f) {
    return ConstantFP::get(ctx, APFloat(f));
}

Value* llvm_context::create_pop(Function* f) {
    auto pop_f = functions.at("stack_pop");
    return builder.CreateCall(pop_f, { unwrap_gmachine_stack_ptr(f->arg_begin()) });
}
Value* llvm_context::create_peek(Function* f, Value* off) {
    auto peek_f = functions.at("stack_peek");
    return builder.CreateCall(peek_f, { unwrap_gmachine_stack_ptr(f->arg_begin()), off });
}
void llvm_context::create_push(Function* f, Value* v) {
    auto push_f = functions.at("stack_push");
    builder.CreateCall(push_f, { unwrap_gmachine_stack_ptr(f->arg_begin()), v });
}
void llvm_context::create_popn(Function* f, Value* off) {
    auto popn_f = functions.at("stack_popn");
    builder.CreateCall(popn_f, { unwrap_gmachine_stack_ptr(f->arg_begin()), off });
}
void llvm_context::create_update(Function* f, Value* off) {
    auto update_f = functions.at("gmachine_update");
    builder.CreateCall(update_f, { f->arg_begin(), off });
}
void llvm_context::create_pack(Function* f, Value* c, Value* t) {
    auto pack_f = functions.at("gmachine_pack");
    builder.CreateCall(pack_f, { f->arg_begin(), c, t });
}
void llvm_context::create_split(Function* f, Value* c) {
    auto split_f = functions.at("gmachine_split");
    builder.CreateCall(split_f, { f->arg_begin(), c });
}
void llvm_context::create_slide(Function* f, Value* off) {
    auto slide_f = functions.at("gmachine_slide");
    builder.CreateCall(slide_f, { f->arg_begin(), off });
}
void llvm_context::create_alloc(Function* f, Value* n) {
    auto alloc_f = functions.at("gmachine_alloc");
    builder.CreateCall(alloc_f, { f->arg_begin(), n });
}
void llvm_context::create_enablegc(Function *f) {
    auto enablegc_f = functions.at("gmachine_enablegc");
    builder.CreateCall(enablegc_f, { f->arg_begin() });
}
void llvm_context::create_disablegc(Function *f) {
    auto disablegc_f = functions.at("gmachine_disablegc");
    builder.CreateCall(disablegc_f, { f->arg_begin() });
}
Value *llvm_context::create_track(Function *f, Value *v)
{
    auto track_f = functions.at("gmachine_track");
    return builder.CreateCall(track_f, { f->arg_begin(), v });
}

void llvm_context::create_unwind(Function* f) {
    auto unwind_f = functions.at("unwind");
    builder.CreateCall(unwind_f, { f->args().begin() });
}

Value* llvm_context::unwrap_gmachine_stack_ptr(Value* g) {
    auto offset_0 = create_i32(0);
    return builder.CreateGEP(g, { offset_0, offset_0 });
}

Value* llvm_context::unwrap_num(Value* v) {
    auto num_ptr_type = PointerType::getUnqual(struct_types.at("node_num"));
    auto cast = builder.CreatePointerCast(v, num_ptr_type);
    auto offset_0 = create_i32(0);
    auto offset_1 = create_i32(1);
    auto int_ptr = builder.CreateGEP(cast, { offset_0, offset_1 });
    return builder.CreateLoad(int_ptr);
}
Value* llvm_context::unwrap_float(Value* v) {
    auto float_ptr_type = PointerType::getUnqual(struct_types.at("node_float"));
    auto cast = builder.CreatePointerCast(v, float_ptr_type);
    auto offset_0 = create_i32(0);  // Not "create_f32(0)" here.
    auto offset_1 = create_i32(1);
    auto float_ptr = builder.CreateGEP(cast, { offset_0, offset_1 });
    return builder.CreateLoad(float_ptr);
}

Value* llvm_context::create_num(Function* f, Value* v) {
    auto alloc_num_f = functions.at("alloc_num");
    auto alloc_num_call = builder.CreateCall(alloc_num_f, { v });
    return create_track(f, alloc_num_call);
}
Value* llvm_context::create_float(Function* f, Value* v) {
    auto alloc_float_f = functions.at("alloc_float");
    auto alloc_float_call = builder.CreateCall(alloc_float_f, { v });
    return create_track(f, alloc_float_call);
}
Value* llvm_context::create_data(Function* f, Value* tag, Value* size) {
    auto alloc_data_f = functions.at("alloc_data");
    auto alloc_data_call = builder.CreateCall(alloc_data_f, { tag, size });
    return create_track(f, alloc_data_call);
}

Value* llvm_context::unwrap_data_tag(Value* v) {
    auto data_ptr_type = PointerType::getUnqual(struct_types.at("node_data"));
    auto cast = builder.CreatePointerCast(v, data_ptr_type);
    auto offset_0 = create_i32(0);
    auto offset_1 = create_i32(1);
    auto tag_ptr = builder.CreateGEP(cast, { offset_0, offset_1 });
    return builder.CreateLoad(tag_ptr);
}

Value* llvm_context::get_node_tag(Value* node_ptr) {
    auto offset_0 = create_i32(0);
    auto offset_tag = create_i32(0);
    auto tag_ptr = builder.CreateGEP(node_ptr, { offset_0, offset_tag });
    return builder.CreateLoad(tag_ptr);
}

Value* llvm_context::create_global(Function* f, Value* gf, Value* a) {
    auto alloc_global_f = functions.at("alloc_global");
    auto alloc_global_call = builder.CreateCall(alloc_global_f, { gf, a });
    return create_track(f, alloc_global_call);
}

Value* llvm_context::create_app(Function* f, Value* l, Value* r) {
    auto alloc_app_f = functions.at("alloc_app");
    auto alloc_app_call = builder.CreateCall(alloc_app_f, { l, r });
    return create_track(f, alloc_app_call);
}

llvm::Function* llvm_context::create_custom_function(std::string name, int32_t arity) {
    auto void_type = llvm::Type::getVoidTy(ctx);
    auto new_function = llvm::Function::Create(
            function_type,
            llvm::Function::LinkageTypes::ExternalLinkage,
            "f_" + name,
            &module
    );
    auto start_block = llvm::BasicBlock::Create(ctx, "entry", new_function);

    auto new_custom_f = custom_function_ptr(new custom_function());
    new_custom_f->arity = arity;
    new_custom_f->function = new_function;
    custom_functions["f_" + name] = std::move(new_custom_f);

    return new_function;
}
