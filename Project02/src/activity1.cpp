#include "student_support.hpp"
#include "student_tasks.hpp"
int main() {return support::run([] {
    support::check(student::dot({1,2,3},{4,5,6})==32,"Dot product check failed");
    support::check(student::norm({3,4})==5,"Norm check failed");
    support::check(student::norm({0,0})==0,"Zero norm check failed");
    // Padding example: logical 3x3 matrix, lda=5; zero-based indices.
    support::check(student::row_offset(2,1,5)==11,"Row offset check failed");
    support::check(student::col_offset(2,1,5)==7,"Column offset check failed");
    std::cout<<"Activity 1 small checks PASS; explain why each answer follows.\n";
});}
