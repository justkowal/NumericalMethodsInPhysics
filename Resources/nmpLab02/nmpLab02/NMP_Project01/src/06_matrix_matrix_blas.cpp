#include "nmp/io.hpp"
#include "nmp/blas_adapter.hpp"
int main(int argc,char** argv){return nmp::run_checked([&] {
    nmp::Options o(argc,argv,{"n","gamma","output"});
    if(o.has("help")){std::cout<<"06_matrix_matrix_blas [--n 128] [--gamma 2|3] [--output results/blas]\n"
        "--n greater than 512 requires --allow-large. Tiny rectangular self-check runs first.\n";return 0;}
    const auto tiny_a=nmp::from_rows(2,3,{1,2,3,4,5,6});
    const auto tiny_b=nmp::from_rows(3,2,{7,8,9,10,11,12});nmp::Matrix<double> tiny_c(2,2);
    nmp::blas_product(tiny_a,tiny_b,tiny_c);
    nmp::require(tiny_c.logical()==std::vector<double>({58,64,139,154}),"CBLAS rectangular smoke check failed");
    const auto n=o.number("n",128);nmp::require(n<=512||o.has("allow-large"),"Cubic n>512 requires --allow-large");
    const auto gs=nmp::gammas(o.get("gamma","2"));nmp::require(gs.size()==1,"Choose one gamma for a matrix dump");
    const nmp::fs::path out=o.get("output","results/blas");
    nmp::prepare_files(out,{"matrix.csv","metadata.json"},o.has("overwrite"));
    const auto a=nmp::gamma_matrix(n,static_cast<double>(gs[0]));nmp::Matrix<double> c(n,n);
    nmp::blas_product(a,a,c);nmp::write_matrix(out/"matrix.csv",c);
    auto metadata=nmp::output_file(out/"metadata.json");metadata<<"{\"n\":"<<n<<",\"gamma\":"<<gs[0]
        <<",\"method\":\"cblas_dgemm\",\"blas_integer_bytes\":"<<sizeof(nmp::blas_int)
        <<",\"dump_order\":\"logical row,column,value\"}\n";metadata.close();
    std::cout<<"PASS rectangular check; C=A*A; n="<<n<<"; Frobenius norm="<<std::setprecision(17)<<nmp::norm2(c.logical())<<'\n';return 0;
});}
