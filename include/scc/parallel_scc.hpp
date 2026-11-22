
#ifndef SCC_PARALLEL_SCC_HPP
#define SCC_PARALLEL_SCC_HPP

#include "scc/graph.hpp"
#include <vector>

class ParallelSCC {
private:
    const Graph& graph;

public:
    explicit ParallelSCC(const Graph& g);

    std::vector<std::vector<int>> run();
};

#endif
