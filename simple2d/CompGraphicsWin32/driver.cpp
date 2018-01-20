#include <windows.h>  
#include <tchar.h>
#include <time.h>
#include <ShObjIdl.h>
#include <fstream>
#include <iostream>
#include <string>
#include "DrawGraphics.h"
#include "Matrix.h"
#include "resource.h"

/* App name */
static TCHAR szAppName[] = _T("CS4050Program");
/* App title bar label */
static TCHAR szTitle[] = _T("Graphics Drawing Program");
static int LEFTMARGIN = 10;
static int RIGHTMARGIN = 50;

// Struct for containing the point matrices of a line
struct lines
{
	Matrix point1;
	Matrix point2;
};
std::vector<Matrix> transConcat;

HWND mainWinHwd = NULL;			// Main window handle
HINSTANCE hInst;				// Main window instance
RECT mainWinRect;				// Main window rect
RECT drawRect;					// Draw window rect
BITMAPINFO bufferBmp = { 0 };	// Draw buffer bitmap info
HBITMAP bufferHBmp = NULL;		// Draw Buffer bitmap
DWORD * bufferArray = NULL;		// Draw buffer array
HDC bufferMemHDC = NULL;		// Draw buffer HDC
HDC drawHDC = NULL;				// Draw window HDC
int drawWidth;					// Draw window width
int drawHeight;					// Draw window height

HWND lineList;						// Display window for lines
std::string outputLines;			// Line copy for file save
std::vector<lines> globLinePoints;	// Vector of all lines

LRESULT CALLBACK TransformDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void DrawButton(COLORREF colorRef, bool randomColors);
void LoadLines();
void ApplyTransformation();
void DisplayLineList();
void SaveLines();
void RandomizeColors(COLORREF & colorRef);

