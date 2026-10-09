// INSTRUCTOR REFERENCE: the complete assessed driver. Not a student starter.
#include "nmp/io.hpp"
int main(int argc,char** argv){return nmp::run_checked([&] {
    nmp::Options o(argc,argv,{"n","steps","gamma","output"});
    if(o.has("help")) {
        std::cout<<"04_gamma_iterations [--n 2000] [--steps 30] [--gamma 2|3|both]\n"
                    "  [--output results/official] [--snapshots] [--overwrite]\n"
                    "Unnormalized matvec only. No cubic multiplication. n<=4096, steps<=10000.\n";
        return 0;
    }
    const auto n=o.number("n",2000),steps=o.number("steps",30,true);
    nmp::require(n<=4096 && steps<=10000,"Dimension/step safety limit exceeded");
    const auto cases=nmp::gammas(o.get("gamma","both"));
    const nmp::fs::path out=o.get("output","results/official");
    std::vector<std::string> files{"run_metadata.json"};
    for(int g:cases) {
        const auto tag=std::to_string(g);
        files.push_back("norms_gamma"+tag+".csv"); files.push_back("final_vector_gamma"+tag+".csv");
        if(o.has("snapshots")) for(std::size_t k=1;k<=steps;++k)
            files.push_back("step_gamma"+tag+"_"+std::to_string(k)+".csv");
    }
    nmp::prepare_files(out,files,o.has("overwrite"));
    for(int g:cases) {
        const auto a=nmp::gamma_matrix(n,static_cast<double>(g));
        nmp::check_finite(a.values);
        std::vector<double> x(n,1.0),y(n,0.0); // reset independently for each gamma
        const auto tag=std::to_string(g);
        auto csv=nmp::output_file(out/("norms_gamma"+tag+".csv"));
        csv<<"iteration,norm2\n";
        for(std::size_t k=1;k<=steps;++k) {
            // Dimensions and distinct buffers are fixed by construction above.
            nmp::matvec_kernel(a,x,y);
            nmp::check_finite(y);
            x.swap(y); // previous input is now available as the next output buffer
            const double length=nmp::norm2(x);
            csv<<k<<','<<length<<'\n';
            if(o.has("snapshots"))nmp::write_vector(out/("step_gamma"+tag+"_"+std::to_string(k)+".csv"),x);
        }
        csv.close();nmp::write_vector(out/("final_vector_gamma"+tag+".csv"),x);
        std::cout<<"gamma="<<g<<", n="<<n<<", steps="<<steps
                 <<", final norm="<<std::setprecision(17)<<nmp::norm2(x)<<'\n';
    }
    auto metadata=nmp::output_file(out/"run_metadata.json");
    metadata<<"{\n  \"command\": "<<nmp::json_quote(nmp::command_line(argc,argv))
        <<",\n  \"n\": "<<n<<", \"lda\": "<<n<<", \"steps\": "<<steps
        <<",\n  \"gamma\": "<<nmp::json_quote(o.get("gamma","both"))
        <<",\n  \"layout\": \"row-major\", \"initial_value\": 1, \"normalized\": false,"
        <<"\n  \"arithmetic\": \"double storage and accumulation; scaled norm\","
        <<"\n  \"sizeof_double\": "<<sizeof(double)<<", \"digits\": "<<std::numeric_limits<double>::digits
        <<", \"max_digits10\": "<<std::numeric_limits<double>::max_digits10
        <<",\n  \"compiler\": "<<nmp::json_quote(__VERSION__)
        <<",\n  \"machine_category\": \"record externally; do not infer target machine\"\n}\n";
    metadata.close();return 0;
});}
