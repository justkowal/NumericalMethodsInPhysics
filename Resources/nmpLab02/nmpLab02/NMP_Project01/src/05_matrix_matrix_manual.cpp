#include "nmp/io.hpp"
int main(int argc,char** argv){return nmp::run_checked([&] {
    nmp::Options o(argc,argv,{"n","gamma","output"});
    if(o.has("help")){std::cout<<"05_matrix_matrix_manual [--n 128] [--gamma 2|3] [--output results/manual]\n"
        "--n greater than 512 requires --allow-large. Existing files require --overwrite.\n";return 0;}
    const auto n=o.number("n",128);nmp::require(n<=512||o.has("allow-large"),"Cubic n>512 requires --allow-large");
    const auto gs=nmp::gammas(o.get("gamma","2"));nmp::require(gs.size()==1,"Choose one gamma for a matrix dump");
    const nmp::fs::path out=o.get("output","results/manual");
    nmp::prepare_files(out,{"matrix.csv","metadata.json"},o.has("overwrite"));
    const auto a=nmp::gamma_matrix(n,static_cast<double>(gs[0]));nmp::Matrix<double> c(n,n);
    nmp::matmul(a,a,c);nmp::write_matrix(out/"matrix.csv",c);
    auto metadata=nmp::output_file(out/"metadata.json");metadata<<"{\"n\":"<<n<<",\"gamma\":"<<gs[0]
        <<",\"method\":\"manual_ijk\",\"dump_order\":\"logical row,column,value\"}\n";metadata.close();
    std::cout<<"C=A*A; n="<<n<<"; Frobenius norm="<<std::setprecision(17)<<nmp::norm2(c.logical())<<'\n';return 0;
});}
