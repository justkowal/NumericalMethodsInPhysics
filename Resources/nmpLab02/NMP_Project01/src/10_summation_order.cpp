#include "nmp/io.hpp"
#include <numeric>
int main(int argc,char** argv){return nmp::run_checked([&] {
    nmp::Options o(argc,argv,{});if(o.has("help")){std::cout<<"10_summation_order: strict binary64 signed examples, optional extension\n";return 0;}
    const std::vector<double> a{1e16,1,-1e16},b{1e16,-1e16,1};
    std::cout<<std::setprecision(17)<<"method,order_a,order_b,exact\n"
        <<"sequential,"<<std::accumulate(a.begin(),a.end(),0.0)<<','<<std::accumulate(b.begin(),b.end(),0.0)<<",1\n"
        <<"pairwise,"<<nmp::pairwise_sum(a,0,a.size())<<','<<nmp::pairwise_sum(b,0,b.size())<<",1\n"
        <<"Neumaier,"<<nmp::compensated_sum(a)<<','<<nmp::compensated_sum(b)<<",1\n";
    nmp::require(nmp::compensated_sum(a)==1&&nmp::compensated_sum(b)==1,"Compensation example failed (check compiler semantics)");return 0;
});}
