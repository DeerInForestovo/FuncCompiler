#include "type.hpp"
#include <sstream>
#include <algorithm>

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

type_ptr type_mgr::new_arrow_type() {
    return type_ptr(new type_arr(new_type(), new_type()));
}

type_ptr type_mgr::resolve(type_ptr t, type_var*& var) {
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
    type_var* lvar;
    type_var* rvar;
    type_arr* larr;
    type_arr* rarr;
    type_base* lid;
    type_base* rid;

    l = resolve(l, lvar);
    r = resolve(r, rvar);

    if(lvar) {
        bind(lvar->name, r);
        return;
    } else if(rvar) {
        bind(rvar->name, l);
        return;
    } else if((larr = dynamic_cast<type_arr*>(l.get())) &&
            (rarr = dynamic_cast<type_arr*>(r.get()))) {
        unify(larr->left, rarr->left);
        unify(larr->right, rarr->right);
        return;
    } else if((lid = dynamic_cast<type_base*>(l.get())) &&
            (rid = dynamic_cast<type_base*>(r.get()))) {
        if(lid->name == rid->name) return;
    }

    throw 0;
}

void type_mgr::bind(const std::string& s, type_ptr t) {
    type_var* other = dynamic_cast<type_var*>(t.get());
    
    if(other && other->name == s) return;
    types[s] = t;
}