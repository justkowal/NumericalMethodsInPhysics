#include "Vector.hpp"

#include <cmath>
#include <numeric>

Vector::Vector() : Matrix(0, 0) {}

Vector::Vector(size_t n, double initialValue) : Matrix(n, 1, initialValue) {}

Vector::Vector(std::initializer_list<double> values)
    : Matrix(values.size(), 1, 0.0)
{
    size_t idx = 0;
    for (double val : values)
    {
        _data[idx++] = val;
    }
}

size_t Vector::size() const
{
    return rows();
}

double &Vector::operator()(size_t i)
{
    if (i >= size())
    {
        throw std::out_of_range("Vector index out of bounds");
    }
    return _data[i];
}

const double &Vector::operator()(size_t i) const
{
    if (i >= size())
    {
        throw std::out_of_range("Vector index out of bounds");
    }
    return _data[i];
}

double &Vector::operator[](size_t i)
{
    return _data[i];
}

const double &Vector::operator[](size_t i) const
{
    return _data[i];
}

double Vector::dot(const Vector &other) const
{
    if (size() != other.size())
    {
        throw std::invalid_argument("Vectors must have the same dimension for dot product");
    }
    return std::inner_product(_data.begin(), _data.end(), other._data.begin(), 0.0);
}

double Vector::norm() const
{
    return std::sqrt(dot(*this));
}