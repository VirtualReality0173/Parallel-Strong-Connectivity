
#ifndef SCC_REACHABILITY_HPP
#define SCC_REACHABILITY_HPP

#include "scc/graph.hpp"
#include <vector>

// Performs level-synchronous parallel BFS.
// reverse = false: forward reachability
// reverse = true:  backward reachability
//
// Returns a vector where reachable[v] = 1
// if vertex v is reachable from the source.

std::vector<int> parallelBFS(
    const Graph& graph,
    int source,
    bool reverse = false
);

#endif
