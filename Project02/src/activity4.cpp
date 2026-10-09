#include "student_support.hpp"
#include "student_tasks.hpp"
#include <chrono>
int main() {return support::run([] {
    const std::vector<double> a={1,2,3,4},expected={7,10,15,22};
    std::vector<double> c(4,-99);student::square_manual(a,2,c);
    support::check(c==expected,"Manual matrix-square check failed");
    std::cout<<"Manual tiny square PASS.\n";
#ifdef STUDENT_WITH_BLAS
    std::vector<double> b(4,-99);student::square_blas(a,2,b);
    support::check(b==expected,"CBLAS matrix-square check failed");
    std::cout<<"CBLAS tiny square PASS.\n";
#else
    std::cout<<"CBLAS NOT BUILT: manual-only preparation does not complete the library task.\n";
#endif
    // TODO A4c-d: extend to the official gamma matrix, full-array discrepancy,
    // and steady_clock timings. Small tests precede any large cubic run.
    // Keep validation and output outside the timed kernel. Record repeats,
    // compiler flags, gamma, n, actual BLAS implementation and thread policy.
    std::cout<<"Large experiment and timing/report stages still need your implementation.\n";
});}
