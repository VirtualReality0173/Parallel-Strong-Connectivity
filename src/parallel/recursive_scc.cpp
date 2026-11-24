
#include "scc/parallel_scc.hpp"
#include "scc/pivot_scc.hpp"
#include "scc/tarjan.hpp"
#include <vector>
#include <cstdlib>

#ifdef _OPENMP
#include <omp.h>
#endif

// A recursive subproblem contains an induced graph
// and a mapping from local IDs to original graph IDs.
struct Subproblem {
    Graph graph;
    std::vector<int> originalVertices;

    Subproblem(
        const Graph& g,
        const std::vector<int>& mapping
    ) : graph(g), originalVertices(mapping) {}
};


// Construct an induced subgraph using local vertex IDs.
static Subproblem makeSubproblem(
    const Subproblem& parent,
    const std::vector<int>& vertices
) {
    int n = static_cast<int>(vertices.size());

    Graph childGraph(n);

    // Map parent-local IDs to child-local IDs.
    std::vector<int> mapping(parent.graph.V, -1);

    for (int i = 0; i < n; i++) {
        mapping[vertices[i]] = i;
    }

    // Copy only edges whose endpoints belong to this subset.
    for (int i = 0; i < n; i++) {
        int oldVertex = vertices[i];

        for (int neighbor : parent.graph.adj[oldVertex]) {
            if (mapping[neighbor] != -1) {
                childGraph.addEdge(
                    i,
                    mapping[neighbor]
                );
            }
        }
    }

    // Preserve original vertex IDs.
    std::vector<int> original(n);

    for (int i = 0; i < n; i++) {
        original[i] =
            parent.originalVertices[vertices[i]];
    }

    return Subproblem(childGraph, original);
}


static int choosePivot(const Graph& graph) {
    int bestVertex = 0;
    std::size_t bestDegree = 0;
    int bestDistance = graph.V;

    int center = graph.V / 2;

    for (int v = 0; v < graph.V; v++) {
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

    // Base case: empty graph.
    if (n == 0) {
        return result;
    }

    // Base case: one vertex.
    if (n == 1) {
        result.push_back(problem.originalVertices);
        return result;
    }

    // Sequential cutoff for small subproblems.
    const int SEQUENTIAL_CUTOFF = 32;
    if (n <= SEQUENTIAL_CUTOFF) {
    TarjanSCC tarjan(problem.graph);
    auto localComponents = tarjan.run();
    // Convert local vertex IDs to original vertex IDs.
    for (const auto& component : localComponents) {
        std::vector<int> originalComponent;
        for (int v : component) {
            originalComponent.push_back(
                problem.originalVertices[v]
            );
        }

        result.push_back(originalComponent);
    }

    return result;
}

    // Select the first local vertex as pivot.
    int pivot = choosePivot(problem.graph);

    PivotPartition partition =
        pivotSCC(problem.graph, pivot);

    // Add the pivot SCC, converting IDs back to original IDs.
    std::vector<int> pivotComponent;

    for (int v : partition.scc) {
        pivotComponent.push_back(
            problem.originalVertices[v]
        );
    }

    result.push_back(pivotComponent);

    // Construct the three independent recursive subproblems.
    Subproblem forwardProblem =
        makeSubproblem(problem, partition.forwardOnly);

    Subproblem backwardProblem =
        makeSubproblem(problem, partition.backwardOnly);

    Subproblem neitherProblem =
        makeSubproblem(problem, partition.neither);

    // Each task writes to its own result vector.
    std::vector<std::vector<int>> forwardResult;
    std::vector<std::vector<int>> backwardResult;
    std::vector<std::vector<int>> neitherResult;


#pragma omp taskgroup
{
    if (forwardProblem.graph.V > TASK_CUTOFF) {
        #pragma omp task shared(forwardResult) firstprivate(forwardProblem)
        {
            forwardResult = solveSubproblem(forwardProblem);
        }
    } else if (forwardProblem.graph.V > 0) {
        forwardResult = solveSubproblem(forwardProblem);
    }

    if (backwardProblem.graph.V > TASK_CUTOFF) {
        #pragma omp task shared(backwardResult) firstprivate(backwardProblem)
        {
            backwardResult = solveSubproblem(backwardProblem);
        }
    } else if (backwardProblem.graph.V > 0) {
        backwardResult = solveSubproblem(backwardProblem);
    }

    if (neitherProblem.graph.V > TASK_CUTOFF) {
        #pragma omp task shared(neitherResult) firstprivate(neitherProblem)
        {
            neitherResult = solveSubproblem(neitherProblem);
        }
    } else if (neitherProblem.graph.V > 0) {
        neitherResult = solveSubproblem(neitherProblem);
    }
}


    // The taskgroup guarantees all tasks have completed.
    // Merge the results.

    result.insert(
        result.end(),
        forwardResult.begin(),
        forwardResult.end()
    );

    result.insert(
        result.end(),
        backwardResult.begin(),
        backwardResult.end()
    );

    result.insert(
        result.end(),
        neitherResult.begin(),
        neitherResult.end()
    );

    return result;
}


// Public interface.
ParallelSCC::ParallelSCC(const Graph& g)
    : graph(g) {}


std::vector<std::vector<int>> ParallelSCC::run() {

    // Initially, every original vertex belongs to the problem.
    std::vector<int> vertices(graph.V);

    for (int i = 0; i < graph.V; i++) {
        vertices[i] = i;
    }

    Subproblem initial(graph, vertices);

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
