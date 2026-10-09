#ifndef NMP_NUMERIC_UTILS_HPP
#define NMP_NUMERIC_UTILS_HPP
// Original teaching implementation, Project 01. C++17, no numerical library required.
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
namespace nmp {
inline void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}
template<class T> void check_finite(const std::vector<T>& x) {
    for (T value : x) require(std::isfinite(value), "Non-finite input/result");
}
template<class T> T dot(const std::vector<T>& x, const std::vector<T>& y) {
    require(x.size() == y.size(), "Dot-product size mismatch");
    check_finite(x); check_finite(y);
    T sum = T(0);
    for (std::size_t i = 0; i < x.size(); ++i) sum += x[i] * y[i];
    require(std::isfinite(sum), "Non-finite dot product");
    return sum; // Empty dot product is defined as zero in this package.
}
// Scaled sum of squares avoids forming large or tiny unscaled squares.
// All arithmetic is in T; using float here is NOT double accumulation.
template<class T> T norm2(const std::vector<T>& x) {
    T scale = T(0), ssq = T(1);
    for (T value : x) {
        require(std::isfinite(value), "Non-finite norm input");
        const T a = std::abs(value);
        if (a == T(0)) continue;
        if (scale < a) {
            const T ratio = scale / a;
            ssq = T(1) + ssq * ratio * ratio;
            scale = a;
        } else {
            const T ratio = a / scale;
            ssq += ratio * ratio;
        }
    }
    const T answer = scale == T(0) ? T(0) : scale * std::sqrt(ssq);
    require(std::isfinite(answer), "Norm is not representable in this type");
    return answer;
}
struct Difference {
    double absolute = 0, relative = 0, max_absolute = 0, scale = 0;
    bool relative_defined = false;
};
inline Difference discrepancy(const std::vector<double>& x,
                              const std::vector<double>& reference) {
    require(x.size() == reference.size(), "Comparison size mismatch");
    check_finite(x); check_finite(reference);
    std::vector<double> differences(x.size());
    Difference d;
    for (std::size_t i=0; i<x.size(); ++i) {
        differences[i] = x[i] - reference[i];
        require(std::isfinite(differences[i]), "Difference overflow");
        d.max_absolute = std::max(d.max_absolute, std::abs(differences[i]));
    }
    d.absolute = norm2(differences);
    d.scale = norm2(reference);
    d.relative_defined = d.scale != 0.0;
    if (d.relative_defined) d.relative = d.absolute / d.scale;
    return d;
}
inline bool accepted(const Difference& d, double atol=1e-13, double rtol=1e-12) {
    require(std::isfinite(atol) && std::isfinite(rtol) && atol>=0 && rtol>=0,
            "Invalid tolerance");
    return std::isfinite(d.absolute) && std::isfinite(d.scale) &&
           d.absolute <= atol + rtol*d.scale;
}
inline double pairwise_sum(const std::vector<double>& x, std::size_t first,
                           std::size_t last) {
    require(first <= last && last <= x.size(), "Invalid sum range");
    if (last == first) return 0;
    if (last-first == 1) return x[first];
    const auto middle = first + (last-first)/2;
    return pairwise_sum(x,first,middle) + pairwise_sum(x,middle,last);
}
// Neumaier's compensation handles a new term larger than the running sum.
// It improves many sums but is not exact arithmetic; compile without fast-math.
inline double compensated_sum(const std::vector<double>& x) {
    check_finite(x);
    double sum=0.0, correction=0.0;
    for (double value:x) {
        const double next = sum + value;
        if (std::abs(sum) >= std::abs(value)) correction += (sum-next)+value;
        else correction += (value-next)+sum;
        sum = next;
    }
    require(std::isfinite(sum+correction), "Sum overflow");
    return sum + correction;
}
} // namespace nmp
#endif
