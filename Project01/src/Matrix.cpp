#include "Matrix.hpp"

#include <cblas.h>

Matrix::Matrix(size_t n, size_t m, MatrixStorageOrder order, size_t lda, double initialValue)
    : n(n), m(m), lda(lda), _order(order), _data()
{
    if (n == 0 || m == 0)
    {
        throw std::invalid_argument("Matrix dimensions must be positive");
    }

    if (lda == 0)
    {
        lda = order == MatrixStorageOrder::row_major ? m : n;
    }

    if (order == MatrixStorageOrder::row_major)
    {
        if (lda < m)
        {
            throw std::invalid_argument("Row-major LDA must be at least the number of columns");
        }
        _data.assign(n * lda, initialValue);
    }
    else
    {
        if (lda < n)
        {
            throw std::invalid_argument("Column-major LDA must be at least the number of rows");
        }
        _data.assign(m * lda, initialValue);
    }

    this->lda = lda;
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

MatrixStorageOrder Matrix::order() const
{
    return _order;
}

bool Matrix::is_row_major() const
{
    return _order == MatrixStorageOrder::row_major;
}

bool Matrix::is_column_major() const
{
    return _order == MatrixStorageOrder::column_major;
}

size_t Matrix::row_offset(size_t i, size_t j, size_t lda)
{
    return i * lda + j;
}

size_t Matrix::column_offset(size_t i, size_t j, size_t lda)
{
    return i + j * lda;
}

size_t Matrix::offset(size_t i, size_t j) const
{
    if (i >= n || j >= m)
    {
        throw std::out_of_range("Matrix indices out of range");
    }

    return is_row_major() ? row_offset(i, j, lda) : column_offset(i, j, lda);
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
    return _data[offset(i, j)];
}

const double &Matrix::operator()(size_t i, size_t j) const
{
    return _data[offset(i, j)];
}

Vector Matrix::matvec(const Vector &x) const
{
    Vector result(n, x.order());
    matvec(x, result);
    return result;
}

void Matrix::matvec(const Vector &x, Vector &y) const
{
    if (x.size() != m || y.size() != n)
    {
        throw std::invalid_argument("Matrix-vector dimensions do not match");
    }

    if (&x == &y)
    {
        throw std::invalid_argument("Matrix-vector input and output must not alias");
    }

    for (size_t i = 0; i < n; ++i)
    {
        double sum = 0.0;
        for (size_t j = 0; j < m; ++j)
        {
            sum += (*this)(i, j) * x[j];
        }
        y[i] = sum;
    }
}

Matrix Matrix::multiply_manual(const Matrix &other) const
{
    if (m != other.n)
    {
        throw std::invalid_argument("Matrix dimensions do not match for multiplication");
    }

    if (_order != other._order)
    {
        throw std::invalid_argument("Matrix multiplication requires matching storage order");
    }

    const size_t resultLda = is_row_major() ? other.m : n;

    Matrix result(n, other.m, _order, resultLda, 0.0);
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t k = 0; k < m; ++k)
        {
            const double a = (*this)(i, k);
            for (size_t j = 0; j < other.m; ++j)
            {
                result(i, j) += a * other(k, j);
            }
        }
    }
    return result;
}

Matrix Matrix::multiply_blas(const Matrix &other) const
{
    if (m != other.n)
    {
        throw std::invalid_argument("Matrix dimensions do not match for multiplication");
    }

    if (_order != other._order)
    {
        throw std::invalid_argument("BLAS multiplication requires matching matrix storage order");
    }

    const size_t resultLda = is_row_major() ? other.m : n;
    Matrix result(n, other.m, _order, resultLda, 0.0);
    const CBLAS_ORDER blasOrder = is_row_major() ? CblasRowMajor : CblasColMajor;

    cblas_dgemm(blasOrder, CblasNoTrans, CblasNoTrans,
                static_cast<int>(n), static_cast<int>(other.m), static_cast<int>(m),
                1.0, _data.data(), static_cast<int>(lda),
                other._data.data(), static_cast<int>(other.lda),
                0.0, result._data.data(), static_cast<int>(result.lda));
    return result;
}