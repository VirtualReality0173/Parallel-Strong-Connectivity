
#include "scc/pivot_scc.hpp"
#include "scc/reachability.hpp"

#include <stdexcept>

PivotPartition pivotSCC(
    const Graph& graph,
    int pivot
) {
    if (pivot < 0 || pivot >= graph.V) {
        throw std::out_of_range("Invalid pivot vertex");
    }

    // Vertices reachable FROM the pivot.
    std::vector<int> forward =
        parallelBFS(graph, pivot, false);

    // Vertices that can reach the pivot.
    std::vector<int> backward =
        parallelBFS(graph, pivot, true);

    PivotPartition result;

    // Partition every vertex into exactly one of four sets.
    for (int v = 0; v < graph.V; v++) {

        if (forward[v] && backward[v]) {
            result.scc.push_back(v);
        }
        else if (forward[v]) {
            result.forwardOnly.push_back(v);
        }
        else if (backward[v]) {
            result.backwardOnly.push_back(v);
        }
        else {
            result.neither.push_back(v);
        }
    }

    return result;
}
