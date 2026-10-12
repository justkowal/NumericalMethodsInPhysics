#include "Matrix.hpp"
#include <cblas.h>

template <>
Matrix<double> Matrix<double>::multiply_blas(const Matrix<double> &other) const
{
    if (cols_ != other.rows_)
    {
        throw std::invalid_argument("Incompatible dimensions for multiplication");
    }

    Matrix<double> result(rows_, other.cols_, 0.0);
    cblas_dgemm(
        CblasRowMajor,
        CblasNoTrans,
        CblasNoTrans,
        static_cast<int>(rows_),
        static_cast<int>(other.cols_),
        static_cast<int>(cols_),
        1.0,
        data_.data(),
        static_cast<int>(lda_),
        other.data_.data(),
        static_cast<int>(other.lda_),
        0.0,
        result.data_.data(),
        static_cast<int>(result.lda_)
    );
    return result;
}

template <>
Matrix<float> Matrix<float>::multiply_blas(const Matrix<float> &other) const
{
    if (cols_ != other.rows_)
    {
        throw std::invalid_argument("Incompatible dimensions for multiplication");
    }

    Matrix<float> result(rows_, other.cols_, 0.0f);
    cblas_sgemm(
        CblasRowMajor,
        CblasNoTrans,
        CblasNoTrans,
        static_cast<int>(rows_),
        static_cast<int>(other.cols_),
        static_cast<int>(cols_),
        1.0f,
        data_.data(),
        static_cast<int>(lda_),
        other.data_.data(),
        static_cast<int>(other.lda_),
        0.0f,
        result.data_.data(),
        static_cast<int>(result.lda_)
    );
    return result;
}

// explicit template instantiations
template class Matrix<double>;
template class Matrix<float>;