/*
* Main window setup and tomfoolery (basic stuff from the MSDN tutorials with some customization)
*/
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	srand((unsigned)time(NULL)); // Seed the RNG with the current time
	WNDCLASSEX mainWindow;
	mainWindow.cbSize = sizeof(WNDCLASSEX);
	mainWindow.style = CS_HREDRAW | CS_VREDRAW;
	mainWindow.lpfnWndProc = WndProc;
	mainWindow.cbClsExtra = 0;
	mainWindow.cbWndExtra = 0;
	mainWindow.hInstance = hInstance;
	mainWindow.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	mainWindow.hCursor = LoadCursor(NULL, IDC_ARROW);
	mainWindow.hbrBackground = CreateSolidBrush(RGB(200, 200, 200));
	mainWindow.lpszMenuName = NULL;
	mainWindow.lpszClassName = szAppName;
	mainWindow.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	if (!RegisterClassEx(&mainWindow))
	{
		MessageBox(NULL, _T("RegisterClassEx failed!"), _T("Graphics Drawing Error"), NULL);
		return 1;
	}

	hInst = hInstance;

	/* Main App Window area */
	HWND hWnd = CreateWindow(
		szAppName,
		szTitle,
		WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
		CW_USEDEFAULT, CW_USEDEFAULT,
		1200, 700,
		NULL,
		NULL,
		hInst,
		NULL
	);

	/* Check for window creation failure */
	if (!hWnd)
	{
		MessageBox(NULL, _T("Window creation failed!"), _T("Graphics Drawing Error"), NULL);
		return 1;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	mainWinHwd = hWnd;
	MSG msg;

	/* Loop until we're done with the program */
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

/* 
* Handles the guts of our main window
*/
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HWND redColorValue, blueColorValue, greenColorValue;
	static HWND drawSpace;
	static HWND randColorCheck;
	COLORREF colorRef;
	PAINTSTRUCT ps;
	HDC hdc;
	GetClientRect(hWnd, &mainWinRect); // Get the window size for later

	switch (message)
	{
		case WM_CREATE:
		{
			//COLOR STUFF
			{
				randColorCheck = CreateWindowEx(
					NULL,
					TEXT("BUTTON"),
					TEXT("Random Colors"),
					WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
					mainWinRect.right - 375 - LEFTMARGIN,  // x pos
					20,					// y pos
					120,				// width
					20,					// height
					hWnd,				// parent
					(HMENU)55,
					hInst,
					NULL);

				/* R label for R color value */
				CreateWindow(
					TEXT("STATIC"),
					TEXT("R"),
					WS_CHILD | WS_VISIBLE | SS_LEFT | WS_EX_TRANSPARENT,
					mainWinRect.right - 233 - LEFTMARGIN,	// x pos
					5,				// y pos
					15,				// width
					20,				// height
					hWnd,			// parent
					(HMENU)3,
					hInst,
					NULL);

				/* R color value text box */
				redColorValue = CreateWindow(
					WC_EDIT,
					TEXT("0"),
					WS_CHILD | WS_BORDER | WS_VISIBLE | ES_CENTER | ES_NUMBER,
					mainWinRect.right - 245 - LEFTMARGIN,	// x pos
					19,			// y pos
					35,			// width
					20,			// height
					hWnd,		//parent
					(HMENU)6,
					hInst,
					NULL
				);

				/* G label for G color value */
				CreateWindow(
					TEXT("STATIC"),
					TEXT("G"),
					WS_CHILD | WS_VISIBLE | SS_LEFT | WS_EX_TRANSPARENT,
					mainWinRect.right - 193 - LEFTMARGIN,	// x pos
					5,					// y pos
					15,					// width
					20,					// height
					hWnd,				// parent
					(HMENU)4,
					hInst,
					NULL);

				/* G color value text box */
				greenColorValue = CreateWindow(
					WC_EDIT,
					TEXT("0"),
					WS_CHILD | WS_BORDER | WS_VISIBLE | ES_CENTER | ES_NUMBER,
					mainWinRect.right - 205 - LEFTMARGIN,	// x pos
					19,			// y pos
					35,			// width
					20,			// height
					hWnd,		//parent
					(HMENU)7,
					hInst,
					NULL
				);

				/* B label for B color value */
				CreateWindow(
					TEXT("STATIC"),
					TEXT("B"),
					WS_CHILD | WS_VISIBLE | SS_LEFT | WS_EX_TRANSPARENT,
					mainWinRect.right - 153 - LEFTMARGIN,	// x pos
					5,					// y pos
					15,					// width
					20,					// height
					hWnd,				// parent
					(HMENU)5,
					hInst,
					NULL);

				/* B color value text box */
				blueColorValue = CreateWindow(
					WC_EDIT,
					TEXT("0"),
					WS_CHILD | WS_BORDER | WS_VISIBLE | ES_CENTER | ES_NUMBER,
					mainWinRect.right - 165 - LEFTMARGIN,			// x pos
					19,			// y pos
					35,			// width
					20,			// height
					hWnd,		//parent
					(HMENU)8,
					hInst,
					NULL
				);
			}


			/* The draw button */
			CreateWindow(
				L"BUTTON",
				L"Draw",
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
				mainWinRect.right - 120 - LEFTMARGIN,         // x pos 
				10,         // y pos
				120,        // width
				30,         // height
				hWnd,		// parent
				(HMENU)1,
				hInst,
				NULL
			);

			/* The clear button */
			CreateWindow(
				L"BUTTON",
				L"Clear Canvas",
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
				10,         // x pos 
				10,         // y pos
				100,        // width
				30,         // height
				hWnd,		// parent
				(HMENU)10,
				hInst,
				NULL
			);

			//TRANSFORMATION BUTTONS
			{
				CreateWindow(
					L"BUTTON",
					L"Translate",
					WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
					150,         // x pos 
					10,         // y pos
					100,        // width
					30,         // height
					hWnd,		// parent
					(HMENU)20,
					hInst,
					NULL
				);

				CreateWindow(
					L"BUTTON",
					L"Rotate (Basic)",
					WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
					255,         // x pos 
					10,         // y pos
					100,        // width
					30,         // height
					hWnd,		// parent
					(HMENU)21,
					hInst,
					NULL
				);

				CreateWindow(
					L"BUTTON",
					L"Rotate (Point)",
					WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
					360,         // x pos 
					10,         // y pos
					100,        // width
					30,         // height
					hWnd,		// parent
					(HMENU)22,
					hInst,
					NULL
				);

				CreateWindow(
					L"BUTTON",
					L"Scale (Basic)",
					WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
					465,         // x pos 
					10,         // y pos
					100,        // width
					30,         // height
					hWnd,		// parent
					(HMENU)23,
					hInst,
					NULL
				);

				CreateWindow(
					L"BUTTON",
					L"Scale (Point)",
					WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
					570,         // x pos 
					10,         // y pos
					100,        // width
					30,         // height
					hWnd,		// parent
					(HMENU)24,
					hInst,
					NULL
				);
			}

			/* Load Lines button */
			CreateWindow(
				L"BUTTON",
				L"Load Lines",
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
				mainWinRect.right - 145,         // x pos 
				50,         // y pos
				120,        // width
				25,         // height
				hWnd,		// parent
				(HMENU)15,
				hInst,
				NULL
			);

			/* Save Lines button */
			CreateWindow(
				L"BUTTON",
				L"Save Lines",
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
				mainWinRect.right - 145,         // x pos 
				80,         // y pos
				120,        // width
				25,         // height
				hWnd,		// parent
				(HMENU)16,
				hInst,
				NULL
			);

			/* Clear Lines button */
			CreateWindow(
				L"BUTTON",
				L"Clear Lines",
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
				mainWinRect.right - 145,         // x pos 
				110,         // y pos
				120,        // width
				25,         // height
				hWnd,		// parent
				(HMENU)17,
				hInst,
				NULL
			);

			/* List of lines */
			lineList = CreateWindowEx(
				0,
				L"EDIT",
				NULL,
				WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_VSCROLL | WS_BORDER | SS_SUNKEN | ES_READONLY | ES_MULTILINE,
				mainWinRect.right - 155,		// x pos
				RIGHTMARGIN + 95,					// y pos
				145,								// width
				mainWinRect.bottom - 155,			// height
				hWnd,							//parent
				(HMENU)53,
				hInst,
				NULL
			);

			/* Canvas for drawing in */
			drawSpace = CreateWindow(
				WC_STATIC,
				TEXT(""),
				WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_BORDER | SS_SUNKEN,
				LEFTMARGIN,						// x pos
				RIGHTMARGIN,					// y pos
				mainWinRect.right - 175,// width
				mainWinRect.bottom - 60,			// height
				hWnd,							//parent
				NULL,
				hInst,
				NULL
			);

			/* Set up the canvas for drawing */
			drawHDC = GetDC(drawSpace);				// Get canvas device context
			GetClientRect(drawSpace, &drawRect);	// Get canvas info
			drawWidth = drawRect.right;				// Store canvas width
			drawHeight = drawRect.bottom;			// Store canvas height

			/* Set up the frame buffer bitmap for our canvas */
			bufferBmp.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bufferBmp.bmiHeader.biWidth = drawWidth;
			bufferBmp.bmiHeader.biHeight = -drawHeight;
			bufferBmp.bmiHeader.biPlanes = 1;
			bufferBmp.bmiHeader.biBitCount = 32;
			bufferBmp.bmiHeader.biCompression = BI_RGB;

			bufferHBmp = CreateDIBSection(NULL, &bufferBmp, DIB_RGB_COLORS, (LPVOID*)&bufferArray, NULL, 0);
			bufferMemHDC = CreateCompatibleDC(NULL);
			SelectObject(bufferMemHDC, bufferHBmp);
			FillRect(bufferMemHDC, &drawRect, CreateSolidBrush(GetSysColor(COLOR_MENU))); // Buffer background color
			DrawGraphics::Setup(bufferArray, drawWidth, drawHeight); // Set the buffer into the graphic drawing class
			
			/* Check for any window creation failures */
			if (!drawSpace || !blueColorValue || !redColorValue || !greenColorValue)
			{
				MessageBox(NULL, _T("Child window creation failed!"), _T("Graphics Drawing Error"), NULL);
				return 1;
			}

			break;
		}
		/* Repaint the window when necessary */
		case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			// Copy the frame buffer only when the window is being repainted
			BeginPaint(drawSpace, &ps); 
			BitBlt(drawHDC, 0, 0, drawWidth, drawHeight, bufferMemHDC, 0, 0, SRCCOPY);
			EndPaint(drawSpace, &ps);
			break;
		}
		case WM_COMMAND:
		{
			/* Handle the draw button */
			if (LOWORD(wParam) == 1)
			{
				bool randomColors = false;
				TCHAR redVal[4];
				TCHAR greenVal[4];
				TCHAR blueVal[4];

				if (SendMessage(randColorCheck, BM_GETCHECK, TRUE, 0) == BST_CHECKED)
				{
					randomColors = true;
					colorRef = RGB(0, 0, 0);
				}
				else
				{
					GetWindowText(redColorValue, redVal, 4);		// Get red value
					GetWindowText(greenColorValue, greenVal, 4);	// Get green value
					GetWindowText(blueColorValue, blueVal, 4);		// Get blue value
					colorRef = RGB(_tstoi(blueVal), _tstoi(greenVal), _tstoi(redVal)); //RGB is actually BGR, weird
				}

				FillRect(bufferMemHDC, &drawRect, CreateSolidBrush(GetSysColor(COLOR_MENU)));
				ApplyTransformation();
				DrawButton(colorRef, randomColors);

			}
			/* Handle the clear CANVAS button */
			else if (LOWORD(wParam) == 10)
			{
				FillRect(bufferMemHDC, &drawRect, CreateSolidBrush(GetSysColor(COLOR_MENU)));
				InvalidateRect(mainWinHwd, 0, TRUE);
			}
			/* Handle the load lines button */
			else if (LOWORD(wParam) == 15)
			{
				LoadLines();
				DisplayLineList();
			}
			/* Handle the save lines button */
			else if (LOWORD(wParam) == 16)
			{
				SaveLines();
			}
			/* Handle the clear lines button */
			else if (LOWORD(wParam) == 17)
			{
				globLinePoints.clear();
				DisplayLineList();
			}
			/* Handle the Translate button */
			else if (LOWORD(wParam) == 20)
			{
				DialogBox(hInst, MAKEINTRESOURCE(TRANSDLG), hWnd, TransformDlgProc);
			}
			/* Handle the Rotate BASIC button */
			else if (LOWORD(wParam) == 21)
			{
				DialogBox(hInst, MAKEINTRESOURCE(BAS_ROTDLG), hWnd, TransformDlgProc);
			}
			/* Handle the Rotate POINT button */
			else if (LOWORD(wParam) == 22)
			{
				DialogBox(hInst, MAKEINTRESOURCE(ADV_ROTDLG), hWnd, TransformDlgProc);
			}
			/* Handle the Scale BASIC button */
			else if (LOWORD(wParam) == 23)
			{
				DialogBox(hInst, MAKEINTRESOURCE(BAS_SCALEDLG), hWnd, TransformDlgProc);
			}
			/* Handle the Scale POINT button */
			else if (LOWORD(wParam) == 24)
			{
				DialogBox(hInst, MAKEINTRESOURCE(ADV_SCALEDLG), hWnd, TransformDlgProc);
			}
			break;
		}
		case WM_CTLCOLORSTATIC:
			// Labels need to be colored specially because Windows sucks
			if (GetDlgItem(hWnd, 3) == (HWND)lParam)
			{
				SetBkMode((HDC)wParam, TRANSPARENT);
				SetTextColor((HDC)wParam, RGB(255, 0, 0));
				return (BOOL)GetStockObject(HOLLOW_BRUSH);
			}
			else if (GetDlgItem(hWnd, 4) == (HWND)lParam)
			{
				SetBkMode((HDC)wParam, TRANSPARENT);
				SetTextColor((HDC)wParam, RGB(0, 255, 0));
				return (BOOL)GetStockObject(HOLLOW_BRUSH);
			}
			else if (GetDlgItem(hWnd, 5) == (HWND)lParam)
			{
				SetBkMode((HDC)wParam, TRANSPARENT);
				SetTextColor((HDC)wParam, RGB(0, 0, 255));
				return (BOOL)GetStockObject(HOLLOW_BRUSH);
			}
			else if (GetDlgItem(hWnd, 55) == (HWND)lParam)
			{
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (BOOL)GetStockObject(HOLLOW_BRUSH);
			}
			break;
		case WM_DESTROY:
		{
			// Free up all of our stuff
			DeleteObject(bufferHBmp);
			DeleteDC(bufferMemHDC);
			DeleteDC(drawHDC);
			PostQuitMessage(0);
			break;
		}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
			break;
	}
	return 0;
}

