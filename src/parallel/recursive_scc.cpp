
#include "scc/parallel_scc.hpp"
#include "scc/pivot_scc.hpp"
#include "scc/tarjan.hpp"

#include <vector>
#include <cstdlib>
#include <utility>
#include <iterator>
#include <cstddef>

#ifdef _OPENMP
#include <omp.h>
#endif

// Tuning parameters.
static const int SEQUENTIAL_CUTOFF = 256;
static const int TASK_CUTOFF = 256;


// A recursive subproblem stores its induced graph and the mapping
// from local vertex IDs to original graph vertex IDs.
struct Subproblem {
    Graph graph;
    std::vector<int> originalVertices;

    // Taking Graph by value allows child graphs to be moved into place.
    Subproblem(Graph g, std::vector<int> mapping)
        : graph(std::move(g)),
          originalVertices(std::move(mapping)) {}
};


// Construct one child graph.
//
// mapping[v] gives the child's local ID for parent vertex v.
// label[v] identifies which child contains parent vertex v.
static Subproblem makeSubproblem(
    const Subproblem& parent,
    const std::vector<int>& vertices,
    const std::vector<int>& mapping,
    const std::vector<int>& label,
    int childLabel
) {
    int n = static_cast<int>(vertices.size());

    Graph childGraph(n);
    std::vector<int> original(n);

    // Preserve original vertex IDs.
    for (int i = 0; i < n; ++i) {
        original[i] =
            parent.originalVertices[vertices[i]];
    }

    // Copy only edges whose endpoints belong to this child.
    for (int i = 0; i < n; ++i) {
        int u = vertices[i];

        for (int v : parent.graph.adj[u]) {
            if (label[v] == childLabel) {
                childGraph.addEdge(i, mapping[v]);
            }
        }
    }

    return Subproblem(
        std::move(childGraph),
        std::move(original)
    );
}


// Choose a pivot with high total degree.
// Ties are broken using distance from the local vertex-ID midpoint.
static int choosePivot(const Graph& graph) {
    int bestVertex = 0;
    std::size_t bestDegree = 0;

    int center = graph.V / 2;
    int bestDistance = graph.V;

    for (int v = 0; v < graph.V; ++v) {
        std::size_t degree =
            graph.adj[v].size() + graph.rev_adj[v].size();

        int distance = std::abs(v - center);

        if (degree > bestDegree ||
            (degree == bestDegree && distance < bestDistance)) {

            bestDegree = degree;
            bestDistance = distance;
            bestVertex = v;
        }
    }

    return bestVertex;
}


