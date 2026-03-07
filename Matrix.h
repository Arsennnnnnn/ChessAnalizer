#ifndef MATRIX_H
#define MATRIX_H
#include <vector>
#include <stdexcept>

template<typename T>
class Matrix {
protected:
    int rows_, cols_;
    std::vector<std::vector<T>> data_;

public:
    Matrix(int rows, int cols, T defaultVal = T())
        : rows_(rows), cols_(cols), data_(rows, std::vector<T>(cols, defaultVal)) {}

    Matrix(const Matrix&) = default;
    Matrix& operator=(const Matrix&) = default;
    virtual ~Matrix() = default;

    T& at(int row, int col) {
        if (row < 0 || row >= rows_ || col < 0 || col >= cols_)
            throw std::out_of_range("Matrix index out of range");
        return data_[row][col];
    }

    const T& at(int row, int col) const {
        if (row < 0 || row >= rows_ || col < 0 || col >= cols_)
            throw std::out_of_range("Matrix index out of range");
        return data_[row][col];
    }

    int rows() const { return rows_; }
    int cols() const { return cols_; }

    bool inBounds(int row, int col) const {
        return row >= 0 && row < rows_ && col >= 0 && col < cols_;
    }
};

#endif // MATRIX_H