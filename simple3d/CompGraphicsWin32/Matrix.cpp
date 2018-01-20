#include "Matrix.h"

/* Default constructor, set some basic values */
Matrix::Matrix()
{
	rows = 0;
	cols = 0;
}

/* Initialize the matrix from a list of values, preferred method
* Syntax: Matrix example({{x,x,x}, {x,x,x}, {x,x,x}})
*/
Matrix::Matrix(std::initializer_list<std::initializer_list<double>> list) : Matrix(list.size(), list.size() ? list.begin()->size() : 0)
{
	int i = 0;
	for (const auto& l : list)
	{
		int j = 0;
		for (const auto& v : l)
		{
			matrix[i][j++] = v;
		}
		i++;
	}
}

/* Initializes a matrix to all 0s */
Matrix::Matrix(size_t setRows, size_t setCols)
{
	rows = setRows;
	cols = setCols;
	matrix.resize(setRows);
		for (int i = 0; i < setRows; i++)
			matrix[i].resize(setCols, 0.0);
}

/* Copy constructor */
Matrix::Matrix(const Matrix & mat)
{
	matrix = mat.matrix;
	rows = mat.getRows();
	cols = mat.getCols();
}

/* Destructor, ain't nothin goin on here */
Matrix::~Matrix()
{

}

/* Return number of rows in Matrix */
int Matrix::getRows() const
{
	return rows;
}

/* Return number of columns in Matrix */
int Matrix::getCols() const
{
	return cols;
}

/* Cross product of two vectors */
Matrix Matrix::CrossProduct(const Matrix &v1, const Matrix &v2)
{
	Matrix result(1, 4);

	result(0, 0) = (v1(0, 1) * v2(0, 2)) - (v1(0, 2)*v2(0, 1));
	result(0, 1) = (v1(0, 2) * v2(0, 0)) - (v1(0, 0)*v2(0, 2));
	result(0, 2) = (v1(0, 0) * v2(0, 1)) - (v1(0, 1)*v2(0, 0));
	result(0, 3) = 1;

	return result;
}

/* Normalize a vector */
Matrix Matrix::Normalize(const Matrix &vector)
{
	Matrix result(1, 4);

	double x = vector(0, 0) * vector(0, 0);
	double y = vector(0, 1) * vector(0, 1);
	double z = vector(0, 2) * vector(0, 2);
	double sum = sqrt(x + y + z);

	result(0, 0) = vector(0, 0) / sum;
	result(0, 1) = vector(0, 1) / sum;
	result(0, 2) = vector(0, 2) / sum;
	result(0, 3) = 1;

	return result;
}

/* Get vector length */
double Matrix::Length(const Matrix &vector)
{
	double x = vector(0, 0) * vector(0, 0);
	double y = vector(0, 1) * vector(0, 1);
	double z = vector(0, 2) * vector(0, 2);
	double sum = sqrt(x + y + z);

	return sum;
}

/* Multiply two matrices together and return the product */
Matrix Matrix::operator*(const Matrix & rightMat)
{
	int resRows = rightMat.getRows();
	int resCols = rightMat.getCols();
	
	Matrix result(this->rows, resCols);

	for (int row = 0; row < this->rows; row++)
	{
		for (int col = 0; col < resCols; col++)
		{
			for (int k = 0; k < resRows; k++)
			{
				result(row, col) += matrix[row][k] * rightMat(k, col);
			}
		}
	}

	return result;
}

/* Multiply two matrices together, and set the left hand side to the product */
Matrix & Matrix::operator*=(const Matrix & rightMat)
{
	Matrix result = (*this) * rightMat;
	(*this) = result;
	return *this;
}

/* Assign right matrix to left matrix */
Matrix & Matrix::operator=(const Matrix & rightMat)
{
	if (&rightMat == this)
		return * this;

	int newRows = rightMat.getRows();
	int newCols = rightMat.getCols();
	rows = newRows;
	cols = newCols;

	matrix.resize(newRows);
	for (int i = 0; i < newRows; i++)
		matrix[i].resize(newCols);

	for (int i = 0; i < newRows; i++)
	{
		for (int j = 0; j < newCols; j++)
			matrix[i][j] = rightMat(i, j);
	}

	return * this;
	
}

/* For accessing values */
const double Matrix::operator()(const int & row, const int & col) const
{
	return matrix[row][col];
}

/* Also for accessing values */
double & Matrix::operator()(const int & row, const int & col)
{
	return matrix[row][col];
}

/* Adds two matrices together */
Matrix Matrix::operator+(const Matrix & rightMat)
{
	int resRows = rightMat.getRows();
	int resCols = rightMat.getCols();

	Matrix result(this->rows, resCols);

	for (int row = 0; row < this->rows; row++)
	{
		for (int col = 0; col < resCols; col++)
		{
			result(row, col) = matrix[row][col] += rightMat(row,col);
		}
	}

	return result;
}

/* Subtracts two matrices */
Matrix Matrix::operator-(const Matrix & rightMat)
{
	int resRows = rightMat.getRows();
	int resCols = rightMat.getCols();

	Matrix result(this->rows, resCols);

	for (int row = 0; row < this->rows; row++)
	{
		for (int col = 0; col < resCols; col++)
		{
			result(row, col) = matrix[row][col] - rightMat(row, col);
		}
	}

	return result;
}

/* Divides two matrices */
Matrix Matrix::operator/(const Matrix & rightMat)
{
	int resRows = rightMat.getRows();
	int resCols = rightMat.getCols();

	Matrix result(this->rows, resCols);

	for (int row = 0; row < this->rows; row++)
	{
		for (int col = 0; col < resCols; col++)
		{
			result(row, col) = matrix[row][col] / rightMat(row, col);
		}
	}

	return result;
}