// Recursive pivot-based SCC decomposition.
static std::vector<std::vector<int>> solveSubproblem(
    const Subproblem& problem
) {
    std::vector<std::vector<int>> result;

    int n = problem.graph.V;

    // Empty graph.
    if (n == 0) {
        return result;
    }

    // Single vertex.
    if (n == 1) {
        result.push_back(problem.originalVertices);
        return result;
    }

    // Use Tarjan when the subproblem is sufficiently small.
    if (n <= SEQUENTIAL_CUTOFF) {
        TarjanSCC tarjan(problem.graph);

        auto localComponents = tarjan.run();

        for (const auto& component : localComponents) {
            std::vector<int> originalComponent;
            originalComponent.reserve(component.size());

            for (int v : component) {
                originalComponent.push_back(
                    problem.originalVertices[v]
                );
            }

            result.push_back(std::move(originalComponent));
        }

        return result;
    }

    // Select pivot and calculate its SCC partition.
    int pivot = choosePivot(problem.graph);

    PivotPartition partition =
        pivotSCC(problem.graph, pivot);

    // Add the pivot SCC, translating IDs back to original IDs.
    std::vector<int> pivotComponent;
    pivotComponent.reserve(partition.scc.size());

    for (int v : partition.scc) {
        pivotComponent.push_back(
            problem.originalVertices[v]
        );
    }

    result.push_back(std::move(pivotComponent));

    // Create one shared mapping and label array for all three children.
    std::vector<int> mapping(n, -1);
    std::vector<int> label(n, -1);

    // Child 0: forward-only vertices.
    for (int i = 0; i < static_cast<int>(
             partition.forwardOnly.size()); ++i) {
        int v = partition.forwardOnly[i];
        mapping[v] = i;
        label[v] = 0;
    }

    // Child 1: backward-only vertices.
    for (int i = 0; i < static_cast<int>(
             partition.backwardOnly.size()); ++i) {
        int v = partition.backwardOnly[i];
        mapping[v] = i;
        label[v] = 1;
    }

    // Child 2: neither forward-only nor backward-only.
    for (int i = 0; i < static_cast<int>(
             partition.neither.size()); ++i) {
        int v = partition.neither[i];
        mapping[v] = i;
        label[v] = 2;
    }

    // Each child writes to its own result vector.
    std::vector<std::vector<int>> forwardResult;
    std::vector<std::vector<int>> backwardResult;
    std::vector<std::vector<int>> neitherResult;

    // Wait for all three child computations before merging results.
    #pragma omp taskgroup
    {
        // Forward-only subproblem.
        if (!partition.forwardOnly.empty()) {
            if (static_cast<int>(partition.forwardOnly.size())
                    > TASK_CUTOFF) {

                #pragma omp task shared(forwardResult, problem, partition, mapping, label)
                {
                    Subproblem child = makeSubproblem(
                        problem,
                        partition.forwardOnly,
                        mapping,
                        label,
                        0
                    );

                    forwardResult = solveSubproblem(child);
                }
            } else {
                Subproblem child = makeSubproblem(
                    problem,
                    partition.forwardOnly,
                    mapping,
                    label,
                    0
                );

                forwardResult = solveSubproblem(child);
            }
        }

        // Backward-only subproblem.
        if (!partition.backwardOnly.empty()) {
            if (static_cast<int>(partition.backwardOnly.size())
                    > TASK_CUTOFF) {

                #pragma omp task shared(backwardResult, problem, partition, mapping, label)
                {
                    Subproblem child = makeSubproblem(
                        problem,
                        partition.backwardOnly,
                        mapping,
                        label,
                        1
                    );

                    backwardResult = solveSubproblem(child);
                }
            } else {
                Subproblem child = makeSubproblem(
                    problem,
                    partition.backwardOnly,
                    mapping,
                    label,
                    1
                );

                backwardResult = solveSubproblem(child);
            }
        }

        // Neither subproblem.
        if (!partition.neither.empty()) {
            if (static_cast<int>(partition.neither.size())
                    > TASK_CUTOFF) {

                #pragma omp task shared(neitherResult, problem, partition, mapping, label)
                {
                    Subproblem child = makeSubproblem(
                        problem,
                        partition.neither,
                        mapping,
                        label,
                        2
                    );

                    neitherResult = solveSubproblem(child);
                }
            } else {
                Subproblem child = makeSubproblem(
                    problem,
                    partition.neither,
                    mapping,
                    label,
                    2
                );

                neitherResult = solveSubproblem(child);
            }
        }
    }

    // The taskgroup guarantees that all child results are ready.
    result.insert(
        result.end(),
        std::make_move_iterator(forwardResult.begin()),
        std::make_move_iterator(forwardResult.end())
    );

    result.insert(
        result.end(),
        std::make_move_iterator(backwardResult.begin()),
        std::make_move_iterator(backwardResult.end())
    );

    result.insert(
        result.end(),
        std::make_move_iterator(neitherResult.begin()),
        std::make_move_iterator(neitherResult.end())
    );

    return result;
}


// Public interface.
ParallelSCC::ParallelSCC(const Graph& g)
    : graph(g) {}


std::vector<std::vector<int>> ParallelSCC::run() {

    // Initially, every vertex belongs to the original graph.
    std::vector<int> vertices(graph.V);

    for (int i = 0; i < graph.V; ++i) {
        vertices[i] = i;
    }

    Subproblem initial(graph, std::move(vertices));

    std::vector<std::vector<int>> result;

    // Create the OpenMP team once.
    #pragma omp parallel
    {
        #pragma omp single
        {
            result = solveSubproblem(initial);
        }
    }

    return result;
}
