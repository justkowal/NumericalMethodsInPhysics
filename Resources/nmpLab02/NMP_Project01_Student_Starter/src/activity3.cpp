#include "student_support.hpp"
#include "student_tasks.hpp"
int main(int argc,char** argv) {return support::run([&] {
    if(argc==2 && std::string(argv[1])=="--help") {
        std::cout<<"activity3 --n 2000 --steps 30 --gamma 2|3|both --output data [--overwrite]\n";return;
    }
    auto o=support::parse(argc,argv);
    for(int gamma:{2,3}) {
        if(o.gamma!="both" && o.gamma!=std::to_string(gamma))continue;
        auto norms=student::norm_history(o.n,o.steps,static_cast<double>(gamma));
        support::check(norms.size()==o.steps,"Incorrect post-step row count");
        auto path=std::filesystem::path(o.output)/("norms_gamma"+std::to_string(gamma)+".csv");
        support::save(path,norms,o.overwrite);
        std::cout<<"Saved "<<path<<"; validate the values independently.\n";
    }
});}
