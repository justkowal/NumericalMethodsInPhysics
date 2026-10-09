#include "Vector.hpp"

#include <cmath>

Vector::Vector() : _data(), _order(StorageOrder::column_major) {}

Vector::Vector(size_t n, double initialValue) : _data(n, initialValue), _order(StorageOrder::column_major) {}

Vector::Vector(size_t n, StorageOrder order, double initialValue) : _data(n, initialValue), _order(order) {}

Vector::Vector(std::initializer_list<double> values) : _data(values), _order(StorageOrder::column_major) {}

Vector::Vector(std::initializer_list<double> values, StorageOrder order) : _data(values), _order(order) {}

size_t Vector::size() const
{
    return _data.size();
}

size_t Vector::rows() const
{
    return is_row_major() ? 1 : size();
}

size_t Vector::cols() const
{
    return is_row_major() ? size() : 1;
}

std::vector<double> &Vector::data()
{
    return _data;
}

const std::vector<double> &Vector::data() const
{
    return _data;
}

StorageOrder Vector::order() const
{
    return _order;
}

bool Vector::is_row_major() const
{
    return _order == StorageOrder::row_major;
}

bool Vector::is_column_major() const
{
    return _order == StorageOrder::column_major;
}

size_t Vector::row_offset(size_t i, size_t j, size_t lda)
{
    return i * lda + j;
}

size_t Vector::column_offset(size_t i, size_t j, size_t lda)
{
    return i + j * lda;
}

size_t Vector::offset(size_t i) const
{
    if (i >= size())
    {
        throw std::out_of_range("Vector index out of range");
    }

    return is_row_major() ? row_offset(0, i, size()) : column_offset(i, 0, size());
}

double &Vector::operator[](size_t i)
{
    return _data[offset(i)];
}

const double &Vector::operator[](size_t i) const
{
    return _data[offset(i)];
}

double Vector::dot(const Vector &other) const
{
    if (size() != other.size())
    {
        throw std::invalid_argument("Vector sizes do not match for dot product");
    }

    double sum = 0.0;
    for (size_t i = 0; i < size(); ++i)
    {
        sum += (*this)[i] * other[i];
    }
    return sum;
}

double Vector::norm() const
{
    return std::sqrt(dot(*this));
}