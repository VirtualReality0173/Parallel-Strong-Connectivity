
#include "scc/reachability.hpp"

#include <vector>
#include <algorithm>

#ifdef _OPENMP
#include <omp.h>
#endif

std::vector<int> parallelBFS(
    const Graph& graph,
    int source,
    bool reverse
) {
    std::vector<int> reached;

    if (source < 0 || source >= graph.V) {
        return reached;
    }

    const std::vector<std::vector<int>>& adjacency =
        reverse ? graph.rev_adj : graph.adj;

    std::vector<int> visited(graph.V, 0);

    std::vector<int> frontier;
    frontier.push_back(source);

    visited[source] = 1;
    reached.push_back(source);

    int threadCount = 1;

#ifdef _OPENMP
    if (omp_in_parallel()) {
        threadCount = omp_get_num_threads();
    }
#endif

    while (!frontier.empty()) {

        std::vector<int> nextFrontier;

        bool useParallel =
            threadCount > 1 && frontier.size() >= 128;

        if (!useParallel) {

            // Sequential frontier processing.
            for (int u : frontier) {
                for (int v : adjacency[u]) {

                    if (!visited[v]) {
                        visited[v] = 1;
                        nextFrontier.push_back(v);
                    }
                }
            }

        }

#ifdef _OPENMP
        else {

            // Create bounded-sized tasks for the current frontier.
            const int CHUNK_SIZE = 64;

            int numTasks = std::min(
                static_cast<int>(
                    (frontier.size() + CHUNK_SIZE - 1)
                    / CHUNK_SIZE
                ),
                threadCount * 4
            );

            std::vector<std::vector<int>> localNext(numTasks);

            #pragma omp taskgroup
            {
                for (int t = 0; t < numTasks; ++t) {

                    int begin = static_cast<int>(
                        frontier.size() * t / numTasks
                    );

                    int end = static_cast<int>(
                        frontier.size() * (t + 1) / numTasks
                    );

                    #pragma omp task firstprivate(t, begin, end) \
                        shared(frontier, adjacency, visited, localNext)
                    {
                        for (int i = begin; i < end; ++i) {

                            int u = frontier[i];

                            for (int v : adjacency[u]) {

                                int oldValue;

                                #pragma omp atomic capture
                                {
                                    oldValue = visited[v];
                                    visited[v] = 1;
                                }

                                if (oldValue == 0) {
                                    localNext[t].push_back(v);
                                }
                            }
                        }
                    }
                }
            }

            // Merge the thread/task-local frontiers.
            for (int t = 0; t < numTasks; ++t) {

                nextFrontier.insert(
                    nextFrontier.end(),
                    localNext[t].begin(),
                    localNext[t].end()
                );
            }
        }
#endif

        reached.insert(
            reached.end(),
            nextFrontier.begin(),
            nextFrontier.end()
        );

        frontier.swap(nextFrontier);
    }

    return reached;
}
