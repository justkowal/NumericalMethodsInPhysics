#include "nmp/io.hpp"
#include "nmp/blas_adapter.hpp"
int main(){return nmp::run_checked([]{
    for(auto layout:{nmp::Layout::row_major,nmp::Layout::column_major}){
        for(bool pad:{false,true}){
            nmp::Matrix<double>a(2,3,layout,pad?5:0,-333),b(3,2,layout,pad?6:0,-333),c(2,2,layout,pad?4:0,-333);
            for(std::size_t i=0;i<2;++i)for(std::size_t j=0;j<3;++j)a(i,j)=static_cast<double>(i*3+j+1);
            for(std::size_t i=0;i<3;++i)for(std::size_t j=0;j<2;++j)b(i,j)=static_cast<double>(7+i*2+j);
            nmp::blas_product(a,b,c);
            nmp::require(c.logical()==std::vector<double>({58,64,139,154}),"V09 rectangular/padded result");
            for(std::size_t p=0;p<c.values.size();++p){bool used=false;for(std::size_t i=0;i<2;++i)for(std::size_t j=0;j<2;++j)used=used||p==c.offset(i,j);if(!used)nmp::require(c.values[p]==-333,"V09 padding changed");}
        }
    }
    auto a=nmp::from_rows(2,2,{1,2,3,4}),b=nmp::from_rows(2,2,{5,6,7,8});nmp::Matrix<double>c(2,2);
    nmp::blas_product(a,a,c);nmp::require(c.logical()==std::vector<double>({7,10,15,22}),"V09 square");
    std::cout<<"V09,PASS,square and nonsymmetric rectangular in both packed/padded layouts\n";
    std::fill(c.values.begin(),c.values.end(),10);
    nmp::blas_product(a,b,c,1,0);nmp::require(c.logical()==std::vector<double>({19,22,43,50}),"V10 beta zero");
    std::fill(c.values.begin(),c.values.end(),10);
    nmp::blas_product(a,b,c,2,.5);nmp::require(c.logical()==std::vector<double>({43,49,91,105}),"V10 nonzero beta");
    const auto rectangular=nmp::from_rows(2,3,{1,2,3,4,5,6});nmp::Matrix<double>gram(3,3);
    cblas_dgemm(CblasRowMajor,CblasTrans,CblasNoTrans,3,3,2,1,rectangular.values.data(),3,rectangular.values.data(),3,0,gram.values.data(),3);
    nmp::require(gram.logical()==std::vector<double>({17,22,27,22,29,36,27,36,45}),"V10 transpose");
    std::cout<<"V10,PASS,prefilled beta0 beta0.5 alpha2 and transpose Gram product\n"
             <<"BLAS integer bytes="<<sizeof(nmp::blas_int)<<'\n';return 0;
});}
