
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

#include "scc/graph.hpp"
#include "scc/tarjan.hpp"
#include "scc/parallel_scc.hpp"

using namespace std;

// Sort vertices within each SCC and then sort the SCCs.
// This lets us compare partitions regardless of output order.
vector<vector<int>> normalize(vector<vector<int>> components) {
    for (auto& component : components) {
        sort(component.begin(), component.end());
    }

    sort(components.begin(), components.end());
    return components;
}

bool testCase(
    const string& name,
    int V,
    const vector<pair<int, int>>& edges
) {
    Graph graph(V);

    for (const auto& edge : edges) {
    int u = edge.first;
    int v = edge.second;

    graph.addEdge(u, v);
}

    TarjanSCC tarjan(graph);
    ParallelSCC parallel(graph);

    auto sequentialResult = normalize(tarjan.run());
    auto parallelResult = normalize(parallel.run());

    bool passed = (sequentialResult == parallelResult);

    cout << name << ": "
         << (passed ? "PASSED" : "FAILED") << "\n";

    if (!passed) {
        cout << "Tarjan SCC count: "
             << sequentialResult.size() << "\n";

        cout << "Parallel SCC count: "
             << parallelResult.size() << "\n";
    }

    return passed;
}

int main() {
    int passed = 0;
    int total = 0;

    auto run = [&](const string& name, int V,
                   const vector<pair<int, int>>& edges) {
        total++;
        if (testCase(name, V, edges)) {
            passed++;
        }
    };

    // 1. Empty graph
    run("Empty graph", 0, {});

    // 2. Single vertex, no edges
    run("Single isolated vertex", 1, {});

    // 3. Multiple vertices, no edges
    run("Graph with no edges", 5, {});

    // 4. All vertices belong to one SCC
    run("One large SCC", 6, {
        {0, 1}, {1, 2}, {2, 3},
        {3, 4}, {4, 5}, {5, 0}
    });

    // 5. Directed chain: every vertex is its own SCC
    run("Directed chain", 6, {
        {0, 1}, {1, 2}, {2, 3},
        {3, 4}, {4, 5}
    });

    // 6. Several disconnected SCCs and isolated vertices
    run("Disconnected SCCs", 7, {
        {0, 1}, {1, 0},
        {2, 3}, {3, 4}, {4, 2},
        {5, 6}
    });

    // 7. Self-loops
    run("Self-loops", 4, {
        {0, 0}, {1, 1}, {2, 2}, {3, 3}
    });

    cout << "\nPassed " << passed
         << " out of " << total << " tests.\n";

    return (passed == total) ? 0 : 1;
}
