#ifndef PROJECT01_VECTOR_HPP
#define PROJECT01_VECTOR_HPP

#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <vector>

enum class StorageOrder
{
    row_major,
    column_major
};

class Vector
{
protected:
    std::vector<double> _data;
    StorageOrder _order = StorageOrder::column_major;

public:
    Vector();

    explicit Vector(size_t n, double initialValue = 0.0);

    Vector(size_t n, StorageOrder order, double initialValue = 0.0);

    Vector(std::initializer_list<double> values);

    Vector(std::initializer_list<double> values, StorageOrder order);

    template <typename Function>
    explicit Vector(size_t n, Function f) : _data(n), _order(StorageOrder::column_major)
    {
        for (size_t i = 0; i < n; ++i)
        {
            _data[i] = f(i);
        }
    }

    template <typename Function>
    Vector(size_t n, StorageOrder order, Function f) : _data(n), _order(order)
    {
        for (size_t i = 0; i < n; ++i)
        {
            _data[i] = f(i);
        }
    }

    size_t size() const;

    size_t rows() const;

    size_t cols() const;

    std::vector<double> &data();

    const std::vector<double> &data() const;

    StorageOrder order() const;

    bool is_row_major() const;

    bool is_column_major() const;

    static size_t row_offset(size_t i, size_t j, size_t lda);

    static size_t column_offset(size_t i, size_t j, size_t lda);

    size_t offset(size_t i) const;

    double &operator[](size_t i);

    const double &operator[](size_t i) const;

    double dot(const Vector &other) const;

    double norm() const;
};

#endif