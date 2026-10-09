#include "Matrix.hpp"

#include <cstddef>
#include <iomanip>
#include <iostream>
#include <utility>

int main()
{
    Matrix a(2, 2);
    a(0, 0) = 1.0;
    a(0, 1) = 2.0;
    a(1, 0) = 3.0;
    a(1, 1) = 4.0;

    Vector x{1.0, 2.0};
    Vector y(2, -99.0);

    std::cout << std::fixed << std::setprecision(6);
    a.matvec(x, y);
    std::cout << "y = [" << y[0] << ", " << y[1] << "]\n";

    std::swap(x, y);
    a.matvec(x, y);
    std::cout << "y after swap = [" << y[0] << ", " << y[1] << "]\n";

    return 0;
}