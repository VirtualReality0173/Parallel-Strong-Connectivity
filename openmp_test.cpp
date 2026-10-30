#include <iostream>
#include <omp.h>

int main() {
#ifdef _OPENMP
    std::cout << "OpenMP version: " << _OPENMP << std::endl;
#else
    std::cout << "OpenMP is not enabled" << std::endl;
#endif
    std::cout << "Maximum threads: " << omp_get_max_threads() << std::endl;
    return 0;
}
