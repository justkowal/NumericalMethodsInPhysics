#include "Matrix.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace
{

    Matrix<double> make_dense_matrix(std::size_t n, double gamma)
    {
        return Matrix(n, n, [gamma](std::size_t i, std::size_t j) {
            const double distance = static_cast<double>(i) - static_cast<double>(j);
            return 1.0 / (distance * distance * distance * distance + gamma);
        });
    }

    template <typename Kernel>
    std::vector<double> benchmark_kernel(Kernel kernel, std::size_t warmup, std::size_t repeats)
    {
        for (std::size_t i = 0; i < warmup; ++i)
        {
            Matrix result = kernel();
            if (result.rows() == 0)
            {
                std::exit(1);
            }
        }

        std::vector<double> samples;
        samples.reserve(repeats);
        for (std::size_t i = 0; i < repeats; ++i)
        {
            const auto start = std::chrono::steady_clock::now();
            Matrix result = kernel();
            const auto stop = std::chrono::steady_clock::now();
            if (result.rows() == 0)
            {
                std::exit(1);
            }
            const std::chrono::duration<double> elapsed = stop - start;
            samples.push_back(elapsed.count());
        }
        return samples;
    }

    double median(std::vector<double> values)
    {
        std::sort(values.begin(), values.end());
        const std::size_t m = values.size() / 2;
        return values.size() % 2 == 1 ? values[m] : 0.5 * (values[m - 1] + values[m]);
    }

} // namespace

int main()
{
    const std::vector<std::size_t> sizes = {128, 256, 512};
    const std::size_t repeats = 3;
    const std::size_t warmup = 1;
    const double gamma = 2.0;

    std::cout << "Running benchmark sweep across dimensions n={128, 256, 512} for manual_ijk, manual_ikj, and cblas_dgemm...\n";

    for (std::size_t n : sizes)
    {
        const Matrix a = make_dense_matrix(n, gamma);

        const auto ijk_samples = benchmark_kernel([&]() {
            return a.multiply_manual_ijk(a);
        }, warmup, repeats);

        const auto ikj_samples = benchmark_kernel([&]() {
            return a.multiply_manual_ikj(a);
        }, warmup, repeats);

        const auto blas_samples = benchmark_kernel([&]() {
            return a.multiply_blas(a);
        }, warmup, repeats);

        const double ijk_med = median(ijk_samples);
        const double ikj_med = median(ikj_samples);
        const double blas_med = median(blas_samples);

        const std::filesystem::path dir = std::filesystem::path("results/benchmarks") / ("n" + std::to_string(n));
        std::filesystem::create_directories(dir);

        std::ofstream summary(dir / "benchmark_summary.csv");
        summary << std::setprecision(std::numeric_limits<double>::max_digits10);
        summary << "run_id,method,n,gamma,precision,repeats,median_seconds,min_seconds,max_seconds,reference_method,speedup,speedup_defined\n";

        // manual ijk
        const double min_ijk = *std::min_element(ijk_samples.begin(), ijk_samples.end());
        const double max_ijk = *std::max_element(ijk_samples.begin(), ijk_samples.end());
        summary << "n" << n << "g2,manual_ijk," << n << ",2,double," << repeats << ','
                << ijk_med << ',' << min_ijk << ',' << max_ijk << ",manual_ijk,1.0,true\n";

        // manual ikj
        const double min_ikj = *std::min_element(ikj_samples.begin(), ikj_samples.end());
        const double max_ikj = *std::max_element(ikj_samples.begin(), ikj_samples.end());
        const double speedup_ikj = ijk_med / ikj_med;
        summary << "n" << n << "g2,manual_ikj," << n << ",2,double," << repeats << ','
                << ikj_med << ',' << min_ikj << ',' << max_ikj << ",manual_ijk,"
                << speedup_ikj << ",true\n";

        // cblas dgemm
        const double min_blas = *std::min_element(blas_samples.begin(), blas_samples.end());
        const double max_blas = *std::max_element(blas_samples.begin(), blas_samples.end());
        const double speedup_blas = ijk_med / blas_med;
        summary << "n" << n << "g2,cblas_dgemm," << n << ",2,double," << repeats << ','
                << blas_med << ',' << min_blas << ',' << max_blas << ",manual_ijk,"
                << speedup_blas << ",true\n";

        std::cout << "  n=" << n
                  << " ijk=" << ijk_med * 1000.0 << "ms"
                  << ", ikj=" << ikj_med * 1000.0 << "ms (" << speedup_ikj << "x)"
                  << ", blas=" << blas_med * 1000.0 << "ms (" << speedup_blas << "x)\n";
    }

    std::cout << "Benchmark summary CSVs saved to results/benchmarks/\n";
    return 0;
}
