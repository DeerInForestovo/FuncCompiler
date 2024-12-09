#include "type.hpp"
#include <ostream>
#include <sstream>
#include <algorithm>
#include <vector>
#include <iostream>
#include "error.hpp"

void type_scheme::print(const type_mgr& mgr, std::ostream& to) const {
    if(forall.size() != 0) {
        to << "forall ";
        for(auto& var : forall) {
            to << var.first << (var.second ? "(Num) " :  " ");
        }
        to << ". ";
    }
    monotype->print(mgr, to);
}

type_ptr type_scheme::instantiate(type_mgr& mgr) const {
    if(forall.size() == 0) return monotype;
    std::map<std::string, type_ptr> subst;
    for(auto& var : forall) {
        subst[var.first] = var.second ? mgr.new_num_type() : mgr.new_type();
    }
    return mgr.substitute(subst, monotype);
}

void type_var::print(const type_mgr& mgr, std::ostream& to) const {
    auto it = mgr.types.find(name);
    if(it != mgr.types.end()) {
        it->second->print(mgr, to);
    } else {
        to << name;
    }
}

void type_var::set_num_type() {
    num_type = true;
}

void type_base::print(const type_mgr& mgr, std::ostream& to) const {
    to << name;
}

void type_arr::print(const type_mgr& mgr, std::ostream& to) const {
    left->print(mgr, to);
    to << " -> (";
    right->print(mgr, to);
    to << ")";
}

void type_app::print(const type_mgr& mgr, std::ostream& to) const {
    constructor->print(mgr, to);
    to << "* ";
    for(auto& arg : arguments) {
        to << " ";
        arg->print(mgr, to);
    }
}

std::string type_mgr::new_type_name() {
    int temp = last_id++;
    std::string str = "";

    while(temp != -1) {
        str += (char) ('a' + (temp % 26));
        temp = temp / 26 - 1;
    }

    std::reverse(str.begin(), str.end());
    return str;
}

type_ptr type_mgr::new_type() {
    return type_ptr(new type_var(new_type_name()));
}

type_ptr type_mgr::new_num_type() {
    auto type_num_var = new type_var(new_type_name());
    type_num_var->set_num_type();
    return type_ptr(type_num_var);
}

type_ptr type_mgr::new_arrow_type() {
    return type_ptr(new type_arr(new_type(), new_type()));
}

type_ptr type_mgr::resolve(type_ptr t, type_var*& var) const {
    type_var* cast;

    var = nullptr;
    while((cast = dynamic_cast<type_var*>(t.get()))) {
        auto it = types.find(cast->name);

        if(it == types.end()) {
            var = cast;
            break;
        }
        t = it->second;
    }

    return t;
}

void type_mgr::unify(type_ptr l, type_ptr r) {
    type_var *lvar, *rvar;
    type_arr *larr, *rarr;
    type_base *lid, *rid;
    type_app *lapp, *rapp;

    l = resolve(l, lvar);
    r = resolve(r, rvar);

    if(lvar) {
        if (bind(lvar, r)) return;
    } else if(rvar) {
        if (bind(rvar, l)) return;
    } else if((larr = dynamic_cast<type_arr*>(l.get())) &&
            (rarr = dynamic_cast<type_arr*>(r.get()))) {
        unify(larr->left, rarr->left);
        unify(larr->right, rarr->right);
        return;
    } else if((lid = dynamic_cast<type_base*>(l.get())) &&
            (rid = dynamic_cast<type_base*>(r.get()))) {
        if(lid->name == rid->name && lid->arity == rid->arity) return;
    } else if((lapp = dynamic_cast<type_app*>(l.get())) &&
            (rapp = dynamic_cast<type_app*>(r.get()))) {
        unify(lapp->constructor, rapp->constructor);
        auto left_it = lapp->arguments.begin();
        auto right_it = rapp->arguments.begin();
        while(left_it != lapp->arguments.end() &&
                right_it != rapp->arguments.end()) {
            unify(*left_it, *right_it);
            left_it++, right_it++;
        }
        return;
    }

    throw unification_error(l, r);
}

type_ptr type_mgr::substitute(const std::map<std::string, type_ptr>& subst, const type_ptr& t) const {
    type_ptr temp = t;
    while(type_var* var = dynamic_cast<type_var*>(temp.get())) {
        auto subst_it = subst.find(var->name);
        if(subst_it != subst.end()) return subst_it->second;
        auto var_it = types.find(var->name);
        if(var_it == types.end()) return t;
        temp = var_it->second;
    }

    if(type_arr* arr = dynamic_cast<type_arr*>(temp.get())) {
        auto left_result = substitute(subst, arr->left);
        auto right_result = substitute(subst, arr->right);
        if(left_result == arr->left && right_result == arr->right) return t;
        return type_ptr(new type_arr(left_result, right_result));
    } else if(type_app* app = dynamic_cast<type_app*>(temp.get())) {
        auto constructor_result = substitute(subst, app->constructor);
        bool arg_changed = false;
        std::vector<type_ptr> new_args;
        for(auto& arg : app->arguments) {
            auto arg_result = substitute(subst, arg);
            arg_changed |= arg_result != arg;
            new_args.push_back(std::move(arg_result));
        }

        if(constructor_result == app->constructor && !arg_changed) return t;
        type_app* new_app = new type_app(std::move(constructor_result));
        std::swap(new_app->arguments, new_args);
        return type_ptr(new_app);
    }
    return t;
}

bool type_mgr::bind(type_var* s, type_ptr t) {
    // std::cout << "bind s=" << s->name << std::endl;
    type_var* tvar = dynamic_cast<type_var*>(t.get());
    if (tvar && tvar->name == s->name) return true;  // No need to bind
    if (s->num_type) {
        if (tvar) {
            tvar->set_num_type();  // Pass num_type tag
        } else {
            type_base* tid = dynamic_cast<type_base*>(t.get());
            if (!tid) return std::cout << "error: Bind num_type to app/arr" << std::endl, false;
            if (tid->name != "Int" && tid->name != "Float")
                return std::cout << "error: Bind num_type to not Int*/Float*" << std::endl, false;
        }
    }
    types[s->name] = t;
    return true;
}

void type_mgr::find_free(const type_ptr& t, std::set<std::pair<std::string, bool>>& into) const {
    type_var* var;
    type_ptr resolved = resolve(t, var);

    if(var) {
        into.emplace(var->name, var->num_type);
    } else if(type_arr* arr = dynamic_cast<type_arr*>(resolved.get())) {
        find_free(arr->left, into);
        find_free(arr->right, into);
    } else if(type_app* app = dynamic_cast<type_app*>(resolved.get())) {
        find_free(app->constructor, into);
        for(auto& arg : app->arguments) find_free(arg, into);
    }
}
