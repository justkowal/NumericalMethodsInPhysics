#pragma once

#include "Matrix.hpp"
#include <cmath>
#include <initializer_list>
#include <numeric>

template <typename T = double>
class Vector : public Matrix<T>
{
public:
    Vector() : Matrix<T>(0, 0) {}

    explicit Vector(size_t size, T initial_value = T()) : Matrix<T>(size, 1, initial_value) {}

    Vector(std::initializer_list<T> values)
        : Matrix<T>(values.size(), 1, T())
    {
        size_t index = 0;
        for (T val : values)
        {
            this->data_[index++] = val;
        }
    }

    template <typename Function, typename = std::enable_if_t<!std::is_arithmetic_v<Function>>>
    explicit Vector(size_t size, Function f)
        : Matrix<T>(size, 1, [f](size_t row, size_t) { return f(row); })
    {
    }

    size_t size() const
    {
        return this->rows();
    }

    using Matrix<T>::operator();
    T &operator()(size_t index)
    {
        if (index >= size())
        {
            throw std::out_of_range("Vector index out of bounds");
        }
        return this->data_[index];
    }

    const T &operator()(size_t index) const
    {
        if (index >= size())
        {
            throw std::out_of_range("Vector index out of bounds");
        }
        return this->data_[index];
    }

    T &operator[](size_t index)
    {
        return this->data_[index];
    }

    const T &operator[](size_t index) const
    {
        return this->data_[index];
    }

    void swap(Vector<T> &other) noexcept
    {
        std::swap(this->rows_, other.rows_);
        std::swap(this->cols_, other.cols_);
        std::swap(this->lda_, other.lda_);
        this->data_.swap(other.data_);
    }

    T dot(const Vector<T> &other) const
    {
        if (size() != other.size())
        {
            throw std::invalid_argument("Vectors must have the same dimension for dot product");
        }
        return std::inner_product(this->data_.begin(), this->data_.end(), other.data_.begin(), T());
    }

    double norm() const
    {
        double sum = 0.0;
        for (T val : this->data_)
        {
            const double v = static_cast<double>(val);
            sum += v * v;
        }
        return std::sqrt(sum);
    }
};