#include "Vector.hpp"

#include <iomanip>
#include <iostream>

int main()
{
    const Vector u{1.0, 2.0, 3.0};
    const Vector v{4.0, 5.0, 6.0};
    const Vector x{3.0, 4.0};
    const Vector z{0.0, 0.0};

    std::cout << std::fixed << std::setprecision(6);
    std::cout << "dot([1,2,3],[4,5,6]) = " << u.dot(v) << '\n';
    std::cout << "norm([3,4]) = " << x.norm() << '\n';
    std::cout << "norm([0,0]) = " << z.norm() << '\n';
    std::cout << "row_offset(2,1,5) = " << Vector<>::row_offset(2, 1, 5) << '\n';
    std::cout << "col_offset(2,1,5) = " << Vector<>::column_offset(2, 1, 5) << '\n';

    return 0;
}