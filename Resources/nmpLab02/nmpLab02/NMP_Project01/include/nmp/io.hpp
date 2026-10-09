#ifndef NMP_IO_HPP
#define NMP_IO_HPP
#include "matrix.hpp"
#include <charconv>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
namespace nmp {
namespace fs=std::filesystem;
struct Options {
    std::map<std::string,std::string> values;
    std::set<std::string> flags;
    Options(int argc,char** argv,std::set<std::string> keys,
            std::set<std::string> switches={"help","overwrite","allow-large","snapshots"}) {
        for(int i=1;i<argc;++i) {
            std::string key=argv[i];
            require(key.rfind("--",0)==0,"Expected --option, received: "+key);
            key=key.substr(2);
            require(!values.count(key)&&!flags.count(key),"Duplicate option: "+key);
            if(switches.count(key)) {flags.insert(key);continue;}
            require(keys.count(key) && i+1<argc,"Unknown option or missing value: --"+key);
            values[key]=argv[++i];
        }
    }
    bool has(const std::string& key)const {return flags.count(key)!=0;}
    std::string get(const std::string& key,std::string fallback)const {
        auto it=values.find(key);return it==values.end()?fallback:it->second;
    }
    std::size_t number(const std::string& key,std::size_t fallback,bool allow_zero=false)const {
        auto it=values.find(key); if(it==values.end()) return fallback;
        const auto& s=it->second; std::size_t n=0;
        const auto r=std::from_chars(s.data(),s.data()+s.size(),n);
        require(r.ec==std::errc() && r.ptr==s.data()+s.size() && (allow_zero||n>0),"Invalid --"+key+": "+s);
        return n;
    }
};
inline std::vector<int> gammas(const std::string& text) {
    if(text=="both")return {2,3};
    if(text=="2")return {2};
    if(text=="3")return {3};
    throw std::runtime_error("--gamma must be 2, 3 or both");
}
inline std::string json_quote(const std::string& s) {
    std::ostringstream o;o<<'"';
    for(unsigned char c:s) {
        if(c=='"'||c=='\\')o<<'\\'<<c;
        else if(c=='\n')o<<"\\n";
        else if(c=='\t')o<<"\\t";
        else if(c<32)o<<'?';
        else o<<c;
    }
    o<<'"';return o.str();
}
inline std::string command_line(int argc,char** argv) {
    std::ostringstream o;for(int i=0;i<argc;++i){if(i)o<<' ';o<<argv[i];}return o.str();
}
inline void prepare_files(const fs::path& dir,const std::vector<std::string>& names,bool overwrite) {
    require(!dir.empty(),"Empty output directory");
    if(fs::exists(dir)) require(fs::is_directory(dir),"Output is not a directory");
    for(const auto& name:names)
        require(overwrite||!fs::exists(dir/name),"Refusing overwrite: "+(dir/name).string()+" (use --overwrite explicitly)");
    fs::create_directories(dir);
}
inline std::ofstream output_file(const fs::path& p) {
    std::ofstream f(p);
    if(!f)throw std::runtime_error("Cannot open output: "+p.string());
    f.exceptions(std::ios::badbit|std::ios::failbit);
    f<<std::setprecision(std::numeric_limits<double>::max_digits10);
    return f;
}
template<class T> void write_vector(const fs::path& p,const std::vector<T>& x) {
    check_finite(x);auto f=output_file(p); f<<"index,value\n";
    f<<std::setprecision(std::numeric_limits<T>::max_digits10);
    for(std::size_t i=0;i<x.size();++i) { f<<i<<','<<x[i]<<'\n'; }
    f.close();
}
inline void write_matrix(const fs::path& p,const Matrix<double>& a) {
    check_finite(a.logical());auto f=output_file(p);
    f<<"row,column,value\n";
    for(std::size_t i=0;i<a.rows;++i)for(std::size_t j=0;j<a.cols;++j)f<<i<<','<<j<<','<<a(i,j)<<'\n';
    f.close();
}
inline std::vector<double> read_values(const fs::path& p) {
    std::ifstream f(p);require(bool(f),"Cannot read: "+p.string());
    std::string line;std::getline(f,line);
    require(line=="index,value","Validator expects index,value CSV");
    std::vector<double> x;
    while(std::getline(f,line)) {
        require(!line.empty(),"Empty data row");auto comma=line.find(',');
        require(comma!=std::string::npos,"Malformed CSV row");
        std::size_t index=0;const auto r=std::from_chars(line.data(),line.data()+comma,index);
        require(r.ec==std::errc()&&r.ptr==line.data()+comma&&index==x.size(),"Nonsequential CSV index");
        std::size_t used=0;const auto value=std::stod(line.substr(comma+1),&used);
        require(used==line.size()-comma-1&&std::isfinite(value),"Invalid/non-finite CSV value");
        x.push_back(value);
    }
    require(f.eof(),"Read error");return x;
}
template<class F> int run_checked(F action) {
    try {return action();}
    catch(const std::exception& e){std::cerr<<"ERROR: "<<e.what()<<'\n';return 2;}
}
} // namespace nmp
#endif
