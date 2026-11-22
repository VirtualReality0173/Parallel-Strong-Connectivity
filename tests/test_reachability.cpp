
#include <iostream>
#include <vector>

#include "scc/graph.hpp"
#include "scc/reachability.hpp"

void printReachable(const std::vector<int>& result) {
    for (int i = 0; i < (int)result.size(); i++) {
        if (result[i]) {
            std::cout << i << " ";
        }
    }
    std::cout << "\n";
}

int main() {
    Graph g(6);

    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(2, 0);

    g.addEdge(2, 3);
    g.addEdge(3, 4);
    g.addEdge(4, 5);

    auto forward = parallelBFS(g, 0, false);
    auto backward = parallelBFS(g, 0, true);

    std::cout << "Forward reachable from 0: ";
    printReachable(forward);

    std::cout << "Backward reachable to 0: ";
    printReachable(backward);

    return 0;
}
