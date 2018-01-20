#include "DrawGraphics.h"

COLORREF DrawGraphics::color;
DWORD * DrawGraphics::bufferArray;
int DrawGraphics::width;
int DrawGraphics::height;

/* 
* Setup the buffer array appropriately
*/
void DrawGraphics::Setup(DWORD * newBits, int drawWidth, int drawHeight)
{
	width = drawWidth;
	height = drawHeight;
	bufferArray = newBits;
}

/* 
* Calculates the correct place in the array and sets the color
*/
void DrawGraphics::setPixel(int x, int y)
{
	if (x > width - 1|| x < 0)
		return;
	if (y > height - 1 || y < 0)
		return;

	bufferArray[x + (y * width)] = color;
}
	
/* Public interface for drawing lines */
void DrawGraphics::Line(Matrix mat1, Matrix mat2, COLORREF newColor)
{
	color = newColor;
	int x1 = ((mat1(0, 0) / mat1(0, 2))*width / 2) + (width / 2);
	int x2 = ((mat2(0, 0) / mat2(0, 2))*width / 2) + (width / 2);
	int y1 = ((mat1(0, 1) / mat1(0, 2))*height / 2) + (height / 2);
	int y2 = ((mat2(0, 1) / mat2(0, 2))*height / 2) + (height / 2);
	if (DrawGraphics::Clip(x1, x2, y1, y2))
		DrawLine(x1, y1, x2, y2);
}

/*
* Fancy Bresenham algorithm. Uses ints and magic.
*/
void DrawGraphics::DrawLine(int x0, int y0, int x1, int y1)
{
	int x, y;
	int dx, dy;
	int dxA, dyA;
	int inc1, inc2;
	int step, error, endLine;

	dx = x1 - x0;  // deltaX
	dy = y1 - y0;  // deltaY
	dxA = abs(dx); // Absolute value of deltaX
	dyA = abs(dy); // Absolute value of deltaY

	if ((dx < 0 && dy < 0) || (dx > 0 && dy > 0))
		step = 1;
	else
		step = -1;

	if (dyA <= dxA) // Not a steep line
	{
		error = (dyA << 1) - dxA;
		inc1 = dyA << 1;
		inc2 = (dyA - dxA) << 1;

		if (dx >= 0)
		{
			x = x0;
			y = y0;
			endLine = x1;
		}
		else
		{
			x = x1;
			y = y1;
			endLine = x0;
		}

		setPixel(x, y);

		while (x < endLine)
		{
			x++;
			if (error < 0)
				error += inc1;
			else
			{
				y += step;
				error += inc2;
			}

			setPixel(x, y);
		}
	}
	else // Steep line
	{
		inc1 = dxA << 1;
		inc2 = (dxA - dyA) << 1;
		error = (dxA << 1) - dyA;

		if (dy >= 0)
		{
			x = x0;
			y = y0;
			endLine = y1;
		}
		else
		{
			x = x1;
			y = y1;
			endLine = y0;
		}

		setPixel(x, y);

		while (y < endLine)
		{
			y++;
			if (error <= 0)
				error += inc1;
			else
			{
				x += step;
				error += inc2;
			}

			setPixel(x, y);
		}
	}
}

/* Rotate a matrix */
Matrix DrawGraphics::Rotate(double Xangle, double Yangle, double Zangle, double cx, double cy, double cz)
{
	Matrix returnMatrix = Translate(-cx, -cy, -cz);
	returnMatrix *= RotateX(Xangle);
	returnMatrix *= RotateY(Yangle);
	returnMatrix *= RotateZ(Zangle);
	returnMatrix *= Translate(cx, cy, cz);
	return returnMatrix;
}

/* Translate a matrix */
Matrix DrawGraphics::Translate(double tx, double ty, double tz)
{
	Matrix returnMatrix({
		{1,0,0,0},
		{0,1,0,0},
		{0,0,1,0},
		{tx,ty,tz,1} });
	return returnMatrix;
}

/* Scale a matrix */
Matrix DrawGraphics::Scale(double sx, double sy, double sz, double cx, double cy, double cz)
{
	Matrix returnMatrix = Translate(-cx, -cy, -cz);
	returnMatrix *= BasicScale(sx, sy, sz);
	returnMatrix *= Translate(cx, cy, cz);
	return returnMatrix;
}

/* Basic scale component, does the actual work */
Matrix DrawGraphics::BasicScale(double sx, double sy, double sz)
{
	Matrix returnMatrix({
		{ sx, 0, 0, 0 },
		{ 0, sy, 0, 0 },
		{ 0, 0, sz, 0 },
		{ 0, 0, 0, 1 } });
	return returnMatrix;
}

