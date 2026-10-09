// INSTRUCTOR REFERENCE. Controlled wall-clock measurements; no installation actions.
#include "nmp/io.hpp"
#ifdef NMP_WITH_BLAS
#include "nmp/blas_adapter.hpp"
#endif
#include <chrono>
#include <numeric>
#include <cstdlib>
namespace {
using Mat=nmp::Matrix<double>;
using Kernel=void(*)(const Mat&,const Mat&,Mat&);
struct Method {std::string name; Kernel kernel;};
double median(std::vector<double> x){std::sort(x.begin(),x.end());const auto m=x.size()/2;
    return x.size()%2?x[m]:(x[m-1]+x[m])/2;}
std::string env(const char* key){const char* v=std::getenv(key);return v?v:"unset";}
}
int main(int argc,char** argv){return nmp::run_checked([&] {
    nmp::Options o(argc,argv,{"n","gamma","repeats","output","method"}, {"help","overwrite","allow-large","dump-final"});
    if(o.has("help")){std::cout<<"08_benchmark [--n 256] [--gamma 2|3|both] [--repeats 5]\n"
        "  [--method manual|reordered|blas|both|all] [--output results/benchmarks/run]\n"
        "n>512 requires --allow-large; output collisions require --overwrite.\n"
        "--dump-final saves last matrices for independent strict checking (outside timing).\n"
        "Timing excludes allocation, matrix construction, CSV, checksum, and validation.\n";return 0;}
    const auto n=o.number("n",256),repeats=o.number("repeats",5);
    nmp::require(n<=512||o.has("allow-large"),"Cubic n>512 requires explicit --allow-large");
    nmp::require(repeats<=1000,"Repeat safety limit exceeded");
#ifdef NMP_WITH_BLAS
    const auto requested=o.get("method","both");
#else
    const auto requested=o.get("method","manual");
#endif
    nmp::require(requested=="manual"||requested=="reordered"||requested=="blas"||requested=="both"||requested=="all","Unknown method");
    std::vector<Method> methods;
    if(requested=="manual"||requested=="both"||requested=="all")methods.push_back({"manual_ijk",nmp::matmul_kernel});
    if(requested=="reordered"||requested=="all")methods.push_back({"manual_ikj",nmp::matmul_reordered_kernel});
    if(requested=="blas"||requested=="both"||requested=="all") {
#ifdef NMP_WITH_BLAS
        methods.push_back({"cblas_dgemm",nmp::blas_kernel});
#else
        throw std::runtime_error("SKIPPED-DEPENDENCY: this binary has no BLAS; build make blas");
#endif
    }
    const auto cases=nmp::gammas(o.get("gamma","2"));const nmp::fs::path out=o.get("output","results/benchmarks/run");
    std::vector<std::string> planned{"benchmark_raw.csv","benchmark_summary.csv","validation.csv","run_metadata.json"};
    if(o.has("dump-final")) for(int g:cases) for(const auto& method:methods) planned.push_back("final_"+method.name+"_gamma"+std::to_string(g)+".csv");
    nmp::prepare_files(out,planned,o.has("overwrite"));
    auto raw=nmp::output_file(out/"benchmark_raw.csv"),summary=nmp::output_file(out/"benchmark_summary.csv"),checks=nmp::output_file(out/"validation.csv");
    raw<<"run_id,method,n,gamma,precision,repeat,elapsed_seconds,checksum,validation_status\n";
    summary<<"run_id,method,n,gamma,precision,repeats,median_seconds,min_seconds,max_seconds,reference_method,speedup,speedup_defined\n";
    checks<<"gamma,method,repeat,abs_frobenius,rel_frobenius,max_abs,pass\n";
    for(int g:cases) {
        const auto a=nmp::gamma_matrix(n,static_cast<double>(g));Mat reference(n,n),c(n,n);
        nmp::matmul(a,a,reference); // untimed baseline and immutable comparison target
        const auto ref=reference.logical();
        std::vector<std::vector<double>> samples(methods.size());
        for(const auto& method:methods) {method.kernel(a,a,c);nmp::require(nmp::accepted(nmp::discrepancy(c.logical(),ref)),"Warm-up validation failed");}
        // Alternate method order to reduce a fixed first/last ordering bias.
        for(std::size_t r=0;r<repeats;++r)for(std::size_t j=0;j<methods.size();++j) {
            const auto index=r%2?methods.size()-1-j:j;const auto& method=methods[index];
            // Volatile function-pointer load prevents assuming a fixed pure call
            // across repeats. Arithmetic arrays themselves are NOT volatile.
            Kernel volatile selected=method.kernel;
            const auto start=std::chrono::steady_clock::now();selected(a,a,c);
            const auto stop=std::chrono::steady_clock::now();
            const double seconds=std::chrono::duration<double>(stop-start).count();
            nmp::require(std::isfinite(seconds)&&seconds>0,"Timer did not resolve positive interval");
            const auto logical=c.logical();const auto d=nmp::discrepancy(logical,ref);
            const bool valid=nmp::accepted(d);checks<<g<<','<<method.name<<','<<r+1<<','<<d.absolute<<',';
            if(d.relative_defined)checks<<d.relative;
            checks<<','<<d.max_absolute<<','<<(valid?"true":"false")<<'\n';
            nmp::require(valid,"Timed output failed validation: "+method.name);
            const double checksum=std::accumulate(logical.begin(),logical.end(),0.0);
            nmp::require(std::isfinite(checksum),"Non-finite output checksum");
            raw<<"n"<<n<<"g"<<g<<','<<method.name<<','<<n<<','<<g<<",double,"<<r+1<<','<<seconds<<','<<checksum<<",PASS\n";
            samples[index].push_back(seconds);
            if(o.has("dump-final") && r+1==repeats) nmp::write_matrix(out/("final_"+method.name+"_gamma"+std::to_string(g)+".csv"),c);
        }
        double baseline=0;for(std::size_t j=0;j<methods.size();++j)if(methods[j].name=="manual_ijk")baseline=median(samples[j]);
        for(std::size_t j=0;j<methods.size();++j) {
            const double med=median(samples[j]);summary<<"n"<<n<<"g"<<g<<','<<methods[j].name<<','<<n<<','<<g<<",double,"<<repeats<<','<<med<<','
                <<*std::min_element(samples[j].begin(),samples[j].end())<<','<<*std::max_element(samples[j].begin(),samples[j].end())<<",manual_ijk,";
            if(baseline>0)summary<<baseline/med;
            summary<<','<<(baseline>0?"true":"false")<<'\n';
            std::cout<<"n="<<n<<", gamma="<<g<<", "<<methods[j].name<<", median="<<med<<" s, PASS\n";
        }
    }
    raw.close();summary.close();checks.close();auto meta=nmp::output_file(out/"run_metadata.json");
    meta<<"{\n\"command\":"<<nmp::json_quote(nmp::command_line(argc,argv))<<",\n\"compiler\":"<<nmp::json_quote(__VERSION__)
        <<",\n\"OPENBLAS_NUM_THREADS\":"<<nmp::json_quote(env("OPENBLAS_NUM_THREADS"))
        <<",\n\"OMP_NUM_THREADS\":"<<nmp::json_quote(env("OMP_NUM_THREADS"))
        <<",\n\"effective_threads\":\"unknown unless externally observed\",\n\"timer\":\"steady_clock seconds\","
        <<"\n\"timed_region\":\"one complete overwriting kernel including its intrinsic zeroing; excludes validation allocation I/O\","
        <<"\n\"warmup\":1,\"n\":"<<n<<",\"repeats\":"<<repeats<<",\"large_opt_in\":"<<(o.has("allow-large")?"true":"false")<<"\n}\n";
    meta.close();return 0;
});}
