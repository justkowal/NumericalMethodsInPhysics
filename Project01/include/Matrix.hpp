#pragma once

#include <cstddef>
#include <stdexcept>
#include <vector>

template <typename T>
class Vector;

template <typename T = double>
class Matrix
{
protected:
    size_t rows_;
    size_t cols_;
    size_t lda_;
    std::vector<T> data_;

public:
    Matrix() : rows_(0), cols_(0), lda_(0), data_() {}

    Matrix(size_t rows, size_t cols, T initial_value = T());

    Matrix(size_t rows, size_t cols, size_t lda, T initial_value = T());

    template <typename Function, typename = std::enable_if_t<!std::is_arithmetic_v<Function>>>
    Matrix(size_t rows, size_t cols, Function f)
        : Matrix(rows, cols, T())
    {
        for (size_t row = 0; row < rows; ++row)
        {
            for (size_t col = 0; col < cols; ++col)
            {
                data_[row_offset(row, col, lda_)] = static_cast<T>(f(row, col));
            }
        }
    }

    size_t rows() const;
    size_t cols() const;
    size_t leading_dimension() const;

    static size_t row_offset(size_t row, size_t col, size_t lda);
    static size_t column_offset(size_t row, size_t col, size_t lda);
    size_t offset(size_t row, size_t col) const;

    std::vector<T> &data();
    const std::vector<T> &data() const;

    T &operator()(size_t row, size_t col);
    const T &operator()(size_t row, size_t col) const;

    Vector<T> matvec(const Vector<T> &x) const;

    template <typename OutT = T, typename AccumT = OutT>
    void matvec(const Vector<T> &x, Vector<OutT> &y) const;

    Vector<T> operator*(const Vector<T> &x) const;

    Matrix<T> multiply_manual(const Matrix<T> &other) const;
    Matrix<T> multiply_manual_ijk(const Matrix<T> &other) const;
    Matrix<T> multiply_manual_ikj(const Matrix<T> &other) const;
    Matrix<T> multiply_blas(const Matrix<T> &other) const;
};

// deduction guide for generator callable
template <typename Function, typename = std::enable_if_t<!std::is_arithmetic_v<Function>>>
Matrix(size_t, size_t, Function) -> Matrix<double>;

// explicit specializations for blas
template <>
Matrix<double> Matrix<double>::multiply_blas(const Matrix<double> &other) const;

template <>
Matrix<float> Matrix<float>::multiply_blas(const Matrix<float> &other) const;

#include "Vector.hpp"

template <typename T>
inline Matrix<T>::Matrix(size_t rows, size_t cols, size_t lda, T initial_value)
    : rows_(rows), cols_(cols), lda_(lda == 0 ? cols : lda), data_()
{
    if (this->lda_ < cols)
    {
        throw std::invalid_argument("Row-major LDA must be at least the number of columns");
    }
    data_.assign(rows * this->lda_, initial_value);
}

template <typename T>
inline Matrix<T>::Matrix(size_t rows, size_t cols, T initial_value)
    : Matrix<T>(rows, cols, cols, initial_value)
{
}

template <typename T>
inline size_t Matrix<T>::rows() const
{
    return rows_;
}

template <typename T>
inline size_t Matrix<T>::cols() const
{
    return cols_;
}

template <typename T>
inline size_t Matrix<T>::leading_dimension() const
{
    return lda_;
}

template <typename T>
inline size_t Matrix<T>::row_offset(size_t row, size_t col, size_t lda)
{
    return row * lda + col;
}

template <typename T>
inline size_t Matrix<T>::column_offset(size_t row, size_t col, size_t lda)
{
    return col * lda + row;
}

template <typename T>
inline size_t Matrix<T>::offset(size_t row, size_t col) const
{
    return row_offset(row, col, lda_);
}

template <typename T>
inline std::vector<T> &Matrix<T>::data()
{
    return data_;
}

template <typename T>
inline const std::vector<T> &Matrix<T>::data() const
{
    return data_;
}

template <typename T>
inline T &Matrix<T>::operator()(size_t row, size_t col)
{
    if (row >= rows_ || col >= cols_)
    {
        throw std::out_of_range("Matrix index out of bounds");
    }
    return data_[offset(row, col)];
}

template <typename T>
inline const T &Matrix<T>::operator()(size_t row, size_t col) const
{
    if (row >= rows_ || col >= cols_)
    {
        throw std::out_of_range("Matrix index out of bounds");
    }
    return data_[offset(row, col)];
}

template <typename T>
template <typename OutT, typename AccumT>
inline void Matrix<T>::matvec(const Vector<T> &x, Vector<OutT> &y) const
{
    if (reinterpret_cast<const void *>(&x) == reinterpret_cast<const void *>(&y))
    {
        throw std::invalid_argument("Input and output vectors must not alias");
    }
    if (cols_ != x.size())
    {
        throw std::invalid_argument("Incompatible matrix and vector dimensions");
    }
    if (y.size() != rows_)
    {
        y = Vector<OutT>(rows_, OutT());
    }

    for (size_t row = 0; row < rows_; ++row)
    {
        AccumT sum = AccumT();
        const size_t row_start = row * lda_;
        for (size_t col = 0; col < cols_; ++col)
        {
            sum += static_cast<AccumT>(data_[row_start + col]) * static_cast<AccumT>(x[col]);
        }
        y[row] = static_cast<OutT>(sum);
    }
}

template <typename T>
inline Vector<T> Matrix<T>::matvec(const Vector<T> &x) const
{
    Vector<T> y(rows_, T());
    matvec(x, y);
    return y;
}

template <typename T>
inline Vector<T> Matrix<T>::operator*(const Vector<T> &x) const
{
    return matvec(x);
}

template <typename T>
inline Matrix<T> Matrix<T>::multiply_manual(const Matrix<T> &other) const
{
    return multiply_manual_ikj(other);
}

template <typename T>
inline Matrix<T> Matrix<T>::multiply_manual_ijk(const Matrix<T> &other) const
{
    if (cols_ != other.rows_)
    {
        throw std::invalid_argument("Incompatible dimensions for multiplication");
    }

    Matrix<T> result(rows_, other.cols_, T());
    for (size_t i = 0; i < rows_; ++i)
    {
        for (size_t j = 0; j < other.cols_; ++j)
        {
            T sum = T();
            for (size_t k = 0; k < cols_; ++k)
            {
                sum += data_[offset(i, k)] * other(k, j);
            }
            result(i, j) = sum;
        }
    }
    return result;
}

template <typename T>
inline Matrix<T> Matrix<T>::multiply_manual_ikj(const Matrix<T> &other) const
{
    if (cols_ != other.rows_)
    {
        throw std::invalid_argument("Incompatible dimensions for multiplication");
    }

    Matrix<T> result(rows_, other.cols_, T());
    for (size_t i = 0; i < rows_; ++i)
    {
        for (size_t k = 0; k < cols_; ++k)
        {
            const T a_ik = data_[offset(i, k)];
            for (size_t j = 0; j < other.cols_; ++j)
            {
                result(i, j) += a_ik * other(k, j);
            }
        }
    }
    return result;
}