// Compile-checked pedagogical arithmetic excerpts used on the slides.
#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include "nmp/numeric_utils.hpp"
#ifdef NMP_WITH_BLAS
#include <cblas.h>
#endif
int main() {
    using nmp::require;
    {const std::vector<double> u={1,2,3},v={4,5,6};
     double sum=0.0;
     for(std::size_t i=0;i<u.size();++i) sum+=u[i]*v[i];
     require(sum==32,"C06 dot");}
    {std::vector<double> a(6,0.0);std::size_t count=a.size();double* pointer=a.data();
     require(count==6 && pointer==&a[0],"C09 storage");}
    {const std::size_t n=2,lda=2;std::vector<double>a={1,2,3,4},x={1,2},y(2,-99);
     for(std::size_t i=0;i<n;++i){double sum=0.0;for(std::size_t j=0;j<n;++j){sum+=a[i*lda+j]*x[j];}y[i]=sum;}
     require(y==std::vector<double>({5,11}),"C16 matvec");}
    {const std::size_t M=2,N=2,K=2,lda=2,ldb=2,ldc=2;
     std::vector<double>a={1,2,3,4},b=a,c(4,-99);
     for(std::size_t i=0;i<M;++i){for(std::size_t j=0;j<N;++j){double sum=0.0;for(std::size_t k=0;k<K;++k){sum+=a[i*lda+k]*b[k*ldb+j];}c[i*ldc+j]=sum;}}
     require(c==std::vector<double>({7,10,15,22}),"C35 manual square");}
    {std::ostringstream out;out<<std::setprecision(17)<<0.1;
     require(out.str()=="0.10000000000000001","R08 tested binary64 representation");
     std::cout<<"R08 binary64: "<<out.str()<<'\n';}
    {const std::vector<double>x={1e16,1,-1e16};double sum=0.0,correction=0.0;
     for(double value:x){const double next=sum+value;if(std::abs(sum)>=std::abs(value))correction+=(sum-next)+value;else correction+=(value-next)+sum;sum=next;}
     require(sum+correction==1,"R10 Neumaier");}
#ifdef NMP_WITH_BLAS
    {const int n=2,lda=2,ldc=2;std::vector<double>a={1,2,3,4},c(4,-99);
     cblas_dgemm(CblasRowMajor,CblasNoTrans,CblasNoTrans,n,n,n,1.0,a.data(),lda,a.data(),lda,0.0,c.data(),ldc);
     require(c==std::vector<double>({7,10,15,22}),"C37 dgemm");}
#endif
    std::cout<<"Pedagogical slide arithmetic checks PASS\n";
}
