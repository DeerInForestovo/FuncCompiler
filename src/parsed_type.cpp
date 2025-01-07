#include "parsed_type.hpp"
#include "type.hpp"
#include "type_env.hpp"
#include "error.hpp"

type_ptr parsed_type_app::to_type(
        const std::set<std::string>& vars,
        const type_env& e) const {
    auto parent_type = e.lookup_type(name);
    if(parent_type == nullptr) throw unexpected_error("parsed_type_app::to_type: name not found");
    type_base* base_type;
    if(!(base_type = dynamic_cast<type_base*>(parent_type.get()))) throw unexpected_error("parsed_type_app::to_type: parent_type is not type_base");
    if(base_type->arity != arguments.size()) throw unexpected_error("parsed_type_app::to_type: base_type->arity != arguments.size()");

    type_app* new_app = new type_app(std::move(parent_type));
    type_ptr to_return(new_app);
    for(auto& arg : arguments) {
        new_app->arguments.push_back(arg->to_type(vars, e));
    }
    return to_return;
}

type_ptr parsed_type_var::to_type(
        const std::set<std::string>& vars,
        const type_env& e) const {
    if(vars.find(var) == vars.end()) throw unexpected_error("parsed_type_var::to_type: name not found");
    return type_ptr(new type_var(var));
}

type_ptr parsed_type_arr::to_type(
        const std::set<std::string>& vars,
        const type_env& env) const {
    auto new_left = left->to_type(vars, env);
    auto new_right = right->to_type(vars, env);
    return type_ptr(new type_arr(std::move(new_left), std::move(new_right)));
}
