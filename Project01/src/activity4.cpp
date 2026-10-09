#include "Matrix.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{

    struct Options
    {
        std::size_t n = 256;
        std::size_t repeats = 5;
        std::size_t warmup = 1;
        double gamma = 2.0;
    };

    std::size_t parse_size(const std::string &text)
    {
        std::size_t value = 0;
        try
        {
            value = static_cast<std::size_t>(std::stoull(text));
        }
        catch (const std::exception &)
        {
            throw std::invalid_argument("Expected a positive integer: " + text);
        }

        if (value == 0)
        {
            throw std::invalid_argument("Expected a positive integer: " + text);
        }
        return value;
    }

    Options parse_options(int argc, char **argv)
    {
        Options options;
        for (int i = 1; i < argc; ++i)
        {
            const std::string key = argv[i];
            if (key == "--help")
            {
                std::cout << "activity4 --n 256 --repeats 5 --warmup 1 --gamma 2.0\n";
                std::exit(0);
            }

            if (i + 1 >= argc)
            {
                throw std::invalid_argument("Missing value for " + key);
            }

            const std::string value = argv[++i];
            if (key == "--n")
            {
                options.n = parse_size(value);
            }
            else if (key == "--repeats")
            {
                options.repeats = parse_size(value);
            }
            else if (key == "--warmup")
            {
                options.warmup = parse_size(value);
            }
            else if (key == "--gamma")
            {
                options.gamma = std::stod(value);
            }
            else
            {
                throw std::invalid_argument("Unknown option: " + key);
            }
        }

        return options;
    }

    Matrix make_dense_matrix(std::size_t n, double gamma)
    {
        return Matrix(n, n, [gamma](std::size_t i, std::size_t j) {
            const double distance = static_cast<double>(i) - static_cast<double>(j);
            return 1.0 / (distance * distance * distance * distance + gamma);
        });
    }

    double frobenius_norm(const Matrix &m)
    {
        double sum = 0.0;
        for (double value : m.data())
        {
            sum += value * value;
        }
        return std::sqrt(sum);
    }

    double frobenius_difference(const Matrix &lhs, const Matrix &rhs)
    {
        if (lhs.rows() != rhs.rows() || lhs.cols() != rhs.cols())
        {
            throw std::invalid_argument("frobenius_difference expects same-sized matrices");
        }

        const auto &a = lhs.data();
        const auto &b = rhs.data();
        double sum = 0.0;
        for (std::size_t i = 0; i < a.size(); ++i)
        {
            const double diff = a[i] - b[i];
            sum += diff * diff;
        }
        return std::sqrt(sum);
    }

    template <typename Kernel>
    std::vector<double> benchmark(Kernel kernel, std::size_t warmup, std::size_t repeats)
    {
        for (std::size_t i = 0; i < warmup; ++i)
        {
            Matrix result = kernel();
            if (result.rows() == 0 || result.cols() == 0)
            {
                throw std::runtime_error("Unexpected empty benchmark result");
            }
        }

        std::vector<double> samples;
        samples.reserve(repeats);
        for (std::size_t i = 0; i < repeats; ++i)
        {
            const auto start = std::chrono::steady_clock::now();
            Matrix result = kernel();
            const auto stop = std::chrono::steady_clock::now();
            if (result.rows() == 0 || result.cols() == 0)
            {
                throw std::runtime_error("Unexpected empty benchmark result");
            }
            const std::chrono::duration<double, std::milli> elapsed = stop - start;
            samples.push_back(elapsed.count());
        }
        return samples;
    }

    double median(std::vector<double> values)
    {
        if (values.empty())
        {
            throw std::invalid_argument("median requires at least one sample");
        }

        std::sort(values.begin(), values.end());
        const std::size_t middle = values.size() / 2;
        if (values.size() % 2 == 1)
        {
            return values[middle];
        }
        return 0.5 * (values[middle - 1] + values[middle]);
    }

    double spread(const std::vector<double> &values)
    {
        if (values.empty())
        {
            throw std::invalid_argument("spread requires at least one sample");
        }

        const auto [min_it, max_it] = std::minmax_element(values.begin(), values.end());
        return *max_it - *min_it;
    }

    void verify_small_case()
    {
        Matrix a(2, 2);
        a(0, 0) = 1.0;
        a(0, 1) = 2.0;
        a(1, 0) = 3.0;
        a(1, 1) = 4.0;

        const Matrix manual = a.multiply_manual(a);

        const double tolerance = 1e-12;
        if (std::abs(manual(0, 0) - 7.0) > tolerance || std::abs(manual(0, 1) - 10.0) > tolerance ||
            std::abs(manual(1, 0) - 15.0) > tolerance || std::abs(manual(1, 1) - 22.0) > tolerance)
        {
            throw std::runtime_error("Manual 2x2 matrix square failed");
        }

        const Matrix blas = a.multiply_blas(a);
        if (std::abs(blas(0, 0) - 7.0) > tolerance || std::abs(blas(0, 1) - 10.0) > tolerance ||
            std::abs(blas(1, 0) - 15.0) > tolerance || std::abs(blas(1, 1) - 22.0) > tolerance)
        {
            throw std::runtime_error("CBLAS 2x2 matrix square failed");
        }
    }

} // namespace

int main(int argc, char **argv)
{
    try
    {
        const Options options = parse_options(argc, argv);
        verify_small_case();

        const Matrix input = make_dense_matrix(options.n, options.gamma);
        const Matrix manual_output = input.multiply_manual(input);

        const Matrix blas_output = input.multiply_blas(input);

        const double difference = frobenius_difference(manual_output, blas_output);
        const double reference = frobenius_norm(blas_output);
        const double tolerance = 1e-13 + 1e-12 * reference;
        std::cout << std::setprecision(std::numeric_limits<double>::max_digits10)
                  << "frobenius_difference=" << difference << " tolerance=" << tolerance << '\n';
        if (difference > tolerance)
        {
            throw std::runtime_error("Manual and BLAS results differ beyond tolerance");
        }

        const auto manual_samples = benchmark([&]()
                                              { return input.multiply_manual(input); }, options.warmup, options.repeats);
        const auto blas_samples = benchmark([&]()
                                            { return input.multiply_blas(input); }, options.warmup, options.repeats);

        std::cout << std::fixed << std::setprecision(6);
        std::cout << "manual median_ms=" << median(manual_samples) << " spread_ms=" << spread(manual_samples) << '\n';
        std::cout << "blas   median_ms=" << median(blas_samples) << " spread_ms=" << spread(blas_samples) << '\n';

        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << '\n';
        return 2;
    }
}