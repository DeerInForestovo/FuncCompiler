#include "graph.hpp"

int main() {
    function_graph graph;
    graph.add_edge("f", "g");
    graph.add_edge("g", "h");
    graph.add_edge("h", "f");

    graph.add_edge("i", "j");
    graph.add_edge("j", "i");

    graph.add_edge("j", "f");

    graph.add_edge("x", "f");
    graph.add_edge("x", "i");

    for(auto& group : graph.compute_order()) {
        std::cout << "Group: " << std::endl;
        for(auto& member : group->members) {
            std::cout << member << std::endl;
        }
    }
}
