
#include "scc/graph.hpp"

Graph::Graph(int vertices)
    : V(vertices), adj(vertices), rev_adj(vertices) {}

void Graph::addEdge(int u, int v) {
    adj[u].push_back(v);
    rev_adj[v].push_back(u);
}
