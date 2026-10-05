// Ungraded environment check, not a solution to a course project.
#include <cblas.h>
#include <cmath>
#include <iostream>

int main() {
    const double x[] = {1,2,3};
    const double y[] = {4,5,6};
    const double d =
        cblas_ddot(3,x,1,y,1);
    constexpr double expected = 32.0;
    constexpr double tolerance = 1e-12;
    std::cout << "CBLAS dot product = " << d << " (expected 32)\n";
    if (!std::isfinite(d) || std::abs(d - expected) > tolerance) {
        std::cerr << "FAIL: unexpected dot product\n";
        return 1;
    }
    std::cout << "BLAS check PASS\n";
    return 0;
}
