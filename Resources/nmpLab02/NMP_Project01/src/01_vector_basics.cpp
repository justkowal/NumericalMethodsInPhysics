#include "nmp/io.hpp"
int main(int argc,char** argv) {return nmp::run_checked([&] {
    nmp::Options o(argc,argv,{});
    if(o.has("help")){std::cout<<"01_vector_basics: tiny dot/norm checks; no arguments needed\n";return 0;}
    const double product=nmp::dot<double>({1,2,3},{4,5,6});
    const double length=nmp::norm2<double>({3,4});
    nmp::require(product==32 && length==5 && nmp::norm2<double>({0,0})==0,"Example failed");
    std::cout<<"dot = "<<product<<"\nnorm(3,4) = "<<length<<"\nzero norm = 0\nPASS\n";return 0;
});}