/*
* Draw button sends line information to the draw class
*/
void DrawButton(COLORREF colorRef, bool randomColors)
{
	// Calculate n number of lines
	for (int i = 0; i < globLinePoints.size(); i++)
	{
		if (randomColors)
			RandomizeColors(colorRef);
								   
		DrawGraphics::Line(globLinePoints[i].point1, globLinePoints[i].point2, colorRef);
		
		if (i % 1000 == 0)
			RedrawWindow(mainWinHwd, &drawRect, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	
	// Drawing complete, force a repaint of the window
	InvalidateRect(mainWinHwd, &drawRect, FALSE);
}

/* Randomizes the colors when called */
void RandomizeColors(COLORREF & colorRef)
{
	colorRef = RGB((rand() % 255), (rand() % 255), (rand() % 255));
}

/* Loads lines from a file when called 
*  Handles the dialog box as well as the load itself
*/
void LoadLines()
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileOpenDialog *pFileOpen;

		// Create the FileOpenDialog object
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

		if (SUCCEEDED(hr))
		{
			// Show open dialog box
			hr = pFileOpen->Show(NULL);

			// Get file name
			if (SUCCEEDED(hr))
			{
				IShellItem *pItem;
				hr = pFileOpen->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

					// Load up the matrices
					if (SUCCEEDED(hr))
					{
						std::ifstream hFile(pszFilePath);
						int x0, y0, x1, y1;
						lines newLine;
						while (hFile >> x0)
						{
							hFile >> y0;
							hFile >> x1;
							hFile >> y1;
							Matrix point1({ {(double)x0,(double)y0,1} });
							Matrix point2({ { (double)x1,(double)y1,1 } });
							newLine.point1 = point1;
							newLine.point2 = point2;
							globLinePoints.push_back(newLine);
						}
						hFile.close();
						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}
			}
			pFileOpen->Release();
		}
		CoUninitialize();
	}
}

