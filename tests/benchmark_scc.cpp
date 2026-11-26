
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


// Generate a directed chain of strongly connected blocks.
Graph generateSCCDAG(int blockCount, int blockSize) {

    Graph graph(blockCount * blockSize);

    for (int b = 0; b < blockCount; ++b) {

        int base = b * blockSize;

        for (int i = 0; i < blockSize; ++i) {

            int u = base + i;

            // Internal directed cycle makes each block an SCC.
            int next = base + (i + 1) % blockSize;

            graph.addEdge(u, next);

            // One-way edges to the next block.
            if (b + 1 < blockCount) {

                int nextBlockVertex =
                    (b + 1) * blockSize + i;

                graph.addEdge(u, nextBlockVertex);
            }
        }
    }

    return graph;
}

// Measure average execution time.
// Measure average execution time over a batch of repetitions.
template <typename Function>
double measureAverage(Function function, int repetitions) {

    // Warm-up run (excluded from measurement).
    function();

    auto start = steady_clock::now();

    for (int i = 0; i < repetitions; i++) {
        function();
    }

    auto end = steady_clock::now();

    return duration<double, micro>(end - start).count()
           / repetitions;
}


void benchmarkGraph(const string& name, const Graph& graph) {

    const int REPETITIONS = 50;

    TarjanSCC tarjan(graph);
    ParallelSCC parallel(graph);

    // Check correctness before measuring.
    Components sequentialResult = normalize(tarjan.run());
    Components parallelResult = normalize(parallel.run());

    if (sequentialResult != parallelResult) {

        cout << "\n" << name << ": CORRECTNESS FAILED\n";

        cout << "Tarjan SCC count: "
             << sequentialResult.size() << "\n";

        cout << "Parallel SCC count: "
             << parallelResult.size() << "\n";

        // Find the first Tarjan component missing from Parallel SCC.
        for (const auto& component : sequentialResult) {

            if (find(
                    parallelResult.begin(),
                    parallelResult.end(),
                    component
                ) == parallelResult.end()) {

                cout << "\nFirst missing component:\n";
                cout << "Size: " << component.size() << "\n";
                cout << "Vertices: ";

                for (int i = 0;
                     i < min(15, static_cast<int>(component.size()));
                     ++i) {
                    cout << component[i] << " ";
                }

                cout << "\n";
                break;
            }
        }

        // Find the first unexpected Parallel SCC component.
        for (const auto& component : parallelResult) {

            if (find(
                    sequentialResult.begin(),
                    sequentialResult.end(),
                    component
                ) == sequentialResult.end()) {

                cout << "\nFirst unexpected component:\n";
                cout << "Size: " << component.size() << "\n";
                cout << "Vertices: ";

                for (int i = 0;
                     i < min(15, static_cast<int>(component.size()));
                     ++i) {
                    cout << component[i] << " ";
                }

                cout << "\n";
                break;
            }
        }

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

    // Test Parallel SCC with different thread counts.
    const int threadCounts[] = {1, 2, 4, 8, 16, 20};

    vector<pair<int, double>> parallelTimes;

#ifdef _OPENMP

    for (int threads : threadCounts) {

        omp_set_num_threads(threads);

        double executionTime = measureAverage(
            [&]() {
                ParallelSCC algorithm(graph);
                algorithm.run();
            },
            REPETITIONS
        );

        parallelTimes.push_back({threads, executionTime});
    }

#else

    double executionTime = measureAverage(
        [&]() {
            ParallelSCC algorithm(graph);
            algorithm.run();
        },
        REPETITIONS
    );

    parallelTimes.push_back({1, executionTime});

#endif

    // One-thread Parallel SCC time is our scalability baseline.
    double oneThreadTime = parallelTimes.front().second;

    cout << fixed << setprecision(3);

    cout << "\nGraph: " << name << "\n";
    cout << "Vertices: " << graph.V << "\n";

    cout << "Tarjan time: "
         << sequentialTime << " microseconds\n";

    cout << "\nParallel SCC scaling:\n";

    for (const auto& entry : parallelTimes) {

        int threads = entry.first;
        double time = entry.second;

        cout << threads << " threads: "
             << time << " microseconds"
             << " | Parallel speedup: "
             << oneThreadTime / time
             << "x"
             << " | Speedup over Tarjan: "
             << sequentialTime / time
             << "x\n";
    }

    cout << "\nCorrectness: PASSED\n";
}


int main() {

    #ifdef _OPENMP
    omp_set_dynamic(0);
#endif

    const int V = 20000;

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
