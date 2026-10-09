#ifndef NMP_STUDENT_TASKS_HPP
#define NMP_STUDENT_TASKS_HPP
// STUDENT STARTER. All assessed algorithms below are deliberately unfinished.
// See worksheet activities A1-A4. Missing implementations must not pass tests.
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>
#ifdef STUDENT_WITH_BLAS
#include <cblas.h>
#endif
namespace student {
[[noreturn]] inline void pending(const char* task) {
    throw std::runtime_error(std::string("NOT IMPLEMENTED: ")+task);
}
inline double dot(const std::vector<double>& u,const std::vector<double>& v) {
    (void)u;(void)v;
    // TODO A1a: validate lengths and accumulate all component products.
    pending("A1a dot");
}
inline double norm(const std::vector<double>& x) {
    (void)x;
    // TODO A1b: implement the Euclidean norm. Define your zero-vector policy.
    pending("A1b norm");
}
inline std::size_t row_offset(std::size_t i,std::size_t j,std::size_t lda) {
    (void)i;(void)j;(void)lda;
    // TODO A1c: row-major index; lda is the distance between row starts.
    pending("A1c row offset");
}
inline std::size_t col_offset(std::size_t i,std::size_t j,std::size_t lda) {
    (void)i;(void)j;(void)lda;
    // TODO A1d: column-major index; lda is the distance between column starts.
    pending("A1d column offset");
}
inline void matvec(const std::vector<double>& a,std::size_t n,
                   const std::vector<double>& x,std::vector<double>& y) {
    (void)a;(void)n;(void)x;(void)y;
    // TODO A2: packed row-major square matrix. Check shapes and distinct x/y.
    // Compute every output from the OLD input, not partially updated values.
    pending("A2 matrix-vector product");
}
inline std::vector<double> gamma_matrix(std::size_t n,double gamma) {
    (void)n;(void)gamma;
    // TODO A3a: allocate one contiguous array and fill the prescribed entries.
    // Do not subtract unsigned indices before conversion or use 32-bit d^4.
    pending("A3a gamma matrix");
}
inline std::vector<double> norm_history(std::size_t n,std::size_t steps,double gamma) {
    (void)n;(void)steps;(void)gamma;
    // TODO A3b-d: independent all-ones initial vector, TWO buffers, exactly
    // 'steps' unnormalized products, and one norm AFTER each product.
    // Return those post-step norms only; the main program handles CSV output.
    pending("A3b-d thirty-step experiment");
}
inline void square_manual(const std::vector<double>& a,std::size_t n,
                          std::vector<double>& c) {
    (void)a;(void)n;(void)c;
    // TODO A4a: triple-loop MATRIX product (not elementwise squares).
    // Verify shapes and nonaliasing; reset the accumulator for each output.
    pending("A4a manual square");
}
#ifdef STUDENT_WITH_BLAS
inline void square_blas(const std::vector<double>& a,std::size_t n,
                        std::vector<double>& c) {
    (void)a;(void)n;(void)c;
    // TODO A4b: validated n, packed row-major, no transpose, alpha=1, beta=0.
    // Call the installed cblas_dgemm declaration. Output must not alias a.
    pending("A4b CBLAS square");
}
#endif
} // namespace student
#endif
