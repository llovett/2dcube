/*
 * matrix.h
 *
 * Defines mathematical functions on matrices.
 *
 * Author: Luke Lovett
 *
 */

#include <vector>

using std::vector;

class Matrix {
private:
    int rows, columns;
    vector<vector<float> > entries;
    vector<float> getRow(int) const;
    vector<float> getColumn(int) const;
    // Constructor helper
    void init(int, int, float *);

public:
    Matrix(int rows, int columns);
    Matrix(int rows, int columns, float *entries );
    int getRows() const;
    int getColumns() const;
    float getEntry(int, int) const;
    Matrix subMatrix(int, int, int, int) const;
    Matrix columnRemoved(int) const;
    Matrix rowRemoved(int) const;
    void setEntry(int, int, float);
    void setMatrix(float *matrix);
    void print() const;

    static Matrix Homogenize(const Matrix&);
    static float Determinant(const Matrix&);

    float& operator() (const int row, const int col);
    void operator<< (float *);
};

Matrix operator+(const Matrix& lhs, const Matrix& rhs);
Matrix operator-(const Matrix& lhs, const Matrix& rhs);
Matrix operator*(const Matrix& lhs, const Matrix& rhs);
Matrix operator*(const Matrix& M, const float f);

/* other utility functions for vectors */
float *crossProduct(float *a, float *b);
