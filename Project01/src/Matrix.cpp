#include "Matrix.hpp"
#include <cblas.h>

Matrix::Matrix(size_t n, size_t m, size_t lda, double initialValue)
    : n(n), m(m), lda(lda == 0 ? m : lda), _data()
{
    if (this->lda < m)
    {
        throw std::invalid_argument("Row-major LDA must be at least the number of columns");
    }
    _data.assign(n * this->lda, initialValue);
}

Matrix::Matrix(size_t n, size_t m, double initialValue)
    : Matrix(n, m, m, initialValue)
{
}

size_t Matrix::rows() const
{
    return n;
}

size_t Matrix::cols() const
{
    return m;
}

size_t Matrix::leading_dimension() const
{
    return lda;
}

size_t Matrix::row_offset(size_t i, size_t j, size_t lda)
{
    return i * lda + j;
}

size_t Matrix::column_offset(size_t i, size_t j, size_t lda)
{
    return j * lda + i;
}

size_t Matrix::offset(size_t i, size_t j) const
{
    return row_offset(i, j, lda);
}

std::vector<double> &Matrix::data()
{
    return _data;
}

const std::vector<double> &Matrix::data() const
{
    return _data;
}

double &Matrix::operator()(size_t i, size_t j)
{
    if (i >= n || j >= m)
    {
        throw std::out_of_range("Matrix index out of bounds");
    }
    return _data[offset(i, j)];
}

const double &Matrix::operator()(size_t i, size_t j) const
{
    if (i >= n || j >= m)
    {
        throw std::out_of_range("Matrix index out of bounds");
    }
    return _data[offset(i, j)];
}

void Matrix::matvec(const Vector &x, Vector &y) const
{
    if (&x == &y)
    {
        throw std::invalid_argument("Input and output vectors must not alias");
    }
    if (m != x.size())
    {
        throw std::invalid_argument("Incompatible matrix and vector dimensions");
    }
    if (y.size() != n)
    {
        y = Vector(n, 0.0);
    }

    for (size_t i = 0; i < n; ++i)
    {
        double sum = 0.0;
        const size_t row_start = i * lda;
        for (size_t j = 0; j < m; ++j)
        {
            sum += _data[row_start + j] * x[j];
        }
        y[i] = sum;
    }
}

Vector Matrix::matvec(const Vector &x) const
{
    Vector y(n, 0.0);
    matvec(x, y);
    return y;
}

Vector Matrix::operator*(const Vector &x) const
{
    return matvec(x);
}

Matrix Matrix::multiply_manual(const Matrix &other) const
{
    if (m != other.n)
    {
        throw std::invalid_argument("Incompatible dimensions for multiplication");
    }

    Matrix result(n, other.m, 0.0);
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t k = 0; k < m; ++k)
        {
            const double a_ik = _data[offset(i, k)];
            for (size_t j = 0; j < other.m; ++j)
            {
                result(i, j) += a_ik * other(k, j);
            }
        }
    }
    return result;
}

Matrix Matrix::multiply_blas(const Matrix &other) const
{
    if (m != other.n)
    {
        throw std::invalid_argument("Incompatible dimensions for multiplication");
    }

    Matrix result(n, other.m, 0.0);
    cblas_dgemm(
        CblasRowMajor,
        CblasNoTrans,
        CblasNoTrans,
        static_cast<int>(n),
        static_cast<int>(other.m),
        static_cast<int>(m),
        1.0,
        _data.data(),
        static_cast<int>(lda),
        other._data.data(),
        static_cast<int>(other.lda),
        0.0,
        result._data.data(),
        static_cast<int>(result.lda)
    );
    return result;
}