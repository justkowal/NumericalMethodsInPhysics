#ifndef PROJECT01_MATRIX_HPP
#define PROJECT01_MATRIX_HPP

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "Vector.hpp"

enum class MatrixStorageOrder
{
    row_major,
    column_major
};

class Matrix
{
protected:
    size_t n;
    size_t m;
    size_t lda;
    MatrixStorageOrder _order;
    std::vector<double> _data;

public:
    Matrix(size_t n, size_t m, MatrixStorageOrder order = MatrixStorageOrder::row_major,
           size_t lda = 0, double initialValue = 0.0);

    template <typename Function>
    Matrix(size_t n, size_t m, Function f)
        : Matrix(n, m, MatrixStorageOrder::row_major, m, 0.0)
    {
        for (size_t i = 0; i < n; ++i)
        {
            for (size_t j = 0; j < m; ++j)
            {
                _data[row_offset(i, j, lda)] = f(i, j);
            }
        }
    }

    template <typename Function>
    Matrix(size_t n, size_t m, MatrixStorageOrder order, size_t lda, Function f)
        : Matrix(n, m, order, lda, 0.0)
    {
        for (size_t i = 0; i < n; ++i)
        {
            for (size_t j = 0; j < m; ++j)
            {
                _data[offset(i, j)] = f(i, j);
            }
        }
    }

    size_t rows() const;

    size_t cols() const;

    size_t leading_dimension() const;

    MatrixStorageOrder order() const;

    bool is_row_major() const;

    bool is_column_major() const;

    static size_t row_offset(size_t i, size_t j, size_t lda);

    static size_t column_offset(size_t i, size_t j, size_t lda);

    size_t offset(size_t i, size_t j) const;

    std::vector<double> &data();

    const std::vector<double> &data() const;

    double &operator()(size_t i, size_t j);

    const double &operator()(size_t i, size_t j) const;

    Vector matvec(const Vector &x) const;

    void matvec(const Vector &x, Vector &y) const;

    Matrix multiply_manual(const Matrix &other) const;

    Matrix multiply_blas(const Matrix &other) const;
};

#endif