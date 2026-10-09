#include "Matrix.hpp"

#include <algorithm>
#include <charconv>
#include <cstddef>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace
{

    struct Options
    {
        std::size_t n = 2000;
        std::size_t steps = 30;
        std::string gamma = "both";
        std::filesystem::path output = "data";
        bool overwrite = false;
    };

    std::size_t parse_size(const std::string &text)
    {
        std::size_t value = 0;
        const auto *begin = text.data();
        const auto *end = text.data() + text.size();
        const auto result = std::from_chars(begin, end, value);
        if (result.ec != std::errc() || result.ptr != end || value == 0)
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
            if (key == "--overwrite")
            {
                options.overwrite = true;
                continue;
            }
            if (key == "--help")
            {
                std::cout << "activity3 --n 2000 --steps 30 --gamma 2|3|both --output data [--overwrite]\n";
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
            else if (key == "--steps")
            {
                options.steps = parse_size(value);
            }
            else if (key == "--gamma")
            {
                options.gamma = value;
            }
            else if (key == "--output")
            {
                options.output = value;
            }
            else
            {
                throw std::invalid_argument("Unknown option: " + key);
            }
        }

        if (options.gamma != "2" && options.gamma != "3" && options.gamma != "both")
        {
            throw std::invalid_argument("--gamma must be 2, 3, or both");
        }

        return options;
    }

    void check_finite(const Vector &x)
    {
        for (std::size_t i = 0; i < x.size(); ++i)
        {
            if (!std::isfinite(x[i]))
            {
                throw std::runtime_error("Non-finite iterate encountered");
            }
        }
    }

    Matrix gamma_matrix(std::size_t n, double gamma)
    {
        if (n == 0)
        {
            throw std::invalid_argument("gamma_matrix requires n > 0");
        }

        Matrix a(n, n, MatrixStorageOrder::row_major);
        for (std::size_t i = 0; i < n; ++i)
        {
            for (std::size_t j = 0; j < n; ++j)
            {
                const double distance = static_cast<double>(i) - static_cast<double>(j);
                const double fourth_power = distance * distance * distance * distance;
                a(i, j) = 1.0 / (fourth_power + gamma);
            }
        }
        return a;
    }

    std::vector<double> norm_history(std::size_t n, std::size_t steps, double gamma)
    {
        if (steps == 0)
        {
            throw std::invalid_argument("norm_history requires steps > 0");
        }

        const Matrix a = gamma_matrix(n, gamma);
        Vector x(n, 1.0);
        Vector y(n, 0.0);
        std::vector<double> norms;
        norms.reserve(steps);

        for (std::size_t step = 0; step < steps; ++step)
        {
            a.matvec(x, y);
            check_finite(y);
            std::swap(x, y);
            norms.push_back(x.norm());
        }

        return norms;
    }

    void save_csv(const std::filesystem::path &path, const std::vector<double> &norms, bool overwrite)
    {
        if (std::filesystem::exists(path) && !overwrite)
        {
            throw std::runtime_error("Output exists: " + path.string() + " (use --overwrite)");
        }

        std::filesystem::create_directories(path.parent_path());
        std::ofstream out(path);
        if (!out)
        {
            throw std::runtime_error("Unable to open " + path.string());
        }

        out << std::setprecision(std::numeric_limits<double>::max_digits10);
        out << "iteration,norm2\n";
        for (std::size_t i = 0; i < norms.size(); ++i)
        {
            out << (i + 1) << ',' << norms[i] << '\n';
        }
    }

    void run_case(const Options &options, double gamma)
    {
        const auto norms = norm_history(options.n, options.steps, gamma);
        const std::filesystem::path file = options.output / ("norms_gamma" + std::to_string(static_cast<int>(gamma)) + ".csv");
        save_csv(file, norms, options.overwrite);
        std::cout << "saved " << file.string() << '\n';
    }

} // namespace

int main(int argc, char **argv)
{
    try
    {
        const Options options = parse_options(argc, argv);
        if (options.gamma == "2" || options.gamma == "both")
        {
            run_case(options, 2.0);
        }
        if (options.gamma == "3" || options.gamma == "both")
        {
            run_case(options, 3.0);
        }
        return 0;
    }
    catch (const std::exception &error)
    {
        std::cerr << error.what() << '\n';
        return 2;
    }
}