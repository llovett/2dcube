/**
 * matrix.cpp
 *
 * Implementation file for matrix math library.
 *
 * Author: Luke Lovett
 * */

#include "matrix.h"
#include <iostream>
#include <iomanip>
#include <cassert>
#include <cstdlib>

using namespace std;

// int main() {
//     float m2[] = {
// 	1.0, 0.0, 0.0,
// 	0.0, 1.0, 9.0,
// 	0.0, 0.5, 1.0
//     };
//     float m1[] = {
// 	0.0, 0.0, 1.0,
// 	0.0, 1.0, 0.0,
// 	1.0, 0.0, 0.0
//     };
//     Matrix Mat1(3,3,m1);
//     Matrix Mat2(3,3,m2);

//     Matrix Mat3 = Mat1 + Mat2;
//     // cout << "rows: " << m.getRows() << endl;
//     // cout << "columns: " << m.getColumns() << endl;
//     // cout << "entry at [0,0]: " << m(0,0) << endl;
//     // m(0,0) = 4.0f;
//     // cout << "entry at [0,0]: " << m(0,0) << endl;
//     cout << "MATRIX 1 --------------------" << endl;
//     Mat1.print();
//     // cout << "MATRIX 2 --------------------" << endl;
//     // Mat2.print();
//     // cout << "SUM -------------------------" << endl;
//     // Mat3.print();
//     // cout << "PRODUCT ---------------------" << endl;
//     // Matrix Mat4 = Mat1 * Mat2;
//     // Mat4.print();
//     // cout << "BOTTOM-RIGHT SUB-MATRIX -----" << endl;
//     // Matrix Mat5 = Mat4.subMatrix(0,0,2,2);
//     // Mat5.print();
//     // cout << "REMOVING MIDDLE ROW ----------" << endl;
//     // Matrix Mat6 = Mat3.rowRemoved(1);
//     // Mat6.print();
//     cout << "DETERMINANT OF SECOND MATRIX -" << endl;
//     cout << Matrix::Determinant(Mat2) << endl;


//     return 0;
// }

int main() {
    cout << "--------------------" << endl;
    float dummy[] = {1,2,3};
    Matrix test(3,1,dummy);
    test << dummy;
    test.print();
    cout << "--------------------" << endl;

    float arr1[] = { 1, 0, 0 };
    float arr2[] = { 0, 1, 0 };
    float *arr3 = crossProduct(arr1, arr2);

    int i;
    cout << "Cross-product of <1,0,0> and <0,1,0> is <";
    for ( i=0; i<3; i++ ) {
	cout << arr3[i];
	if ( i < 3-1 )
	    cout << ",";
    }
    cout << ">" << endl;
    
    return 0;
}

Matrix::Matrix(int rows, int columns) {
    this->init(rows,columns,NULL);
}

Matrix::Matrix(int rows, int columns, float *entries ) {
    this->init(rows,columns,entries);
}

void Matrix::init(int rows, int columns, float *entries) {
    // Initialize vectors
    int i;
    this->entries.resize(rows);
    for ( i=0; i<rows; i++ ) {
	this->entries[i].resize(columns);
    }
    if ( NULL != entries ) {
	int i,j;
	for ( i=0; i<rows; i++ ) {
	    for ( j=0; j<columns; j++ ) {
		this->entries[i][j] = entries[i*columns + j];
	    }
	}
    }
    
    this->rows = rows;
    this->columns = columns;
}

int Matrix::getRows() const {
    return this->rows;
}

int Matrix::getColumns() const {
    return this->columns;
}

vector<float> Matrix::getRow(int index) const {
    assert(index >= 0 && index < this->entries.size());
    return this->entries[index];
}

vector<float> Matrix::getColumn(int index) const {
    assert(index >= 0 && index < this->entries[0].size());
    int i;
    vector<float> rows;
    for ( i=0; i<this->entries.size(); i++ ) {
	rows.push_back( this->entries[i][index] );
    }
    return rows;
}

float Matrix::getEntry(int i, int j) const {
    return this->entries[i][j];
}

Matrix Matrix::subMatrix(int row1, int col1, int rows, int cols) const {
    if ( rows <= 0 || cols <= 0 )
	return Matrix(0,0);

    float entries[rows * cols];
    int i,j;
    for ( i=0; i<rows; i++ ) {
	for ( j=0; j<cols; j++ ) {
	    entries[i*cols + j] = this->getEntry(i+col1,j+row1);
	}
    }
    return Matrix(rows,cols,entries);
}