/* Saves lines to a file when called
*  Handles the dialog box as well as the save itself
*/
void SaveLines()
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr))
	{
		IFileSaveDialog *pFileSave;

		// Create the FileSaveDialog object
		hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
			IID_IFileSaveDialog, reinterpret_cast<void**>(&pFileSave));

		if (SUCCEEDED(hr))
		{
			// Show save dialog
			hr = pFileSave->Show(NULL);

			// Get file name
			if (SUCCEEDED(hr))
			{
				IShellItem *pItem;
				hr = pFileSave->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

					// Save the file
					if (SUCCEEDED(hr))
					{
						HANDLE hFile;
						DWORD bWritten;

						// Open the file to save to
						hFile = CreateFile(pszFilePath,
							GENERIC_READ | GENERIC_WRITE,	//Permissions
							FILE_SHARE_READ,				
							NULL,
							CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,
							NULL);

						// Make sure everything went well
						if (hFile == INVALID_HANDLE_VALUE)
						{
							MessageBox(0, _T("Could not create/open a file"), _T("Error"), 16);
							return;
						}

						// Write to the file
						WriteFile(hFile, outputLines.c_str(), outputLines.length(), &bWritten, 0);
						CloseHandle(hFile);
						CoTaskMemFree(pszFilePath);
					}
					pItem->Release();
				}
			}
			pFileSave->Release();
		}
		CoUninitialize();
	}
}

