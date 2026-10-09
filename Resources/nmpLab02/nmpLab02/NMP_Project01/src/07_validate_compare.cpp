// Strict, BLAS-independent validator for index,value CSV vectors/flattened arrays.
#include "nmp/io.hpp"
int main(int argc,char** argv){return nmp::run_checked([&] {
    nmp::Options o(argc,argv,{"candidate","reference"});
    if(o.has("help")){std::cout<<"07_validate_compare --candidate vector.csv --reference reference.csv\n"
        "Both CSV files: index,value. Strict tolerance: atol=1e-13, rtol=1e-12.\n";return 0;}
    const auto a=nmp::read_values(o.get("candidate","")),b=nmp::read_values(o.get("reference",""));
    const auto d=nmp::discrepancy(a,b);
    std::cout<<std::setprecision(17)<<"absolute="<<d.absolute<<"; max_absolute="<<d.max_absolute
             <<"; scale="<<d.scale<<"; relative_defined="<<std::boolalpha<<d.relative_defined;
    if(d.relative_defined)std::cout<<"; relative="<<d.relative;
    const bool pass=nmp::accepted(d);std::cout<<"\n"<<(pass?"PASS":"FAIL")<<'\n';return pass?0:1;
});}
