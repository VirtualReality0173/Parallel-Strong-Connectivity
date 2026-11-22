
#include "scc/reachability.hpp"

#include <stdexcept>

#ifdef _OPENMP
#include <omp.h>
#endif

std::vector<int> parallelBFS(
    const Graph& graph,
    int source,
    bool reverse
) {
    if (source < 0 || source >= graph.V) {
        throw std::out_of_range("Invalid BFS source");
    }

    // Select forward or reverse adjacency list.
    const auto& adj =
        reverse ? graph.rev_adj : graph.adj;

    // 0 = unvisited, 1 = visited.
    std::vector<int> visited(graph.V, 0);

    // Initial frontier.
    std::vector<int> frontier;
    frontier.push_back(source);

    visited[source] = 1;

    int maxThreads = 1;

#ifdef _OPENMP
    maxThreads = omp_get_max_threads();
#endif

    // Continue until no more vertices are reachable.
    while (!frontier.empty()) {

        // Each thread gets a private next frontier.
        std::vector<std::vector<int>> localNext(maxThreads);

        #pragma omp parallel
        {
            int tid = 0;

#ifdef _OPENMP
            tid = omp_get_thread_num();
#endif

            #pragma omp for schedule(dynamic, 1)
            for (long long i = 0;
                 i < static_cast<long long>(frontier.size());
                 i++) {

                int u = frontier[i];

                for (int v : adj[u]) {

                    int wasVisited;

                    // Atomically check and mark the vertex.
                    #pragma omp atomic capture
                    {
                        wasVisited = visited[v];
                        visited[v] = 1;
                    }

                    // Only one thread can discover v.
                    if (wasVisited == 0) {
                        localNext[tid].push_back(v);
                    }
                }
            }
        }

        // Merge thread-local frontiers.
        frontier.clear();

        for (const auto& local : localNext) {
            frontier.insert(
                frontier.end(),
                local.begin(),
                local.end()
            );
        }
    }

    return visited;
}