/* Displays the lines in the text box */
void DisplayLineList()
{
	LPCWSTR wide;
	std::wstring wLines;
	
	// Iterate through the list of lines and put them in the window
	for (int i = 0; i < globLinePoints.size(); i++)
	{
		wLines += std::to_wstring((int)globLinePoints[i].point1(0, 0));
		wLines += L" ";
		wLines += std::to_wstring((int)globLinePoints[i].point1(0, 1));
		wLines += L" ";
		wLines += std::to_wstring((int)globLinePoints[i].point2(0, 0));
		wLines += L" ";
		wLines += std::to_wstring((int)globLinePoints[i].point2(0, 1));
		wLines += L"\r\n";
	}
	outputLines.assign(wLines.begin(), wLines.end()); // Keep a copy for file saving purposes
	wide = wLines.c_str(); // Convert because win32 api is retarded
	SetWindowText(lineList, wide); // Set the text to the window
}

/* Apply given transformation to all lines */
void ApplyTransformation()
{
	Matrix transformation({ {1,0,0}, {0,1,0}, {0,0,1} });
	for (int i = 0; i < transConcat.size(); i++)
	{
		transformation *= transConcat[i];
	}
	transConcat.clear();
	for (int i = 0; i < globLinePoints.size(); i++)
	{
		globLinePoints[i].point1 *= transformation;
		globLinePoints[i].point2 *= transformation;
	}
	DisplayLineList();
}

