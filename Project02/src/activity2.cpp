#include "student_support.hpp"
#include "student_tasks.hpp"
int main() {return support::run([] {
    const std::vector<double> a={1,2,3,4};std::vector<double> x={1,2},y(2,-99);
    student::matvec(a,2,x,y);
    support::check(y==std::vector<double>({5,11}),"First product failed");
    x.swap(y);student::matvec(a,2,x,y);
    support::check(y==std::vector<double>({27,59}),"Second product/reset failed");
    std::cout<<"Activity 2 two-step checks PASS; test dimensions and aliasing too.\n";
});}
