#ifndef PROJECT01_VECTOR_HPP
#define PROJECT01_VECTOR_HPP

#include "Matrix.hpp"
#include <initializer_list>

class Vector : public Matrix
{
public:
    Vector();

    explicit Vector(size_t n, double initialValue = 0.0);

    Vector(std::initializer_list<double> values);

    template <typename Function>
    explicit Vector(size_t n, Function f)
        : Matrix(n, 1, [f](size_t i, size_t) { return f(i); })
    {
    }

    size_t size() const;

    using Matrix::operator();
    double &operator()(size_t i);
    const double &operator()(size_t i) const;

    double &operator[](size_t i);
    const double &operator[](size_t i) const;

    double dot(const Vector &other) const;

    double norm() const;
};

#endif