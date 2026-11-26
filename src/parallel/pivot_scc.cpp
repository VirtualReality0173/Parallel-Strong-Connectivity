
#include "scc/pivot_scc.hpp"
#include "scc/reachability.hpp"

#include <stdexcept>
#include <vector>

PivotPartition pivotSCC(
    const Graph& graph,
    int pivot
) {
    if (pivot < 0 || pivot >= graph.V) {
        throw std::out_of_range("Invalid pivot vertex");
    }

    // Get vertices reachable FROM the pivot.
    std::vector<int> forward =
        parallelBFS(graph, pivot, false);

    // Get vertices that can reach the pivot.
    std::vector<int> backward =
        parallelBFS(graph, pivot, true);

    // Convert the returned vertex lists into membership arrays.
    std::vector<bool> forwardSeen(graph.V, false);
    std::vector<bool> backwardSeen(graph.V, false);

    for (int v : forward) {
        forwardSeen[v] = true;
    }

    for (int v : backward) {
        backwardSeen[v] = true;
    }

    PivotPartition result;

    // Classify every vertex into exactly one of four sets.
    for (int v = 0; v < graph.V; ++v) {

        if (forwardSeen[v] && backwardSeen[v]) {
            // Reachable from pivot AND can reach pivot.
            result.scc.push_back(v);
        }
        else if (forwardSeen[v]) {
            result.forwardOnly.push_back(v);
        }
        else if (backwardSeen[v]) {
            result.backwardOnly.push_back(v);
        }
        else {
            result.neither.push_back(v);
        }
    }

    return result;
}
