
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

    const auto& adj = reverse ? graph.rev_adj : graph.adj;

    std::vector<int> visited(graph.V, 0);
    std::vector<int> frontier;

    frontier.push_back(source);
    visited[source] = 1;

    const std::size_t PARALLEL_FRONTIER_THRESHOLD = 64;

    int maxThreads = 1;

#ifdef _OPENMP
    maxThreads = omp_get_max_threads();
#endif

    while (!frontier.empty()) {

        // Small frontier: avoid OpenMP overhead.
        if (frontier.size() < PARALLEL_FRONTIER_THRESHOLD) {

            std::vector<int> nextFrontier;

            for (int u : frontier) {
                for (int v : adj[u]) {
                    if (!visited[v]) {
                        visited[v] = 1;
                        nextFrontier.push_back(v);
                    }
                }
            }

            frontier.swap(nextFrontier);
        }

        // Large frontier: use parallel traversal.
        else {

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

                        #pragma omp atomic capture
                        {
                            wasVisited = visited[v];
                            visited[v] = 1;
                        }

                        if (wasVisited == 0) {
                            localNext[tid].push_back(v);
                        }
                    }
                }
            }

            frontier.clear();

            for (const auto& local : localNext) {
                frontier.insert(
                    frontier.end(),
                    local.begin(),
                    local.end()
                );
            }
        }
    }

    return visited;
}