/* Transformation dialog for all transformation buttons */
LRESULT CALLBACK TransformDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	TCHAR textfield1[128];
	TCHAR textfield2[128];
	TCHAR textfield3[128];
	TCHAR textfield4[128];
	LPTSTR endPtr;
	int dlgCheck;

	switch (message)
	{
	case WM_INITDIALOG:
	{
		dlgCheck = lParam;
		return TRUE;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case TRANSOK: // Translate dialog
			{
				GetDlgItemText(hwnd, TRANS_X_VAL, textfield1, 128);
				GetDlgItemText(hwnd, TRANS_Y_VAL, textfield2, 128);
				transConcat.push_back(DrawGraphics::BasicTranslate(_tcstod(textfield1, &endPtr), _tcstod(textfield2, &endPtr)));
				EndDialog(hwnd, IDOK);
				break;
			}
		case ROTOK: // Rotate (basic) dialog
			{
				GetDlgItemText(hwnd, ROTATE_VAL, textfield1, 128);
				transConcat.push_back(DrawGraphics::Rotate(_tcstod(textfield1, &endPtr), 0, 0));
				EndDialog(hwnd, IDOK);
				break;
			}
		case ADVROTOK: // Rotate (point) dialog
			{
				GetDlgItemText(hwnd, ROT_VAL, textfield1, 128);
				GetDlgItemText(hwnd, ROT_X, textfield2, 128);
				GetDlgItemText(hwnd, ROT_Y, textfield3, 128);
				transConcat.push_back(DrawGraphics::Rotate(_tcstod(textfield1, &endPtr), _tcstod(textfield2, &endPtr), _tcstod(textfield3, &endPtr)));
				EndDialog(hwnd, IDOK);
				break;
			}
		case SCALOK: // Scale (basic) dialog
			{
				GetDlgItemText(hwnd, SCALE_X_VAL, textfield1, 128);
				GetDlgItemText(hwnd, SCALE_Y_VAL, textfield2, 128);
				transConcat.push_back(DrawGraphics::Scale(_tcstod(textfield1, &endPtr), _tcstod(textfield2, &endPtr), 0, 0));
				EndDialog(hwnd, IDOK);
				break;
			}
		case ADVSCALOK: // Scale (point) dialog
			{
				GetDlgItemText(hwnd, ASCALE_X_VAL, textfield1, 128);
				GetDlgItemText(hwnd, ASCALE_Y_VAL, textfield2, 128);
				GetDlgItemText(hwnd, SCALE_X_POINT, textfield3, 128);
				GetDlgItemText(hwnd, SCALE_Y_POINT, textfield4, 128);
				transConcat.push_back(DrawGraphics::Scale(_tcstod(textfield1, &endPtr), _tcstod(textfield2, &endPtr), _tcstod(textfield3, &endPtr), _tcstod(textfield4, &endPtr)));
				EndDialog(hwnd, IDOK);
				break;
			}
		case IDCANCEL: // Cancel button, abort transformation
			EndDialog(hwnd, IDCANCEL);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}