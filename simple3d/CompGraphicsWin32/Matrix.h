#pragma once
#include <vector>

/* 
* This is a class for matrices and matrix math
*/
class Matrix
{
	public:
		Matrix();
		Matrix(std::initializer_list<std::initializer_list<double>> lst);
		Matrix(const Matrix & mat);
		Matrix(size_t rows, size_t cols);
		~Matrix();

		int getRows() const;
		int getCols() const;

		static Matrix CrossProduct(const Matrix &v1, const Matrix &v2);
		static Matrix Normalize(const Matrix &vector);
		static double Matrix::Length(const Matrix &vector);

		Matrix operator/(const Matrix & rightMat);
		Matrix operator+(const Matrix & rightMat);
		Matrix operator-(const Matrix & rightMat);
		Matrix operator*(const Matrix & rightMat);
		Matrix & operator*=(const Matrix & rightMat);
		Matrix & operator=(const Matrix & rightMat);

		const double operator()(const int & row, const int & col) const;
		double & operator()(const int & row, const int & col);

	private:
		std::vector<std::vector<double>> matrix;
		int rows;
		int cols;
};

