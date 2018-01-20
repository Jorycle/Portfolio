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
	DrawLine(mat1(0, 0), mat1(0, 1), mat2(0, 0), mat2(0, 1));
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
Matrix DrawGraphics::Rotate(double angle, double cx, double cy)
{
	Matrix returnMatrix = BasicTranslate(-cx, -cy);
	returnMatrix *= BasicRotate(-angle); // Negative so default angle is clockwise
	returnMatrix *= BasicTranslate(cx, cy);
	return returnMatrix;
}

/* Translate a matrix */
Matrix DrawGraphics::BasicTranslate(double tx, double ty)
{
	Matrix returnMatrix({ {1,0,0},{0,1,0},{tx,ty, 1} });
	return returnMatrix;
}

/* Scale a matrix */
Matrix DrawGraphics::Scale(double sx, double sy, double cx, double cy)
{
	Matrix returnMatrix = BasicTranslate(-cx, -cy);
	returnMatrix *= BasicScale(sx, sy);
	returnMatrix *= BasicTranslate(cx, cy);
	return returnMatrix;
}

/* Basic scale component, does the actual work */
Matrix DrawGraphics::BasicScale(double sx, double sy)
{
		Matrix returnMatrix{ { sx, 0, 0 },{ 0, sy, 0 },{ 0, 0, 1 } };
		return returnMatrix;
}

/* Basic rotate component, does the actual work */
Matrix DrawGraphics::BasicRotate(double angle)
{
	// Multiply by pi/180 to get angles in degrees
	Matrix returnMatrix({ { cos(angle * M_PI / 180.0), -sin(angle * M_PI / 180.0), 0 },{ sin(angle * M_PI / 180.0), cos(angle * M_PI / 180.0), 0 },{ 0, 0, 1 } });
	return returnMatrix;
}