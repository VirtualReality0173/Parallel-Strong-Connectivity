
#include <iostream>

#include "scc/graph.hpp"
#include "scc/pivot_scc.hpp"

void printSet(
    const char* name,
    const std::vector<int>& vertices
) {
    std::cout << name << ": ";

    for (int v : vertices) {
        std::cout << v << " ";
    }

    std::cout << "\n";
}

int main() {
    Graph g(8);

    // First SCC: 0, 1
    g.addEdge(0, 1);
    g.addEdge(1, 0);

    // Connection to second SCC
    g.addEdge(1, 2);

    // Second SCC: 2, 3, 4
    g.addEdge(2, 3);
    g.addEdge(3, 4);
    g.addEdge(4, 2);

    // Connection to third SCC
    g.addEdge(4, 5);

    // Third SCC: 5, 6
    g.addEdge(5, 6);
    g.addEdge(6, 5);

    // Vertex 7 is isolated.

    PivotPartition result = pivotSCC(g, 0);

    printSet("Pivot SCC", result.scc);
    printSet("Forward only", result.forwardOnly);
    printSet("Backward only", result.backwardOnly);
    printSet("Neither", result.neither);

    return 0;
}
