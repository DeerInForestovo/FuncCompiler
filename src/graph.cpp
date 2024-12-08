#include "graph.hpp"

std::set<function_graph::edge> function_graph::compute_transitive_edges() {
    std::set<edge> transitive_edges;
    transitive_edges.insert(edges.begin(), edges.end());
    for(auto& connector : adjacency_lists) {
        for(auto& from : adjacency_lists) {
            edge to_connector { from.first, connector.first };
            for(auto& to : adjacency_lists) {
                edge full_jump { from.first, to.first };
                if(transitive_edges.find(full_jump) != transitive_edges.end()) continue;

                edge from_connector { connector.first, to.first };
                if(transitive_edges.find(to_connector) != transitive_edges.end() &&
                        transitive_edges.find(from_connector) != transitive_edges.end())
                    transitive_edges.insert(std::move(full_jump));
            }
        }
    }
    return transitive_edges;
}

void function_graph::create_groups(
        const std::set<edge>& transitive_edges,
        std::map<function, group_id>& group_ids,
        std::map<group_id, data_ptr>& group_data_map) {
    group_id id_counter = 0;
    for(auto& vertex : adjacency_lists) {
        if(group_ids.find(vertex.first) != group_ids.end())
            continue;
        data_ptr new_group(new group_data);
        new_group->functions.insert(vertex.first);
        group_data_map[id_counter] = new_group;
        group_ids[vertex.first] = id_counter;
        for(auto& other_vertex : adjacency_lists) {
            if(transitive_edges.find({vertex.first, other_vertex.first}) != transitive_edges.end() &&
                    transitive_edges.find({other_vertex.first, vertex.first}) != transitive_edges.end()) {
                group_ids[other_vertex.first] = id_counter;
                new_group->functions.insert(other_vertex.first);
            }
        }
        id_counter++;
    }
}

void function_graph::create_edges(
        std::map<function, group_id>& group_ids,
        std::map<group_id, data_ptr>& group_data_map) {
    std::set<std::pair<group_id, group_id>> group_edges;
    for(auto& vertex : adjacency_lists) {
        auto vertex_id = group_ids[vertex.first];
        auto& vertex_data = group_data_map[vertex_id];
        for(auto& other_vertex : vertex.second) {
            auto other_id = group_ids[other_vertex];
            if(vertex_id == other_id) continue;
            if(group_edges.find({vertex_id, other_id}) != group_edges.end())
                continue;
            group_edges.insert({vertex_id, other_id});
            vertex_data->adjacency_list.insert(other_id);
            group_data_map[other_id]->indegree++;
        }
    }
}

std::vector<group_ptr> function_graph::generate_order(
        std::map<function, group_id>& group_ids,
        std::map<group_id, data_ptr>& group_data_map) {
    std::queue<group_id> id_queue;
    std::vector<group_ptr> output;
    for(auto& group : group_data_map) {
        if(group.second->indegree == 0) id_queue.push(group.first);
    }

    while(!id_queue.empty()) {
        auto new_id = id_queue.front();
        auto& group_data = group_data_map[new_id];
        group_ptr output_group(new group);
        output_group->members = std::move(group_data->functions);
        id_queue.pop();

        for(auto& adjacent_group : group_data->adjacency_list) {
            if(--group_data_map[adjacent_group]->indegree == 0)
                id_queue.push(adjacent_group);
        }

        output.push_back(std::move(output_group));
    }

    return output;
}

std::set<function>& function_graph::add_function(const function& f) {
    auto adjacency_list_it = adjacency_lists.find(f);
    if(adjacency_list_it != adjacency_lists.end()) {
        return adjacency_list_it->second;
    } else {
        return adjacency_lists[f] = { };
    }
}

void function_graph::add_edge(const function& from, const function& to) {
    add_function(from).insert(to);
    edges.insert({ from, to });
}

std::vector<group_ptr> function_graph::compute_order() {
    std::set<edge> transitive_edges = compute_transitive_edges();
    std::map<function, group_id> group_ids;
    std::map<group_id, data_ptr> group_data_map;

    create_groups(transitive_edges, group_ids, group_data_map);
    create_edges(group_ids, group_data_map);
    return generate_order(group_ids, group_data_map);
}
