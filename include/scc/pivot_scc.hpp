
#ifndef SCC_PIVOT_SCC_HPP
#define SCC_PIVOT_SCC_HPP

#include "scc/graph.hpp"
#include <vector>

struct PivotPartition {
    std::vector<int> scc;
    std::vector<int> forwardOnly;
    std::vector<int> backwardOnly;
    std::vector<int> neither;
};

PivotPartition pivotSCC(
    const Graph& graph,
    int pivot
);

#endif
