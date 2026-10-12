#include "Matrix.hpp"
#include "Vector.hpp"

#include <cmath>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

namespace
{

    template <typename T>
    Matrix<T> make_gamma_matrix(std::size_t matrix_size, double gamma)
    {
        return Matrix<T>(matrix_size, matrix_size, [gamma](std::size_t row, std::size_t col) {
            const double distance = static_cast<double>(row) - static_cast<double>(col);
            return static_cast<T>(1.0 / (distance * distance * distance * distance + gamma));
        });
    }

    double relative_diff(const Vector<float> &vec_float, const Vector<double> &vec_double)
    {
        double diff_sum = 0.0;
        double ref_sum = 0.0;
        for (std::size_t i = 0; i < vec_float.size(); ++i)
        {
            const double diff = static_cast<double>(vec_float[i]) - vec_double[i];
            diff_sum += diff * diff;
            ref_sum += vec_double[i] * vec_double[i];
        }
        return std::sqrt(diff_sum) / std::sqrt(ref_sum);
    }

    double absolute_diff(const Vector<float> &vec_float, const Vector<double> &vec_double)
    {
        double diff_sum = 0.0;
        for (std::size_t i = 0; i < vec_float.size(); ++i)
        {
            const double diff = static_cast<double>(vec_float[i]) - vec_double[i];
            diff_sum += diff * diff;
        }
        return std::sqrt(diff_sum);
    }

} // namespace

int main()
{
    const std::size_t dimension = 2000;
    const std::size_t iteration_steps = 30;

    std::filesystem::create_directories("results/precision");
    std::ofstream csv("results/precision/precision_history.csv");
    csv << std::setprecision(std::numeric_limits<double>::max_digits10);
    csv << "iteration,n,gamma,arithmetic_policy,norm_float,norm_double,vector_abs_diff,vector_rel_diff,relative_defined\n";

    std::cout << "Running precision study (float vs double for gamma 2 and 3)...\n";

    for (int gamma : {2, 3})
    {
        const Matrix<double> matrix_double = make_gamma_matrix<double>(dimension, static_cast<double>(gamma));
        const Matrix<float> matrix_float(dimension, dimension, [&](std::size_t row, std::size_t col) {
            return static_cast<float>(matrix_double(row, col));
        });

        Vector<double> x_double(dimension, 1.0), y_double(dimension);
        Vector<float>  x_float(dimension, 1.0f), y_float(dimension);
        Vector<float>  x_mixed(dimension, 1.0f), y_mixed(dimension);

        for (std::size_t step = 1; step <= iteration_steps; ++step)
        {
            matrix_double.matvec(x_double, y_double);
            x_double.swap(y_double);

            // full float
            matrix_float.matvec(x_float, y_float);
            x_float.swap(y_float);

            // mixed precision (float storage, double accumulation)
            matrix_float.matvec<float, double>(x_mixed, y_mixed);
            x_mixed.swap(y_mixed);

            const double abs_diff_float = absolute_diff(x_float, x_double);
            const double rel_diff_float = relative_diff(x_float, x_double);
            const double abs_diff_mixed = absolute_diff(x_mixed, x_double);
            const double rel_diff_mixed = relative_diff(x_mixed, x_double);

            csv << step << ',' << dimension << ',' << gamma << ",float_storage_float_accum,"
                << x_float.norm() << ',' << x_double.norm() << ','
                << abs_diff_float << ',' << rel_diff_float << ",true\n";

            csv << step << ',' << dimension << ',' << gamma << ",float_storage_double_accum_float_output,"
                << x_mixed.norm() << ',' << x_double.norm() << ','
                << abs_diff_mixed << ',' << rel_diff_mixed << ",true\n";
        }
    }

    std::cout << "Precision study completed. Results written to results/precision/precision_history.csv\n";
    return 0;
}
