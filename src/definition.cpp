#include "definition.hpp"
#include "error.hpp"
#include "ast.hpp"
#include "instruction.hpp"
#include "llvm_context.hpp"
#include "type.hpp"
#include "type_env.hpp"
#include <algorithm>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Type.h>

void definition_defn::find_free(type_mgr& mgr, type_env_ptr& env) {
    this->env = env;

    var_env = type_scope(env);
    return_type = mgr.new_type();
    full_type = return_type;

    for(auto it = params.rbegin(); it != params.rend(); it++) {
        type_ptr param_type = mgr.new_type();
        full_type = type_ptr(new type_arr(param_type, full_type));
        var_env->bind(*it, param_type);
    }

    body->find_free(mgr, var_env, free_variables);
}

void definition_defn::insert_types(type_mgr& mgr) {
    env->bind(name, full_type);
}

void definition_defn::typecheck(type_mgr& mgr) {
    type_ptr body_type = body->typecheck(mgr);
    mgr.unify(return_type, body_type);
}

void definition_defn::compile() {
    env_ptr new_env = env_ptr(new env_offset(0, nullptr));
    for(auto it = params.rbegin(); it != params.rend(); it++) {
        new_env = env_ptr(new env_var(*it, new_env));
    }
    body->compile(new_env, instructions);
    instructions.push_back(instruction_ptr(new instruction_update(params.size())));
    instructions.push_back(instruction_ptr(new instruction_pop(params.size())));
}

void definition_defn::declare_llvm(llvm_context& ctx) {
    generated_function = ctx.create_custom_function(name, params.size());
}

void definition_defn::generate_llvm(llvm_context& ctx) {
    ctx.builder.SetInsertPoint(&generated_function->getEntryBlock());
    for(auto& instruction : instructions) {
        instruction->gen_llvm(ctx, generated_function);
    }
    ctx.builder.CreateRetVoid();
}

void definition_data::insert_types(type_env_ptr& env) {
    this->env = env;
    env->bind_type(name, type_ptr(new type_data(name, vars.size())));
}

void definition_data::insert_constructors() const {
    type_ptr this_type_ptr = env->lookup_type(name);
    type_data* this_type = static_cast<type_data*>(this_type_ptr.get());
    int next_tag = 0;

    std::set<std::string> var_set;
    type_app* return_app = new type_app(std::move(this_type_ptr));
    type_ptr return_type(return_app);
    for(auto& var : vars) {
        if(var_set.find(var) != var_set.end())
            throw type_error("Duplicated placeholders in the data definition.");
        var_set.insert(var);
        return_app->arguments.push_back(type_ptr(new type_var(var)));
    }

    for(auto& constructor : constructors) {
        constructor->tag = next_tag;
        this_type->constructors[constructor->name] = { next_tag++ };

        type_ptr full_type = return_type;
        for(auto it = constructor->types.rbegin(); it != constructor->types.rend(); it++) {
            type_ptr type = (*it)->to_type(var_set, env);
            full_type = type_ptr(new type_arr(type, full_type));
        }

        type_scheme_ptr full_scheme(new type_scheme(std::move(full_type)));
        full_scheme->forall.reserve(vars.size());
        std::transform(vars.begin(), vars.end(), std::back_inserter(full_scheme->forall), 
            [](std::string var) { return std::make_pair(var, false); });
        
        env->bind(constructor->name, full_scheme);
    }
}

void definition_data::generate_llvm(llvm_context& ctx) {
    for(auto& constructor : constructors) {
        auto new_function =
            ctx.create_custom_function(constructor->name, constructor->types.size());
        std::vector<instruction_ptr> instructions;
        instructions.push_back(instruction_ptr(
                new instruction_pack(constructor->tag, constructor->types.size())
        ));
        instructions.push_back(instruction_ptr(new instruction_update(0)));
        ctx.builder.SetInsertPoint(&new_function->getEntryBlock());
        for (auto& instruction : instructions) {
            instruction->gen_llvm(ctx, new_function);
        }
        ctx.builder.CreateRetVoid();
    }
}
