
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <iomanip>

#include "scc/graph.hpp"
#include "scc/tarjan.hpp"
#include "scc/parallel_scc.hpp"

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace std;
using namespace chrono;

using Components = vector<vector<int>>;

// Normalize SCC partitions for correctness comparison.
Components normalize(Components components) {
    for (auto& component : components) {
        sort(component.begin(), component.end());
    }

    sort(components.begin(), components.end());
    return components;
}

// Generate a directed chain.
Graph generateChain(int n) {
    Graph graph(n);

    for (int i = 0; i < n - 1; i++) {
        graph.addEdge(i, i + 1);
    }

    return graph;
}

// Generate one large strongly connected component.
Graph generateCycle(int n) {
    Graph graph(n);

    for (int i = 0; i < n; i++) {
        graph.addEdge(i, (i + 1) % n);
    }

    return graph;
}

// Generate a reproducible sparse random directed graph.
Graph generateRandomGraph(int n) {
    Graph graph(n);

    mt19937 rng(42);
    bernoulli_distribution edgeDist(0.005);

    for (int u = 0; u < n; u++) {
        for (int v = 0; v < n; v++) {
            if (edgeDist(rng)) {
                graph.addEdge(u, v);
            }
        }
    }

    return graph;
}

// Measure average execution time.
template <typename Function>
double measureAverage(Function function, int repetitions) {
    double total = 0.0;

    for (int i = 0; i < repetitions; i++) {
        auto start = steady_clock::now();

        function();

        auto end = steady_clock::now();

        total += duration<double, micro>(end - start).count();;
    }

    return total / repetitions;
}

void benchmarkGraph(const string& name, const Graph& graph) {

    const int REPETITIONS = 3;

    TarjanSCC tarjan(graph);
    ParallelSCC parallel(graph);

    // Check correctness before measuring.
    Components sequentialResult = normalize(tarjan.run());
    Components parallelResult = normalize(parallel.run());

    if (sequentialResult != parallelResult) {
        cout << "\n" << name << ": CORRECTNESS FAILED\n";
        return;
    }

    // Measure sequential Tarjan.
    double sequentialTime = measureAverage(
        [&]() {
            TarjanSCC algorithm(graph);
            algorithm.run();
        },
        REPETITIONS
    );

    // Measure parallel SCC.
    double parallelTime = measureAverage(
        [&]() {
            ParallelSCC algorithm(graph);
            algorithm.run();
        },
        REPETITIONS
    );

    double speedup = sequentialTime / parallelTime;

    cout << "\nGraph: " << name << "\n";
    cout << "Vertices: " << graph.V << "\n";

    cout << fixed << setprecision(3);

    cout << "Tarjan time: "
     << sequentialTime << " microseconds\n";

    cout << "Parallel SCC time: "
     << parallelTime << " microseconds\n";

    cout << "Speedup: "
     << speedup << "x\n";

    cout << "Correctness: PASSED\n";
}

int main() {

    const int V = 1000;

#ifdef _OPENMP
    cout << "OpenMP threads: " << omp_get_max_threads() << "\n";
#else
    cout << "OpenMP unavailable\n";
#endif

    cout << "Vertices per graph: " << V << "\n";

    Graph chain = generateChain(V);
    Graph cycle = generateCycle(V);
    Graph randomGraph = generateRandomGraph(V);

    benchmarkGraph("Directed Chain", chain);
    benchmarkGraph("Single Large SCC (Cycle)", cycle);
    benchmarkGraph("Sparse Random Graph", randomGraph);

    return 0;
}
