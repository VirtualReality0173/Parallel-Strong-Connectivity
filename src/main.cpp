
#include <iostream>
#include "scc/graph.hpp"
#include "scc/tarjan.hpp"

int main() {
    int V, E;

    std::cout << "Enter number of vertices and edges: ";
    std::cin >> V >> E;

    Graph graph(V);

    std::cout << "Enter directed edges (u v):\n";

    for (int i = 0; i < E; i++) {
        int u, v;
        std::cin >> u >> v;

        graph.addEdge(u, v);
    }

    TarjanSCC tarjan(graph);
    auto components = tarjan.run();

    std::cout << "\nNumber of SCCs: "
              << components.size() << "\n";

    for (int i = 0; i < (int)components.size(); i++) {
        std::cout << "SCC " << i << ":";

        for (int v : components[i]) {
            std::cout << " " << v;
        }

        std::cout << "\n";
    }

    return 0;
}
