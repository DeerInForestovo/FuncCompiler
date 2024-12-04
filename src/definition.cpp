#include "ast.hpp"

void definition_defn::typecheck_first(type_mgr& mgr, type_env& env) {
    return_type = mgr.new_type();
    type_ptr full_type = return_type;

    for(auto it = params.rbegin(); it != params.rend(); it++) {
        type_ptr param_type = mgr.new_type();
        full_type = type_ptr(new type_arr(param_type, full_type));
        param_types.push_back(param_type);
    }

    env.bind(name, full_type);
}

void definition_defn::typecheck_second(type_mgr& mgr, const type_env& env) const {
    type_env new_env = env.scope();
    auto param_it = params.begin();
    auto type_it = param_types.rbegin();

    while(param_it != params.end() && type_it != param_types.rend()) {
        new_env.bind(*param_it, *type_it);
        param_it++;
        type_it++;
    }

    type_ptr body_type = body->typecheck(mgr, new_env);
    mgr.unify(return_type, body_type);
}

void definition_data::typecheck_first(type_mgr& mgr, type_env& env) {
    type_ptr return_type = type_ptr(new type_base(name));

    for(auto& constructor : constructors) {
        type_ptr full_type = return_type;

        for(auto& type_name : constructor->types) {
            type_ptr type = type_ptr(new type_base(type_name));
            full_type = type_ptr(new type_arr(type, full_type));
        }

        env.bind(constructor->name, full_type);
    }
}

void definition_data::typecheck_second(type_mgr& mgr, const type_env& env) const {
    // Nothing
}