/* Basic rotate component around X, does the actual work */
Matrix DrawGraphics::RotateX(double angle)
{
	// Multiply by pi/180 to get angles in degrees
	angle = (angle * M_PI / 180.0);
	Matrix returnMatrix({
		{ 1, 0, 0, 0},
		{ 0, cos(angle), sin(angle), 0},
		{ 0, -sin(angle), cos(angle), 0},
		{ 0, 0, 0, 1} });
	return returnMatrix;
}

/* Basic rotate component around Y, does the actual work */
Matrix DrawGraphics::RotateY(double angle)
{
	// Multiply by pi/180 to get angles in degrees
	angle = (angle * M_PI / 180.0);
	Matrix returnMatrix({
		{ cos(angle), 0, sin(angle), 0},
		{ 0, 1, 0, 0},
		{ -sin(angle), 0, cos(angle), 0},
		{ 0, 0, 0, 1} });
	return returnMatrix;
}

/* Basic rotate component around Z, does the actual work */
Matrix DrawGraphics::RotateZ(double angle)
{
	// Multiply by pi/180 to get angles in degrees
	angle = (angle * M_PI / 180.0);
	Matrix returnMatrix({
		{ cos(angle), sin(angle), 0, 0},
		{ -sin(angle), cos(angle), 0, 0},
		{ 0, 0, 1, 0},
		{ 0, 0, 0, 1} });
	return returnMatrix;
}

Matrix DrawGraphics::ViewingTransformation(Matrix camera, Matrix cameraDirection, Matrix up)
{
	double pX = camera(0, 0);
	double pY = camera(0, 1);
	double pZ = camera(0, 2);

	Matrix N = cameraDirection - camera;
	double length = Matrix::Length(N);
	Matrix n = N;
	n(0, 0) = n(0, 0) / length;
	n(0, 1) = n(0, 1) / length;
	n(0, 2) = n(0, 2) / length;

	Matrix u1 = Matrix::CrossProduct(N, up);
	double uLength = Matrix::Length(u1);
	Matrix u = u1;
	u(0, 0) = u(0, 0) / uLength;
	u(0, 1) = u(0, 1) / uLength;
	u(0, 2) = u(0, 2) / uLength;
	Matrix v = Matrix::CrossProduct(u, n);

	Matrix R({ {u(0,0), v(0,0), n(0,0), 0},
	{ u(0,1), v(0,1), n(0,1), 0 },
	{u(0,2), v(0,2), n(0,2), 0},
	{0, 0, 0, 1 }
	});


	Matrix t1 = Translate(-pX, -pY, -pZ);
	Matrix t2({
		{ 1, 0, 0, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 0, -1, 0 },
		{ 0, 0, 0, 1 } });
	Matrix t3({
		{ d/s, 0, 0, 0 },
		{ 0, d/s, 0, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 1 } });
	Matrix v2 = t1 * R * t2 * t3;
	return v2;
}

bool DrawGraphics::Clip(int & x1, int & x2, int & y1, int & y2)
{
	int CSTOP = 1;
	int CSBOTTOM = 2;
	int CSRIGHT = 4;
	int CSLEFT = 8;

	bool finished = false;
	bool draw = false;
	int outcode1 = DrawGraphics::ComputeDisplayCode(x1, y1);
	int outcode2 = DrawGraphics::ComputeDisplayCode(x2, y2);

	do
	{
		if (outcode1 == 0 && outcode2 == 0)
		{
			finished = true;
			draw = true;
		}
		else if (outcode1 & outcode2)
		{
			finished = true;
		}
		else
		{
			double x, y;
			int outcode_ex = outcode1 ? outcode1 : outcode2;
			if (outcode_ex & CSTOP)
			{
				x = x1 + (x2 - x1) * (height - y1) / (y2 - y1);
				y = height;
			}

			else if (outcode_ex & CSBOTTOM)
			{
				x = x1 + (x2 - x1) * (0 - y1) / (y2 - y1);
				y = 0;
			}
			else if (outcode_ex & CSRIGHT)
			{
				y = y1 + (y2 - y1) * (width - x1) / (x2 - x1);
				x = width;
			}
			else
			{
				y = y1 + (y2 - y1) * (0 - x1) / (x2 - x1);
				x = 0;
			}
			if (outcode_ex == outcode1)
			{
				x1 = x;
				y1 = y;
				outcode1 = ComputeDisplayCode(x1, y1);
			}
			else
			{
				x2 = x;
				y2 = y;
				outcode2 = ComputeDisplayCode(x2, y2);
			}
		}
	} while (finished == FALSE);

	return draw;

}

int DrawGraphics::ComputeDisplayCode(int x, int y)
{
	int CSTOP = 1;
	int CSBOTTOM = 2;
	int CSRIGHT = 4;
	int CSLEFT = 8;
	int outcode = 0;

	if (y > height)
		outcode |= CSTOP;
	else if (y < 0)
		outcode |= CSBOTTOM;

	if (x > width)
		outcode |= CSRIGHT;
	else if (x < 0)
		outcode |= CSLEFT;

	return outcode;
}