#include "nmp/io.hpp"
#include <functional>
#include <numeric>
namespace {
int failures=0;
void check(bool value,const char* message){nmp::require(value,message);}
void close(double a,double b,double atol=1e-13,double rtol=1e-12){check(std::isfinite(a)&&std::isfinite(b)&&std::abs(a-b)<=atol+rtol*std::abs(b),"Floating-point check failed");}
template<class F> void throws(F f){bool failed=false;try{f();}catch(const std::exception&){failed=true;}check(failed,"Expected controlled exception");}
template<class F> void test(const char* id,const char* name,F f) {
    try{f();std::cout<<id<<",PASS,"<<name<<'\n';}
    catch(const std::exception& e){++failures;std::cout<<id<<",FAIL,"<<e.what()<<'\n';}
}
}
int main(){
 test("V01","dot norm zero empty incompatible",[]{
    check(nmp::dot<double>({1,2,3},{4,5,6})==32,"Dot");check(nmp::norm2<double>({3,4})==5,"Norm");
    check(nmp::norm2<double>({0,0})==0&&nmp::norm2<double>({})==0&&nmp::dot<double>({},{})==0,"Zero/empty policy");
    throws([]{nmp::dot<double>({1},{1,2});});
 });
 test("V02","packed layouts n1 n2 n3 and rectangular",[]{
    for(std::size_t rows:{1U,2U,3U})for(std::size_t cols:{1U,2U,3U,4U}) {
        nmp::Matrix<double> a(rows,cols),b(rows,cols,nmp::Layout::column_major);
        for(std::size_t i=0;i<rows;++i)for(std::size_t j=0;j<cols;++j){
            a(i,j)=b(i,j)=static_cast<double>(i*cols+j+1);
            check(a.offset(i,j)==i*cols+j && b.offset(i,j)==i+j*rows,"Offset");}
        check(a.logical()==b.logical(),"Logical matrix");
    }
 });
 test("V03","padding and sentinel preservation",[]{
    for(auto layout:{nmp::Layout::row_major,nmp::Layout::column_major}){
        nmp::Matrix<double> a(2,3,layout,5,-99),b(3,2,layout,6,-99),c(2,2,layout,4,-99);
        for(std::size_t i=0;i<2;++i)for(std::size_t j=0;j<3;++j)a(i,j)=static_cast<double>(i*3+j+1);
        for(std::size_t i=0;i<3;++i)for(std::size_t j=0;j<2;++j)b(i,j)=static_cast<double>(7+i*2+j);
        nmp::matmul(a,b,c);check(c.logical()==std::vector<double>({58,64,139,154}),"Padded product");
        for(std::size_t p=0;p<c.values.size();++p){bool used=false;for(std::size_t i=0;i<2;++i)for(std::size_t j=0;j<2;++j)used=used||p==c.offset(i,j);if(!used)check(c.values[p]==-99,"Padding overwritten");}
    }
 });
 test("V04","gamma entries including distance1999",[]{
    for(double g:{2.,3.}){
        close(nmp::gamma_entry(0,0,g),1/g);close(nmp::gamma_entry(0,1,g),1/(1+g));close(nmp::gamma_entry(2,0,g),1/(16+g));
        close(nmp::gamma_entry(0,1999,g),1/(15968023992001.0+g),0,1e-15);
        check(nmp::gamma_entry(0,1999,g)==nmp::gamma_entry(1999,0,g),"Asymmetry");
        const auto a=nmp::gamma_matrix(7,g);for(std::size_t i=0;i<7;++i)for(std::size_t j=0;j<7;++j)check(a(i,j)>0&&a(i,j)==a(j,i),"Symmetry/positivity");
    }
 });
 test("V05","two steps and caught in-place bug",[]{
    const auto a=nmp::from_rows(2,2,{1,2,3,4});std::vector<double>x{1,2},y(2);
    nmp::matvec(a,x,y);check(y==std::vector<double>({5,11}),"Step1");x.swap(y);nmp::matvec(a,x,y);check(y==std::vector<double>({27,59}),"Step2");
    std::vector<double> wrong{1,2};for(std::size_t i=0;i<2;++i){double s=0;for(std::size_t j=0;j<2;++j)s+=a(i,j)*wrong[j];wrong[i]=s;}
    check(wrong==std::vector<double>({5,23})&&wrong!=std::vector<double>({5,11}),"In-place counterexample");
    throws([&]{nmp::matvec(a,x,x);});
 });
 test("V06","output reset and overwrite pseudocode defect",[]{
    const auto a=nmp::from_rows(2,2,{1,2,3,4});std::vector<double>x{1,2},y(2,77);
    nmp::matvec(a,x,y);nmp::matvec(a,x,y);check(y==std::vector<double>({5,11}),"Output must overwrite previous call");
    check(std::vector<double>({4,8})!=y,"Omitted accumulation caught");
 });
 test("V07","n1 n2 analytic n3 long-double recurrence",[]{
    for(std::size_t n:{1U,2U,3U})for(double g:{2.,3.}){
        const auto a=nmp::gamma_matrix(n,g);std::vector<double>x(n,1),y(n);
        std::vector<long double> ref(n,1),next(n);
        for(int k=1;k<=30;++k){nmp::matvec(a,x,y);x.swap(y);
            for(std::size_t i=0;i<n;++i){long double sum=0;for(std::size_t j=0;j<n;++j){const long double d=static_cast<long double>(i)-j;sum+=ref[j]/(d*d*d*d+g);}next[i]=sum;}ref.swap(next);
            for(std::size_t i=0;i<n;++i)close(x[i],static_cast<double>(ref[i]));
            if(n==1)close(x[0],std::pow(1/g,k));
            if(n==2)close(nmp::norm2(x),std::sqrt(2.)*std::pow(1/g+1/(1+g),k));
        }
    }
 });
 test("V08","AB A-square identity zero rectangular",[]{
    const auto a=nmp::from_rows(2,2,{1,2,3,4}),b=nmp::from_rows(2,2,{5,6,7,8});nmp::Matrix<double>c(2,2);
    nmp::matmul(a,b,c);check(c.logical()==std::vector<double>({19,22,43,50}),"AB");nmp::matmul(a,a,c);check(c.logical()==std::vector<double>({7,10,15,22}),"A-square");
    const auto id=nmp::from_rows(2,2,{1,0,0,1});nmp::matmul(a,id,c);check(c.logical()==a.logical(),"Identity");nmp::Matrix<double>zero(2,2);nmp::matmul(a,zero,c);check(c.logical()==zero.logical(),"Zero");
    const auto ar=nmp::from_rows(2,3,{1,2,3,4,5,6}),br=nmp::from_rows(3,2,{7,8,9,10,11,12});nmp::matmul(ar,br,c);check(c.logical()==std::vector<double>({58,64,139,154}),"Rectangular");
 });
 test("V11","metrics zero reference nonfinite equal norms",[]{
    auto d=nmp::discrepancy({1,2},{1,2});check(d.absolute==0&&d.relative_defined&&nmp::accepted(d),"Identical");
    d=nmp::discrepancy({1,0},{0,0});check(d.absolute==1&&!d.relative_defined&&!nmp::accepted(d),"Zero reference");
    d=nmp::discrepancy({1,0},{0,1});close(d.absolute,std::sqrt(2.));check(!nmp::accepted(d),"Matching norms hide disagreement");
    throws([]{nmp::discrepancy({std::numeric_limits<double>::quiet_NaN()},{0});});
    throws([]{nmp::discrepancy({0},{std::numeric_limits<double>::infinity()});});
 });
 test("V12","scaled norm large and small finite inputs",[]{
    check(std::numeric_limits<double>::max_exponent10>=300,"Requires binary64-like range");
    close(nmp::norm2<double>({1e200}),1e200,0,1e-15);close(nmp::norm2<double>({1e-300}),1e-300,0,1e-15);
    close(nmp::norm2<double>({3e200,4e200}),5e200,0,1e-15);
 });
 test("V13","shape bounds overflow and nonpositive guards",[]{
    throws([]{nmp::Matrix<double>a(0,1);});throws([]{nmp::Matrix<double>a(2,3,nmp::Layout::row_major,2);});
    throws([]{nmp::checked_multiply(std::numeric_limits<std::size_t>::max(),2);});
    throws([]{nmp::Matrix<double>a(1,1);(void)a(1,0);});throws([]{nmp::gamma_entry(0,0,0.);});
    throws([]{auto a=nmp::from_rows(2,2,{1,2,3,4});nmp::matmul(a,a,a);});
    throws([]{auto a=nmp::from_rows(2,2,{1,2,3,4});std::vector<double>x(1),y(2);nmp::matvec(a,x,y);});
 });
 std::cout<<"core_failures="<<failures<<'\n';return failures?1:0;
}
