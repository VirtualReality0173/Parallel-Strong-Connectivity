# Parallel-Strong-Connectivity
A parallel graph-processing project focused on implementing and evaluating pivot-based strongly connected components (SCC) algorithms using C++, OpenMP and CUDA.  

Reference Paper: 

Wang, Letong, Xiaojun Dong, Yan Gu, and Yihan Sun.
Parallel Strong Connectivity Based on Faster Reachability.
Proceedings of the ACM on Management of Data (PACMMOD), 2023.
(https://doi.org/10.1145/3589259) 

Scope: Directed static graphs. Implement a simple pivot-based forward/backward method; do not reproduce every paper optimization.

Objectives: 
1. Sequential Tarjan or Kosaraju reference and partition checker.
2. OpenMP forward/backward reachability with recursive subproblem processing.
3. One small-subproblem cutoff or reachability batching optimization.

License:
This project is licensed under the MIT License. See LICENSE for details.
Third-party dependencies and external code remain subject to their respective licenses.
