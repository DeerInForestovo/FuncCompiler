#pragma once
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <map>

struct llvm_context {
    struct custom_function {
        llvm::Function* function;
        int32_t arity;
    };

    using custom_function_ptr = std::unique_ptr<custom_function>;

    llvm::LLVMContext ctx;
    llvm::IRBuilder<> builder;
    llvm::Module module;

    std::map<std::string, custom_function_ptr> custom_functions;
    std::map<std::string, llvm::Function*> functions;
    std::map<std::string, llvm::StructType*> struct_types;

    llvm::StructType* stack_type;
    llvm::StructType* gmachine_type;
    llvm::PointerType* stack_ptr_type;
    llvm::PointerType* gmachine_ptr_type;
    llvm::PointerType* node_ptr_type;
    llvm::IntegerType* tag_type;
    llvm::FunctionType* function_type;

    llvm_context()
        : builder(ctx), module("bloglang", ctx) {
        create_types();
        create_functions();
    }

    void create_types();
    void create_functions();

    llvm::ConstantInt* create_i8(int8_t);
    llvm::ConstantInt* create_i32(int32_t);
    llvm::ConstantInt* create_size(size_t);

    llvm::Value* create_pop(llvm::Function*);
    llvm::Value* create_peek(llvm::Function*, llvm::Value*);
    void create_push(llvm::Function*, llvm::Value*);
    void create_popn(llvm::Function*, llvm::Value*);
    void create_update(llvm::Function*, llvm::Value*);
    void create_pack(llvm::Function*, llvm::Value*, llvm::Value*);
    void create_split(llvm::Function*, llvm::Value*);
    void create_slide(llvm::Function*, llvm::Value*);
    void create_alloc(llvm::Function*, llvm::Value*);
    llvm::Value* create_track(llvm::Function*, llvm::Value*);

    void create_unwind(llvm::Function*);

    llvm::Value* unwrap_gmachine_stack_ptr(llvm::Value*);

    llvm::Value* unwrap_num(llvm::Value*);
    llvm::Value* create_num(llvm::Function*, llvm::Value*);

    llvm::Value* unwrap_data_tag(llvm::Value*);

    llvm::Value* create_global(llvm::Function*, llvm::Value*, llvm::Value*);

    llvm::Value* create_app(llvm::Function*, llvm::Value*, llvm::Value*);

    llvm::Function* create_custom_function(std::string name, int32_t arity);
};
