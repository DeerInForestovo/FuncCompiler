#pragma once
#include <algorithm>
#include <cstddef>
#include <queue>
#include <set>
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <iostream>

using function = std::string;

struct group {
    std::set<function> members;
};

using group_ptr = std::unique_ptr<group>;

class function_graph {
    using group_id = size_t;

    struct group_data {
        std::set<function> functions;
        std::set<group_id> adjacency_list;
        size_t indegree;
    };

    using data_ptr = std::shared_ptr<group_data>;
    using edge = std::pair<function, function>;
    using group_edge = std::pair<group_id, group_id>;

    std::map<function, std::set<function>> adjacency_lists;
    std::set<edge> edges;

    std::set<edge> compute_transitive_edges();
    void create_groups(
            const std::set<edge>&,
            std::map<function, group_id>&,
            std::map<group_id, data_ptr>&);
    void create_edges(
            std::map<function, group_id>&,
            std::map<group_id, data_ptr>&);
    std::vector<group_ptr> generate_order(
            std::map<function, group_id>&,
            std::map<group_id, data_ptr>&);
    
    public:
    std::set<function>& add_function(const function& f);
    void add_edge(const function& from, const function& to);
    std::vector<group_ptr> compute_order();
};
