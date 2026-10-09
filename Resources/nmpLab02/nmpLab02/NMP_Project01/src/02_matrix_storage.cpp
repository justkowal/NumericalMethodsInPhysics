#include "nmp/io.hpp"
int main(int argc,char** argv){return nmp::run_checked([&] {
    nmp::Options o(argc,argv,{});if(o.has("help")){std::cout<<"02_matrix_storage: packed and padded 3x3 layouts\n";return 0;}
    for(auto layout:{nmp::Layout::row_major,nmp::Layout::column_major}) {
        for(std::size_t stride:{3U,5U}) {
            nmp::Matrix<double> a(3,3,layout,stride,-99);
            for(std::size_t i=0;i<3;++i)for(std::size_t j=0;j<3;++j)a(i,j)=static_cast<double>(3*i+j+1);
            std::cout<<(layout==nmp::Layout::row_major?"row-major":"column-major")<<", lda="<<stride<<'\n';
            for(std::size_t i=0;i<3;++i) {
                for(std::size_t j=0;j<3;++j)std::cout<<"A["<<i<<','<<j<<"]="<<a(i,j)<<" @ "<<a.offset(i,j)<<"   ";
                std::cout<<'\n';
            }
            std::cout<<"Storage (-99 denotes padding): ";for(double v:a.values)std::cout<<v<<' ';std::cout<<'\n';
        }
    }
    return 0;
});}
