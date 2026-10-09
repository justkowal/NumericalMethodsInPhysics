#ifndef NMP_MATRIX_HPP
#define NMP_MATRIX_HPP
#include "numeric_utils.hpp"
#include <initializer_list>
#include <utility>
namespace nmp {
enum class Layout { row_major, column_major };
inline std::size_t checked_multiply(std::size_t a, std::size_t b) {
    if (b != 0 && a > std::numeric_limits<std::size_t>::max()/b)
        throw std::overflow_error("Storage-size multiplication overflow");
    return a*b;
}
template<class T=double> struct Matrix {
    std::size_t rows, cols, lda;
    Layout layout;
    std::vector<T> values;
    Matrix(std::size_t m, std::size_t n, Layout order=Layout::row_major,
           std::size_t stride=0, T fill=T(0)):
        rows(m), cols(n), lda(stride ? stride : (order==Layout::row_major?n:m)),
        layout(order) {
        require(m>0 && n>0, "Matrix dimensions must be positive");
        require(lda >= (layout==Layout::row_major?cols:rows), "Leading dimension too small");
        const auto count=checked_multiply(layout==Layout::row_major?rows:cols,lda);
        const auto bytes=checked_multiply(count,sizeof(T));
        require(bytes <= 512ULL*1024*1024, "Per-matrix safety limit: 512 MiB");
        values.assign(count,fill);
    }
    std::size_t offset(std::size_t i, std::size_t j) const {
        require(i<rows && j<cols, "Matrix index out of bounds");
        return layout==Layout::row_major ? i*lda+j : i+j*lda;
    }
    T& operator()(std::size_t i,std::size_t j) { return values[offset(i,j)]; }
    T operator()(std::size_t i,std::size_t j) const { return values[offset(i,j)]; }
    std::vector<T> logical() const {
        std::vector<T> out; out.reserve(checked_multiply(rows,cols));
        for(std::size_t i=0;i<rows;++i) for(std::size_t j=0;j<cols;++j) out.push_back((*this)(i,j));
        return out;
    }
};
inline Matrix<double> from_rows(std::size_t m,std::size_t n,
                               std::initializer_list<double> entries) {
    require(entries.size()==checked_multiply(m,n), "Initializer size mismatch");
    Matrix<double> a(m,n); a.values=entries; return a;
}
template<class T> T gamma_entry(std::size_t i,std::size_t j,T gamma) {
    require(std::isfinite(gamma) && gamma>T(0), "Gamma must be finite and positive");
    // Convert BEFORE subtraction; do not underflow an unsigned i-j.
    const T d=static_cast<T>(i)-static_cast<T>(j);
    const T d2=d*d;
    const T denominator=d2*d2+gamma;
    require(std::isfinite(denominator), "Gamma-matrix denominator overflow");
    return T(1)/denominator;
}
template<class T=double> Matrix<T> gamma_matrix(std::size_t n,T gamma) {
    require(n<=4096, "Teaching driver supports n<=4096 (memory/time safeguard)");
    Matrix<T> a(n,n);
    for(std::size_t i=0;i<n;++i)
        for(std::size_t j=0;j<n;++j) a.values[i*a.lda+j]=gamma_entry(i,j,gamma);
    return a;
}
// Preconditions are checked once, before this performance-critical kernel.
// Checked wrapper below supports both storage layouts; the row-major branch
// exposes precisely the indexing used in the beginner implementation.
template<class T, class Acc=T> void matvec_kernel(const Matrix<T>& a,
                                    const std::vector<T>& x,std::vector<T>& y) {
    for (std::size_t i=0;i<a.rows;++i) {
        Acc sum=Acc(0);
        for (std::size_t j=0;j<a.cols;++j) {
            const auto index=a.layout==Layout::row_major?i*a.lda+j:i+j*a.lda;
            sum += static_cast<Acc>(a.values[index])*static_cast<Acc>(x[j]);
        }
        y[i]=static_cast<T>(sum);
    }
}
template<class T, class Acc=T> void matvec(const Matrix<T>& a,
                            const std::vector<T>& x,std::vector<T>& y) {
    require(x.size()==a.cols && y.size()==a.rows, "Matvec dimension mismatch");
    require(&x!=&y, "Matvec input/output must not alias");
    check_finite(a.logical()); check_finite(x);
    matvec_kernel<T,Acc>(a,x,y);
    check_finite(y);
}
inline void validate_product(const Matrix<double>& a,const Matrix<double>& b,
                             const Matrix<double>& c) {
    require(a.cols==b.rows && c.rows==a.rows && c.cols==b.cols, "GEMM shape mismatch");
    require(&c!=&a && &c!=&b, "GEMM output must not alias its inputs");
    require(a.layout==b.layout && a.layout==c.layout,"GEMM layouts must agree");
    check_finite(a.logical()); check_finite(b.logical());
}
// The transparent i,j,k baseline. No allocation, validation or file I/O here.
inline void matmul_kernel(const Matrix<double>& a,const Matrix<double>& b,
                          Matrix<double>& c) {
    if(a.layout==Layout::row_major) {
        for(std::size_t i=0;i<a.rows;++i)
            for(std::size_t j=0;j<b.cols;++j) {
                double sum=0.0;
                for(std::size_t k=0;k<a.cols;++k)
                    sum+=a.values[i*a.lda+k]*b.values[k*b.lda+j];
                c.values[i*c.lda+j]=sum;
            }
    } else {
        for(std::size_t i=0;i<a.rows;++i)
            for(std::size_t j=0;j<b.cols;++j) {
                double sum=0.0;
                for(std::size_t k=0;k<a.cols;++k)
                    sum+=a.values[i+k*a.lda]*b.values[k+j*b.lda];
                c.values[i+j*c.lda]=sum;
            }
    }
}
inline void matmul(const Matrix<double>& a,const Matrix<double>& b,Matrix<double>& c) {
    validate_product(a,b,c); matmul_kernel(a,b,c); check_finite(c.logical());
}
// Optional row-major i,k,j variant; still a dense cubic algorithm.
inline void matmul_reordered_kernel(const Matrix<double>& a,const Matrix<double>& b,
                                   Matrix<double>& c) {
    for(std::size_t i=0;i<c.rows;++i)
        std::fill_n(c.values.begin()+static_cast<std::ptrdiff_t>(i*c.lda),c.cols,0.0);
    for(std::size_t i=0;i<a.rows;++i)
        for(std::size_t k=0;k<a.cols;++k) {
            const double aik=a.values[i*a.lda+k];
            for(std::size_t j=0;j<b.cols;++j) c.values[i*c.lda+j]+=aik*b.values[k*b.lda+j];
        }
}
} // namespace nmp
#endif
