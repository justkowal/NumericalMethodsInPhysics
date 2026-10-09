#ifndef NMP_BLAS_ADAPTER_HPP
#define NMP_BLAS_ADAPTER_HPP
#include "matrix.hpp"
#include <cblas.h>
#include <tuple>
#include <type_traits>
namespace nmp {
// Derive the actual API integer type from the installed cblas_dgemm prototype.
// This avoids guessing whether this header uses int, int32_t or a wider type.
template<class> struct function_arguments;
template<class R,class... Args> struct function_arguments<R(*)(Args...)> {using tuple=std::tuple<Args...>;};
using blas_int=std::remove_cv_t<std::tuple_element_t<3,typename function_arguments<decltype(&cblas_dgemm)>::tuple>>;
inline blas_int to_blas_int(std::size_t n) {
    require(n<=static_cast<std::size_t>(std::numeric_limits<blas_int>::max()),"Dimension exceeds installed BLAS integer range");
    return static_cast<blas_int>(n);
}
inline void blas_kernel(const Matrix<double>& a,const Matrix<double>& b,Matrix<double>& c) {
    cblas_dgemm(a.layout==Layout::row_major?CblasRowMajor:CblasColMajor,
                CblasNoTrans,CblasNoTrans,to_blas_int(a.rows),to_blas_int(b.cols),
                to_blas_int(a.cols),1.0,a.values.data(),to_blas_int(a.lda),
                b.values.data(),to_blas_int(b.lda),0.0,c.values.data(),to_blas_int(c.lda));
}
inline void blas_product(const Matrix<double>& a,const Matrix<double>& b,
                         Matrix<double>& c,double alpha=1,double beta=0) {
    validate_product(a,b,c);
    require(std::isfinite(alpha)&&std::isfinite(beta),"Non-finite GEMM scalar");
    if(beta!=0)check_finite(c.logical());
    cblas_dgemm(a.layout==Layout::row_major?CblasRowMajor:CblasColMajor,
                CblasNoTrans,CblasNoTrans,to_blas_int(a.rows),to_blas_int(b.cols),
                to_blas_int(a.cols),alpha,a.values.data(),to_blas_int(a.lda),
                b.values.data(),to_blas_int(b.lda),beta,c.values.data(),to_blas_int(c.lda));
    check_finite(c.logical());
}
} // namespace nmp
#endif
