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
	static Matrix Rotate(double Xangle, double Yangle, double Zangle, double cx, double cy, double cz);
	static Matrix Translate(double tx, double ty, double tz);
	static Matrix Scale(double sx, double sy, double sz, double cx, double cy, double cz);
	static Matrix ViewingTransformation(Matrix camera, Matrix cameraDirection, Matrix up);
	static void Line(Matrix mat1, Matrix mat2, COLORREF newColor);

private:
	static const int d = 60;
	static const int s = 15;
	static int width;				// Canvas width
	static int height;				// Canvas height
	static DWORD * bufferArray;		// Pixel array
	static COLORREF color;			// Color for the line

	static void setPixel(int x, int y);
	static void DrawLine(int x0, int y0, int x1, int y1);
	static Matrix BasicScale(double sx, double sy, double sz);
	static Matrix RotateX(double angle);
	static Matrix RotateY(double angle);
	static Matrix RotateZ(double angle);
	static bool Clip(int & x1, int & x2, int & y1, int & y2);
	static int ComputeDisplayCode(int x, int y);
};

