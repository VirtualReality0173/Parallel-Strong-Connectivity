
#ifndef SCC_TARJAN_HPP
#define SCC_TARJAN_HPP

#include "scc/graph.hpp"
#include <vector>

class TarjanSCC {
private:
    const Graph& graph;
    int timer = 0;
    std::vector<int> disc, low;
    std::vector<bool> onStack;
    std::vector<int> st;
    std::vector<std::vector<int>> groups;

    void dfs(int u);


public:
    explicit TarjanSCC(const Graph& g);

    std::vector<std::vector<int>> run();
};

#endif
