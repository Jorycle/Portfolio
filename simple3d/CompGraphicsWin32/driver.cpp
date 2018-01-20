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

struct edges
{
	int end1;
	int end2;
};

/* App name */
static TCHAR szAppName[] = _T("CS4050Program");
/* App title bar label */
static TCHAR szTitle[] = _T("Graphics Drawing Program");
static int LEFTMARGIN = 10;
static int RIGHTMARGIN = 50;

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

HWND verticeList;					// Display window for lines
std::string outputVertices;			// Line copy for file save
std::string outputLines;
std::vector<Matrix> vertexTable;	// Vector of all vertices
std::vector<edges> edgeTable;		// Vector of all edges, referring to vertice table
std::vector<Matrix> transConcat;

Matrix camera({ {6,8,7.5,1} });		// Camera position
Matrix cameraDirection({ {0, 0, 0, 1} }); // Where the camera is pointing
Matrix cameraUp({ { 0,0,1,1 } });	 // Up vector of the camera

LRESULT CALLBACK TransformDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void DrawButton(COLORREF colorRef);
void LoadLines();
void ApplyTransformation();
void DisplayLineList();
void SaveLines();

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
		850, 700,
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
	static HWND drawSpace;
	static HWND locX, locY, locZ;
	static HWND dirX, dirY, dirZ;
	static HWND upX, upY, upZ;
	COLORREF colorRef;
	PAINTSTRUCT ps;
	HDC hdc;
	GetClientRect(hWnd, &mainWinRect); // Get the window size for later

	switch (message)
	{
		case WM_CREATE:
		{

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

			// Camera orientation fields
			{
				/* Camera location label */
				CreateWindow(
					TEXT("STATIC"),
					TEXT("Cam Location (x, y, z)"),
					WS_CHILD | WS_VISIBLE | SS_LEFT | WS_EX_TRANSPARENT,
					mainWinRect.right - 155,	// x pos
					545,				// y pos
					140,				// width
					20,				// height
					hWnd,			// parent
					(HMENU)40,
					hInst,
					NULL);

				/* Camera location x field */
				locX = CreateWindow(
					L"EDIT",
					L"6",
					WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_BORDER | SS_SUNKEN,
					mainWinRect.right - 155,         // x pos 
					560,         // y pos
					40,        // width
					20,         // height
					hWnd,		// parent
					(HMENU)30,
					hInst,
					NULL
				);

				/* Camera location y field */
				locY = CreateWindow(
					L"EDIT",
					L"8",
					WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_BORDER | SS_SUNKEN,
					mainWinRect.right - 110,         // x pos 
					560,         // y pos
					40,        // width
					20,         // height
					hWnd,		// parent
					(HMENU)31,
					hInst,
					NULL
				);

				/* Camera location z field */
				locZ = CreateWindow(
					L"EDIT",
					L"7.5",
					WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_BORDER | SS_SUNKEN,
					mainWinRect.right - 65,         // x pos 
					560,         // y pos
					40,        // width
					20,         // height
					hWnd,		// parent
					(HMENU)32,
					hInst,
					NULL
				);

				/* Camera focus label */
				CreateWindow(
					TEXT("STATIC"),
					TEXT("Cam Focus (x, y, z)"),
					WS_CHILD | WS_VISIBLE | SS_LEFT | WS_EX_TRANSPARENT,
					mainWinRect.right - 155,	// x pos
					580,				// y pos
					140,				// width
					20,				// height
					hWnd,			// parent
					(HMENU)41,
					hInst,
					NULL);

				/* Camera focus x field */
				dirX = CreateWindow(
					L"EDIT",
					L"0",
					WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_BORDER | SS_SUNKEN,
					mainWinRect.right - 155,         // x pos 
					595,         // y pos
					40,        // width
					20,         // height
					hWnd,		// parent
					(HMENU)30,
					hInst,
					NULL
				);

				/* Camera focus y field */
				dirY = CreateWindow(
					L"EDIT",
					L"0",
					WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_BORDER | SS_SUNKEN,
					mainWinRect.right - 110,         // x pos 
					595,         // y pos
					40,        // width
					20,         // height
					hWnd,		// parent
					(HMENU)31,
					hInst,
					NULL
				);

				/* Camera focus z field */
				dirZ = CreateWindow(
					L"EDIT",
					L"0",
					WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_BORDER | SS_SUNKEN,
					mainWinRect.right - 65,         // x pos 
					595,         // y pos
					40,        // width
					20,         // height
					hWnd,		// parent
					(HMENU)32,
					hInst,
					NULL
				);

				/* Camera up label */
				CreateWindow(
					TEXT("STATIC"),
					TEXT("Cam Up (x, y, z)"),
					WS_CHILD | WS_VISIBLE | SS_LEFT | WS_EX_TRANSPARENT,
					mainWinRect.right - 155,	// x pos
					615,				// y pos
					140,				// width
					20,				// height
					hWnd,			// parent
					(HMENU)42,
					hInst,
					NULL);

				/* Camera up x field */
				upX = CreateWindow(
					L"EDIT",
					L"0",
					WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_BORDER | SS_SUNKEN,
					mainWinRect.right - 155,         // x pos 
					630,         // y pos
					40,        // width
					20,         // height
					hWnd,		// parent
					(HMENU)30,
					hInst,
					NULL);

				/* Camera up y field */
				upY = CreateWindow(
					L"EDIT",
					L"1",
					WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_BORDER | SS_SUNKEN,
					mainWinRect.right - 110,         // x pos 
					630,         // y pos
					40,        // width
					20,         // height
					hWnd,		// parent
					(HMENU)31,
					hInst,
					NULL);

				/* Camera up z field */
				upZ = CreateWindow(
					L"EDIT",
					L"0",
					WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_BORDER | SS_SUNKEN,
					mainWinRect.right - 65,         // x pos 
					630,         // y pos
					40,        // width
					20,         // height
					hWnd,		// parent
					(HMENU)32,
					hInst,
					NULL
				);
			}

			/* List of lines */
			verticeList = CreateWindowEx(
				0,
				L"EDIT",
				NULL,
				WS_CHILD | WS_OVERLAPPED | WS_VISIBLE | WS_VSCROLL | WS_BORDER | SS_SUNKEN | ES_READONLY | ES_MULTILINE,
				mainWinRect.right - 155,		// x pos
				RIGHTMARGIN + 95,					// y pos
				145,								// width
				400,			// height
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
			if (!drawSpace)
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
				TCHAR cameraLocX[5], cameraLocY[5], cameraLocZ[5];
				TCHAR cameraDirX[5], cameraDirY[5], cameraDirZ[5];
				TCHAR cameraUpX[5], cameraUpY[5], cameraUpZ[5];
				colorRef = RGB(0,0,0); //RGB is actually BGR, weird
				GetWindowText(locX, cameraLocX, 5);
				GetWindowText(locY, cameraLocY, 5);
				GetWindowText(locZ, cameraLocZ, 5);
				GetWindowText(dirX, cameraDirX, 5);
				GetWindowText(dirY, cameraDirY, 5);
				GetWindowText(dirZ, cameraDirZ, 5);
				GetWindowText(upX, cameraUpX, 5);
				GetWindowText(upY, cameraUpY, 5);
				GetWindowText(upZ, cameraUpZ, 5);
				camera(0, 0) = _tstof(cameraLocX);
				camera(0, 1) = _tstof(cameraLocY);
				camera(0, 2) = _tstof(cameraLocZ);
				cameraDirection(0, 0) = _tstof(cameraDirX);
				cameraDirection(0, 1) = _tstof(cameraDirY);
				cameraDirection(0, 2) = _tstof(cameraDirZ);
				cameraUp(0, 0) = _tstof(cameraUpX);
				cameraUp(0, 1) = _tstof(cameraUpY);
				cameraUp(0, 2) = _tstof(cameraUpZ);


				FillRect(bufferMemHDC, &drawRect, CreateSolidBrush(GetSysColor(COLOR_MENU)));
				ApplyTransformation();
				DrawButton(colorRef);

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
				vertexTable.clear();
				edgeTable.clear();
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
			if (GetDlgItem(hWnd, 40) == (HWND)lParam)
			{
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (BOOL)GetStockObject(HOLLOW_BRUSH);
			}
			else if (GetDlgItem(hWnd, 41) == (HWND)lParam)
			{
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (BOOL)GetStockObject(HOLLOW_BRUSH);
			}
			else if (GetDlgItem(hWnd, 42) == (HWND)lParam)
			{
				SetBkMode((HDC)wParam, TRANSPARENT);
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
void DrawButton(COLORREF colorRef)
{
	// Copy the table for the camera projection
	std::vector<Matrix> vertexCopy = vertexTable;
	Matrix vt = DrawGraphics::ViewingTransformation(camera, cameraDirection, cameraUp);
	for (int i = 0; i < vertexCopy.size(); i++)
	{
		vertexCopy[i] *= vt;
	}

	// Draw the lines
	for (int i = 0; i < edgeTable.size(); i++)
	{							   
		
		DrawGraphics::Line(vertexCopy[edgeTable[i].end1], vertexCopy[edgeTable[i].end2], colorRef);
		
		if (i % 1000 == 0)
			RedrawWindow(mainWinHwd, &drawRect, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	}
	
	// Drawing complete, force a repaint of the window
	InvalidateRect(mainWinHwd, &drawRect, FALSE);
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
						std::string inputType;
						int x, y, z;
						edges newEdge;
						int currentOffset = vertexTable.size();
						bool edgeMode = false, verticeMode = false;

						hFile >> inputType;
						if (inputType == "VERTICES")
							verticeMode = true;

						while (verticeMode)
						{

							hFile >> inputType;

							if (inputType == "EDGES")
							{
								verticeMode = false;
								edgeMode = true;
								break;
							}
							x = stoi(inputType);
							hFile >> y;
							hFile >> z;
							Matrix vertex({ {(double)x,(double)y, (double)z, 1} });
							vertexTable.push_back(vertex);
						}
							
						while (edgeMode && hFile >> x)
						{
							hFile >> y;
							newEdge.end1 = x + currentOffset;
							newEdge.end2 = y + currentOffset;
							edgeTable.push_back(newEdge);
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
						std::string outLines = "VERTICES\r\n" + outputVertices + "EDGES\r\n" + outputLines;

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
						WriteFile(hFile, outLines.c_str(), outLines.length(), &bWritten, 0);
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
	std::wstring wVertices;
	std::wstring wLines;
	
	// Iterate through the list of lines and put them in the window
	for (int i = 0; i < vertexTable.size(); i++)
	{
		wVertices += std::to_wstring((int)vertexTable[i](0, 0));
		wVertices += L" ";
		wVertices += std::to_wstring((int)vertexTable[i](0, 1));
		wVertices += L" ";
		wVertices += std::to_wstring((int)vertexTable[i](0, 2));
		wVertices += L"\r\n";
	}
	// Iterate through the list of lines and put them in the window
	for (int i = 0; i < edgeTable.size(); i++)
	{
		wLines += std::to_wstring((int)edgeTable[i].end1);
		wLines += L" ";
		wLines += std::to_wstring((int)edgeTable[i].end2);
		wLines += L"\r\n";
	}

	outputVertices.assign(wVertices.begin(), wVertices.end()); // Keep a copy for file saving purposes
	outputLines.assign(wLines.begin(), wLines.end());
	wide = wVertices.c_str(); // Convert because win32 api is retarded
	SetWindowText(verticeList, wide); // Set the text to the window
}

/* Apply given transformation to all lines */
void ApplyTransformation()
{
	Matrix transformation({ {1,0,0,0}, {0,1,0,0}, {0,0,1,0}, {0,0,0,1} });
	for (int i = 0; i < transConcat.size(); i++)
	{
		transformation *= transConcat[i];
	}
	transConcat.clear();
	for (int i = 0; i < vertexTable.size(); i++)
	{
		vertexTable[i] *= transformation;
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
	TCHAR textfield5[128];
	TCHAR textfield6[128];
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
				GetDlgItemText(hwnd, TRANS_Z_VAL, textfield3, 128);
				transConcat.push_back(DrawGraphics::Translate(_tcstod(textfield1, &endPtr), _tcstod(textfield2, &endPtr), _tcstod(textfield3, &endPtr)));
				EndDialog(hwnd, IDOK);
				break;
			}
		case ROTOK: // Rotate (basic) dialog
			{
				GetDlgItemText(hwnd, ROTATE_VAL, textfield1, 128);
				GetDlgItemText(hwnd, ROTATE_VALY, textfield2, 128);
				GetDlgItemText(hwnd, ROTATE_VALZ, textfield3, 128);
				transConcat.push_back(DrawGraphics::Rotate(_tcstod(textfield1, &endPtr), _tcstod(textfield2, &endPtr), _tcstod(textfield3, &endPtr), 0, 0, 0));
				EndDialog(hwnd, IDOK);
				break;
			}
		case ADVROTOK: // Rotate (point) dialog
			{
				GetDlgItemText(hwnd, ROT_VAL, textfield1, 128);
				GetDlgItemText(hwnd, ROT_VALY, textfield2, 128);
				GetDlgItemText(hwnd, ROT_VALZ, textfield3, 128);
				GetDlgItemText(hwnd, ROT_X, textfield4, 128);
				GetDlgItemText(hwnd, ROT_Y, textfield5, 128);
				GetDlgItemText(hwnd, ROT_Z, textfield6, 128);
				transConcat.push_back(DrawGraphics::Rotate(_tcstod(textfield1, &endPtr), _tcstod(textfield2, &endPtr), _tcstod(textfield3, &endPtr), _tcstod(textfield4, &endPtr), _tcstod(textfield5, &endPtr), _tcstod(textfield6, &endPtr)));
				EndDialog(hwnd, IDOK);
				break;
			}
		case SCALOK: // Scale (basic) dialog
			{
				GetDlgItemText(hwnd, SCALE_X_VAL, textfield1, 128);
				GetDlgItemText(hwnd, SCALE_Y_VAL, textfield2, 128);
				GetDlgItemText(hwnd, SCALE_Z_VAL, textfield3, 128);
				transConcat.push_back(DrawGraphics::Scale(_tcstod(textfield1, &endPtr), _tcstod(textfield2, &endPtr), _tcstod(textfield3, &endPtr), 0, 0, 0));
				EndDialog(hwnd, IDOK);
				break;
			}
		case ADVSCALOK: // Scale (point) dialog
			{
				GetDlgItemText(hwnd, ASCALE_X_VAL, textfield1, 128);
				GetDlgItemText(hwnd, ASCALE_Y_VAL, textfield2, 128);
				GetDlgItemText(hwnd, ASCALE_Z_VAL, textfield3, 128);
				GetDlgItemText(hwnd, SCALE_X_POINT, textfield4, 128);
				GetDlgItemText(hwnd, SCALE_Y_POINT, textfield5, 128);
				GetDlgItemText(hwnd, SCALE_Y_POINT, textfield6, 128);
				transConcat.push_back(DrawGraphics::Scale(_tcstod(textfield1, &endPtr), _tcstod(textfield2, &endPtr), _tcstod(textfield3, &endPtr), _tcstod(textfield4, &endPtr), _tcstod(textfield5, &endPtr), _tcstod(textfield6, &endPtr)));
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