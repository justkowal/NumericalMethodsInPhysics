#ifndef NMP_STUDENT_SUPPORT_HPP
#define NMP_STUDENT_SUPPORT_HPP
// Allowed support: error reporting, parameter parsing and CSV I/O, NOT the
// assessed mathematical kernels. Utility code does not constitute a solution.
#include <charconv>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>
namespace support {
inline void check(bool ok,const char* message) { if(!ok) throw std::runtime_error(message); }
template<class F> int run(F f) {
    try {f();return 0;} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 2;}
}
inline std::size_t size(const std::string& s) {
    std::size_t n=0;const auto r=std::from_chars(s.data(),s.data()+s.size(),n);
    check(r.ec==std::errc() && r.ptr==s.data()+s.size() && n>0,"Expected a positive integer");return n;
}
struct Options {std::size_t n=2000,steps=30;std::string gamma="both",output="data";bool overwrite=false;};
inline Options parse(int argc,char** argv) {
    Options o;
    for(int i=1;i<argc;++i) {
        std::string key=argv[i];
        if(key=="--overwrite") {o.overwrite=true;continue;}
        check(i+1<argc,"Missing argument value");std::string value=argv[++i];
        if(key=="--n")o.n=size(value);else if(key=="--steps")o.steps=size(value);
        else if(key=="--gamma")o.gamma=value;else if(key=="--output")o.output=value;
        else throw std::runtime_error("Unknown option: "+key);
    }
    check(o.n<=2000 && o.steps<=1000,"Starter safety limit: n<=2000, steps<=1000");
    check(o.gamma=="2" || o.gamma=="3" || o.gamma=="both","Gamma must be 2, 3 or both");return o;
}
inline void save(const std::filesystem::path& path,const std::vector<double>& norms,bool overwrite) {
    check(overwrite || !std::filesystem::exists(path),"Output exists; use a new directory or explicit --overwrite");
    std::filesystem::create_directories(path.parent_path());
    std::ofstream f;f.exceptions(std::ios::badbit|std::ios::failbit);f.open(path);
    f<<std::setprecision(std::numeric_limits<double>::max_digits10)<<"iteration,norm2\n";
    for(std::size_t k=0;k<norms.size();++k) {check(std::isfinite(norms[k]),"Non-finite norm");f<<k+1<<','<<norms[k]<<'\n';}
    f.close();
}
}
#endif
