// Ungraded environment check. Requires an actual LAPACKE development package.
#include <lapacke.h>
#include <algorithm>
#include <cmath>
#include <iostream>

int main() {
    // Row-major 2x2 storage. LAPACKE_dgesv overwrites a and b.
    double a[] = {3.0, 1.0, 1.0, 2.0};
    double b[] = {9.0, 8.0};
    lapack_int pivot[2] = {0, 0};
    const lapack_int info = LAPACKE_dgesv(
        LAPACK_ROW_MAJOR, 2, 1, a, 2, pivot, b, 1);
    if (info != 0) {
        std::cerr << "FAIL: LAPACKE_dgesv info = " << info
                  << " (negative: argument/interface issue; positive: singular U)\n";
        return 1;
    }
    if (!std::isfinite(b[0]) || !std::isfinite(b[1])) {
        std::cerr << "FAIL: non-finite solution\n";
        return 1;
    }
    // Check against the original coefficients, not the overwritten arrays.
    const double r0 = 3.0*b[0] + b[1] - 9.0;
    const double r1 = b[0] + 2.0*b[1] - 8.0;
    const double residual = std::hypot(r0, r1);
    const double error = std::max(std::abs(b[0]-2.0), std::abs(b[1]-3.0));
    std::cout << "Solution = (" << b[0] << ", " << b[1] << ") (expected 2, 3)\n"
              << "Residual 2-norm = " << residual << '\n';
    if (!std::isfinite(residual) || residual > 1e-12 || error > 1e-12) {
        std::cerr << "FAIL: solution/residual outside tolerance\n";
        return 1;
    }
    std::cout << "LAPACKE check PASS\n";
    return 0;
}
