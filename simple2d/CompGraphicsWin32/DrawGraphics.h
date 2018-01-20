#pragma once
#include <windows.h>
#include <stdlib.h>
#include <vector>
#include <utility>
#define _USE_MATH_DEFINES
#include <math.h>
#include "Matrix.h"


/*
* This is a special fancy class for drawing things to the screen
*/
class DrawGraphics
{

public:
	static void Setup(DWORD * bufferArray, int drawWidth, int drawHeight);
	static Matrix Rotate(double angle, double cx, double cy);
	static Matrix BasicTranslate(double tx, double ty);
	static Matrix Scale(double sx, double sy, double cx, double cy);
	static void Line(Matrix mat1, Matrix mat2, COLORREF newColor);

private:
	static int width;				// Canvas width
	static int height;				// Canvas height
	static DWORD * bufferArray;		// Pixel array
	static COLORREF color;			// Color for the line

	static void setPixel(int x, int y);
	static void DrawLine(int x0, int y0, int x1, int y1);
	static Matrix BasicScale(double sx, double sy);
	static Matrix BasicRotate(double angle);
};

