#pragma once
#include <memory>
#include <map>
#include <string>
#include <vector>
#include <set>

struct type_mgr;

struct type {
    virtual ~type() = default;

    virtual void print(const type_mgr& mgr, std::ostream& to) const = 0;
};

using type_ptr = std::shared_ptr<type>;

struct type_scheme {
    std::vector<std::pair<std::string, bool>> forall;
    type_ptr monotype;

    type_scheme(type_ptr type) : forall(), monotype(std::move(type)) {}

    void print(const type_mgr& mgr, std::ostream& to) const;
    type_ptr instantiate(type_mgr& mgr) const;
};

using type_scheme_ptr = std::shared_ptr<type_scheme>;

struct type_var : public type {
    std::string name;
    bool num_type;

    type_var(std::string n)
        : name(std::move(n)), num_type(false) {}

    void set_num_type();
    void print(const type_mgr& mgr, std::ostream& to) const;
};

struct type_base : public type {
    std::string name;
    int32_t arity;

    type_base(std::string n, int32_t a = 0) 
        : name(std::move(n)), arity(a) {}

    void print(const type_mgr& mgr, std::ostream& to) const;
};

struct type_data : public type_base {
    struct constructor {
        int tag;
    };

    std::map<std::string, constructor> constructors;

    type_data(std::string n, int32_t a = 0)
        : type_base(std::move(n), a) {}
};

struct type_arr : public type {
    type_ptr left;
    type_ptr right;

    type_arr(type_ptr l, type_ptr r)
        : left(std::move(l)), right(std::move(r)) {}

    void print(const type_mgr& mgr, std::ostream& to) const;
};

struct type_app : public type {
    type_ptr constructor;
    std::vector<type_ptr> arguments;

    type_app(type_ptr c)
        : constructor(std::move(c)) {}

    void print(const type_mgr& mgr, std::ostream& to) const;
};

struct type_mgr {
    int last_id = 0;
    std::map<std::string, type_ptr> types;

    std::string new_type_name();
    type_ptr new_type();
    type_ptr new_num_type();
    type_ptr new_arrow_type();

    void unify(type_ptr l, type_ptr r);
    type_ptr substitute(
            const std::map<std::string, type_ptr>& subst,
            const type_ptr& t) const;
    type_ptr resolve(type_ptr t, type_var*& var) const;
    bool bind(type_var* s, type_ptr t);  // return bind success or not
    void find_free(const type_ptr& t, std::set<std::pair<std::string, bool>>& into) const;
};
