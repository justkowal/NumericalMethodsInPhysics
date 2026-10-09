#include "nmp/io.hpp"
int main(int argc,char** argv){return nmp::run_checked([&] {
    nmp::Options o(argc,argv,{});if(o.has("help")){std::cout<<"03_matrix_vector: two-step 2x2 check\n";return 0;}
    const auto a=nmp::from_rows(2,2,{1,2,3,4});
    std::vector<double> x{1,2},y(2,99);
    nmp::matvec(a,x,y); nmp::require(y==std::vector<double>({5,11}),"First step failed");
    std::cout<<"Step 1: "<<y[0]<<' '<<y[1]<<"; norm = "<<std::setprecision(17)<<nmp::norm2(y)<<'\n';
    x.swap(y); nmp::matvec(a,x,y); nmp::require(y==std::vector<double>({27,59}),"Second step failed");
    std::cout<<"Step 2: "<<y[0]<<' '<<y[1]<<"\nPASS\n";return 0;
});}