Matrix Matrix::rowRemoved(int row) const {
    Matrix mat = Matrix(this->getRows()-1,this->getColumns());
    int i,k;
    for ( i=0, k=0; i<this->getRows(); i++ ) {
	if ( i == row ) continue;
	vector<float> row = this->getRow(i);
	int j;
	for ( j=0; j<this->getColumns(); j++ ) {
	    mat.setEntry(k,j,row[j]);
	}
	k++;
    }
    return mat;
}

Matrix Matrix::columnRemoved(int col) const {
    Matrix mat = Matrix(this->getRows(),this->getColumns()-1);
    int i;
    for ( i=0; i<this->rows; i++ ) {
	vector<float> row = this->getRow(i);
	int j,k;
	for ( j=0, k=0; j<this->getColumns(); j++ ) {
	    if ( j == col ) continue;
	    mat.setEntry(i,k,row[j]);
	    k++;
	}
    }
    return mat;
}

void Matrix::setEntry(int i, int j, float value) {
    this->entries[i][j] = value;
}

void Matrix::setMatrix(float *matrix) {
    this->init(this->getRows(), this->getColumns(), matrix);
}

float& Matrix::operator() (int row, int col) {
    assert(row >= 0 && col >= 0 && row < this->getRows() && col < this->getColumns());
    return this->entries[row][col];
}

void Matrix::operator<< (float *matrix) {
    this->setMatrix(matrix);
}

Matrix operator+(const Matrix& A, const Matrix& B) {
    assert( A.getRows() == B.getRows() || A.getColumns() == B.getColumns() );
    int i,j;
    Matrix result(A.getRows(), A.getColumns());
    for ( i=0; i<A.getRows(); i++ ) {
	for ( j=0; j<A.getColumns(); j++ ) {
	    result(i,j) = A.getEntry(i,j) + B.getEntry(i,j);
	}
    }
    return result;
}

Matrix operator*(const Matrix& A, const Matrix& B) {
    assert( A.getColumns() == B.getRows() );
    Matrix result(A.getRows(), B.getColumns());

    int i,j;
    // For each entry in the resulting matrix with row i and col j...
    for ( i=0; i<A.getRows(); i++ ) {
	for ( j=0; j<B.getColumns(); j++ ) {
	    float entry = 0.0f;
	    int k;
	    for ( k=0; k<A.getRows(); k++ ) {
		entry += A.getEntry(i,k) * B.getEntry(k,j);
	    }
	    result(i,j) = entry;
	}
    }
    return result;
}

float Matrix::Determinant(const Matrix& matrix) {
    // Square matrices only!
    assert( matrix.getRows() == matrix.getColumns() );

    // Base cases
    if ( matrix.getRows() == 1 )
	return matrix.getEntry(0,0);
    if ( matrix.getRows() == 2 )
	return matrix.getEntry(0,0) * matrix.getEntry(1,1) - matrix.getEntry(1,0) * matrix.getEntry(0,1);

    float det = 0.0f;
    int i,j;
    Matrix subMatrix = matrix.rowRemoved(0);
    for ( i=1, j=0; j<matrix.getColumns(); j++, i*=-1 ) {
	float multiplicand = i*matrix.getEntry(0,j);
	float subDet = Determinant(subMatrix.columnRemoved(j));
	det += subDet * multiplicand;
    }
    return det;
}

void Matrix::print() const {
    int i,j;
    for ( i=0; i<this->getRows(); i++ ) {
	cout << "[ ";
	for ( j=0; j<this->getColumns(); j++ ) {
	    cout << fixed << setprecision(6) << this->getEntry(i,j) << " ";
	}
	cout << "]" << endl;
    }
}

float *crossProduct(float a[3], float b[3]) {
    Matrix vectorB = Matrix(3, 1, b);
    float entriesSkewA[9] = { 0, -a[2], a[1],
			      a[2], 0, -a[0],
			      -a[1], a[0], 0 };
    Matrix skewA = Matrix(3, 3, entriesSkewA);

    Matrix resultVector = skewA * vectorB;
    float *result = (float*)malloc(sizeof(float)*3);
    int i;

    for ( i = 0; i<3; i++ ) {
	result[i] = resultVector(i,0);
    }
    return result;
}
