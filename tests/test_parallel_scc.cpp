
#include <iostream>
#include <vector>
#include <algorithm>

#include "scc/graph.hpp"
#include "scc/tarjan.hpp"
#include "scc/parallel_scc.hpp"

using Components = std::vector<std::vector<int>>;

// Normalize SCC output so ordering doesn't affect comparison.
Components normalize(Components components) {

    for (auto& component : components) {
        std::sort(component.begin(), component.end());
    }

    std::sort(components.begin(), components.end());

    return components;
}

int main() {

    Graph g(8);

    // SCC 1: 0, 1
    g.addEdge(0, 1);
    g.addEdge(1, 0);

    // Connection to SCC 2
    g.addEdge(1, 2);

    // SCC 2: 2, 3, 4
    g.addEdge(2, 3);
    g.addEdge(3, 4);
    g.addEdge(4, 2);

    // Connection to SCC 3
    g.addEdge(4, 5);

    // SCC 3: 5, 6
    g.addEdge(5, 6);
    g.addEdge(6, 5);

    // Vertex 7 is isolated.

    // Sequential reference.
    TarjanSCC tarjan(g);
    Components sequential = tarjan.run();

    // Parallel implementation.
    ParallelSCC parallel(g);
    Components parallelResult = parallel.run();

    sequential = normalize(sequential);
    parallelResult = normalize(parallelResult);

    std::cout << "Sequential SCC count: "
              << sequential.size() << "\n";

    std::cout << "Parallel SCC count: "
              << parallelResult.size() << "\n";

    if (sequential == parallelResult) {
        std::cout << "\nSUCCESS: Both algorithms produce "
                     "identical SCC partitions.\n";
    }
    else {
        std::cout << "\nFAILURE: SCC partitions differ.\n";

        return 1;
    }

    std::cout << "\nParallel SCCs:\n";

    for (int i = 0; i < (int)parallelResult.size(); i++) {

        std::cout << "SCC " << i << ": ";

        for (int v : parallelResult[i]) {
            std::cout << v << " ";
        }

        std::cout << "\n";
    }

    return 0;
}
