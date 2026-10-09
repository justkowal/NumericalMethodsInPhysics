// OPTIONAL: full float and explicitly labelled mixed-precision accumulation.
#include "nmp/io.hpp"
template<class T> void describe(std::ostream& f,const char* name) {
    f<<name<<','<<sizeof(T)<<','<<std::numeric_limits<T>::radix<<','<<std::numeric_limits<T>::digits<<','
     <<std::numeric_limits<T>::digits10<<','<<std::numeric_limits<T>::max_digits10<<','
     <<std::setprecision(std::numeric_limits<T>::max_digits10)<<std::numeric_limits<T>::epsilon()<<','
     <<std::boolalpha<<std::numeric_limits<T>::is_iec559<<'\n';
}
int main(int argc,char** argv){return nmp::run_checked([&] {
    nmp::Options o(argc,argv,{"n","steps","gamma","output"});
    if(o.has("help")){std::cout<<"09_float_vs_double [--n 2000] [--steps 30] [--gamma both] [--output results/precision] [--overwrite]\n";return 0;}
    const auto n=o.number("n",2000),steps=o.number("steps",30,true);nmp::require(n<=4096&&steps<=10000,"Safety limit exceeded");
    const nmp::fs::path out=o.get("output","results/precision");
    nmp::prepare_files(out,{"precision_history.csv","type_properties.csv","run_metadata.json"},o.has("overwrite"));
    auto types=nmp::output_file(out/"type_properties.csv");
    types<<"type,bytes,radix,digits,digits10,max_digits10,epsilon,is_iec559\n";
    describe<float>(types,"float");describe<double>(types,"double");describe<long double>(types,"long_double");types.close();
    auto csv=nmp::output_file(out/"precision_history.csv");
    csv<<"iteration,n,gamma,arithmetic_policy,norm_float,norm_double,vector_abs_diff,vector_rel_diff,relative_defined\n";
    for(int g:nmp::gammas(o.get("gamma","both"))) {
        const auto ad=nmp::gamma_matrix(n,static_cast<double>(g));
        // Exactly the same binary64 input matrix is converted to float once.
        // Differences therefore include storage conversion and the chosen arithmetic.
        nmp::Matrix<float> af(n,n);std::transform(ad.values.begin(),ad.values.end(),af.values.begin(),[](double x){return static_cast<float>(x);});
        std::vector<double> xd(n,1),yd(n);std::vector<float> xf(n,1),yf(n),xm(n,1),ym(n);
        for(std::size_t k=1;k<=steps;++k) {
            nmp::matvec_kernel(ad,xd,yd);xd.swap(yd);nmp::check_finite(xd);
            nmp::matvec_kernel<float,float>(af,xf,yf);xf.swap(yf);nmp::check_finite(xf);
            nmp::matvec_kernel<float,double>(af,xm,ym);xm.swap(ym);nmp::check_finite(xm);
            for(int policy=0;policy<2;++policy) {
                const auto& x=policy==0?xf:xm;std::vector<double> converted(x.begin(),x.end());
                const auto d=nmp::discrepancy(converted,xd);
                csv<<k<<','<<n<<','<<g<<','<<(policy==0?"float_storage_float_accum":"float_storage_double_accum_float_output")<<','
                    <<static_cast<double>(nmp::norm2(x))<<','<<nmp::norm2(xd)<<','<<d.absolute<<',';
                if(d.relative_defined)csv<<d.relative;
                csv<<','<<(d.relative_defined?"true":"false")<<'\n';
            }
        }
    }
    csv.close();auto meta=nmp::output_file(out/"run_metadata.json");
    meta<<"{\"command\":"<<nmp::json_quote(nmp::command_line(argc,argv))
        <<",\"input_policy\":\"binary64 gamma matrix converted once to binary32\","
        <<"\"reference\":\"double recurrence, not exact arithmetic\","
        <<"\"mixed_policy\":\"float entries and iterates; double product/accumulation; cast each output to float\"}\n";
    meta.close();std::cout<<"Precision histories written; differences are not exact errors.\n";return 0;
});}
