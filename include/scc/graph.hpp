
#ifndef SCC_GRAPH_HPP
#define SCC_GRAPH_HPP

#include <vector>

class Graph {
public:
    int V;

    // Forward adjacency list
    std::vector<std::vector<int>> adj;

    // Reverse adjacency list
    std::vector<std::vector<int>> rev_adj;

    explicit Graph(int vertices);

    void addEdge(int u, int v);
};

#endif
