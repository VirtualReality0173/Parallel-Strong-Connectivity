#include <iostream>
#include <vector>
#include <algorithm>
#include <random>

#include "scc/graph.hpp"
#include "scc/tarjan.hpp"
#include "scc/parallel_scc.hpp"

using namespace std;

// Normalize SCC output before comparing.
vector<vector<int>> normalize(vector<vector<int>> components) {
    for (auto& component : components) {
        sort(component.begin(), component.end());
    }

    sort(components.begin(), components.end());
    return components;
}

int main() {
    const int NUM_TESTS = 100;

    // Fixed seed: makes the tests reproducible.
    mt19937 rng(42);

    // Randomly choose graph sizes between 1 and 50.
    uniform_int_distribution<int> vertexDist(1, 50);

    // Randomly choose edge probabilities.
    uniform_real_distribution<double> probabilityDist(0.02, 0.20);

    int passed = 0;

    for (int test = 1; test <= NUM_TESTS; test++) {

        int V = vertexDist(rng);
        double probability = probabilityDist(rng);

        Graph graph(V);

        // Generate random directed edges.
        bernoulli_distribution edgeDist(probability);

        for (int u = 0; u < V; u++) {
            for (int v = 0; v < V; v++) {

                if (edgeDist(rng)) {
                    graph.addEdge(u, v);
                }
            }
        }

        // Run both algorithms.
        TarjanSCC tarjan(graph);
        ParallelSCC parallel(graph);

        auto sequentialResult = normalize(tarjan.run());
        auto parallelResult = normalize(parallel.run());

        // Compare the SCC partitions.
        if (sequentialResult == parallelResult) {
            passed++;
        }
        else {
            cout << "FAILED at test " << test << "\n";
            cout << "Vertices: " << V << "\n";
            cout << "Edge probability: " << probability << "\n";

            cout << "Tarjan SCC count: "
                 << sequentialResult.size() << "\n";

            cout << "Parallel SCC count: "
                 << parallelResult.size() << "\n";

            return 1;
        }

        if (test % 10 == 0) {
            cout << "Completed " << test << " tests...\n";
        }
    }

    cout << "\nRandomized testing completed.\n";
    cout << "Passed: " << passed
         << " / " << NUM_TESTS << "\n";

    return (passed == NUM_TESTS) ? 0 : 1;
}