#pragma once
#include <memory>
#include <string>

struct env {
    virtual ~env() = default;

    virtual int get_offset(const std::string& name) const = 0;
    virtual bool has_variable(const std::string& name) const = 0;
};

using env_ptr = std::shared_ptr<env>;

struct env_var : public env {
    std::string name;
    env_ptr parent;

    env_var(std::string& n, env_ptr p)
        : name(std::move(n)), parent(std::move(p)) {}

    int get_offset(const std::string& name) const;
    bool has_variable(const std::string& name) const;
};

struct env_offset : public env {
    int offset;
    env_ptr parent;

    env_offset(int o, env_ptr p)
        : offset(o), parent(std::move(p)) {}

    int get_offset(const std::string& name) const;
    bool has_variable(const std::string& name) const;
};
