#include <windows.h>  
#include <tchar.h>
#include <time.h>
#include <ShObjIdl.h>
#include <fstream>
#include <iostream>
#include <string>

/* App name */
static TCHAR szAppName[] = _T("CS4050Program");
/* App title bar label */
static TCHAR szTitle[] = _T("Image Encryption Program");
static int LEFTMARGIN = 5;
static int TOPMARGIN = 100;

HWND mainWinHwd = NULL;			// Main window handle
HINSTANCE hInst;				// Main window instance
RECT mainWinRect;				// Main window rect
HWND imageBox, fileBox;			// Path string boxes
HWND hWndComboBox;				// Bit choice combo box

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void GetFilePath(HWND textbox);
void EncodeImage(int bits);
void DecodeImage(int bits);
void SetPixelBits(BYTE * Buffer, int width, int height, BYTE * textArray, int textLength, int bits);
bool SaveBMP(BYTE* Buffer, int width, int height, long paddedsize, LPCTSTR bmpfile);
bool SaveTextFile(BYTE* Buffer, unsigned long long length, LPCTSTR textFile);
BYTE * ReadPixelBits(BYTE * imgBuffer, int imgWidth, int imgHeight, unsigned long long * textLength, int bits);
BYTE * LoadTextFile(int * length, LPCTSTR textFile);
BYTE* LoadBMP(int* width, int* height, LPCTSTR bmpfile);
BYTE* ConvertBMPToRGBBuffer(BYTE* Buffer, int width, int height);
BYTE* ConvertRGBToBMPBuffer(BYTE* Buffer, int width, int height, long* newsize);

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
		400, 155,	// Width, height
		NULL,
		NULL,
		hInst,
		NULL
	);

	/* Check for window creation failure */
	if (!hWnd)
	{
		MessageBox(NULL, _T("Window creation failed!"), _T("Graphics Program Error"), NULL);
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
	PAINTSTRUCT ps;
	HDC hdc;
	GetClientRect(hWnd, &mainWinRect); // Get the window size for later

	switch (message)
	{
		case WM_CREATE:
		{

			/* Image Select button */
			CreateWindow(
				L"BUTTON",
				L"Img",
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
				10,         // x pos 
				10,         // y pos
				40,        // width
				40,         // height
				hWnd,		// parent
				(HMENU)1,
				hInst,
				NULL
			);

			/* Text File Select button */
			CreateWindow(
				L"BUTTON",
				L"Txt",
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
				10,         // x pos 
				55,         // y pos
				40,        // width
				40,         // height
				hWnd,		// parent
				(HMENU)2,
				hInst,
				NULL
			);

			/* Image path text box */
			imageBox = CreateWindow(
				WC_EDIT,
				TEXT(""),
				WS_CHILD | WS_BORDER | WS_VISIBLE | ES_LEFT | WS_DISABLED,
				55,	// x pos
				15,			// y pos
				250,			// width
				20,			// height
				hWnd,		//parent
				(HMENU)5,
				hInst,
				NULL
			);

			/* Text file path text box */
			fileBox = CreateWindow(
				WC_EDIT,
				TEXT(""),
				WS_CHILD | WS_BORDER | WS_VISIBLE | ES_LEFT | WS_DISABLED,
				55,	// x pos
				60,			// y pos
				250,			// width
				20,			// height
				hWnd,		//parent
				(HMENU)6,
				hInst,
				NULL
			);

			/* Encode Button */
			CreateWindow(
				L"BUTTON",
				L"Encode",
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
				mainWinRect.right - 75,         // x pos 
				10,         // y pos
				70,        // width
				40,         // height
				hWnd,		// parent
				(HMENU)3,
				hInst,
				NULL
			);

			/* Decode Button */
			CreateWindow(
				L"BUTTON",
				L"Decode",
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
				mainWinRect.right - 75,         // x pos 
				55,         // y pos
				70,        // width
				40,         // height
				hWnd,		// parent
				(HMENU)4,
				hInst,
				NULL
			);

			/* Bit label for BitBox */
			CreateWindow(
				TEXT("STATIC"),
				TEXT("Bits:"),
				WS_CHILD | WS_VISIBLE | SS_LEFT | WS_EX_TRANSPARENT,
				230,	// x pos
				90,					// y pos
				30,					// width
				20,					// height
				hWnd,				// parent
				(HMENU)8,
				hInst,
				NULL);

			/* Bit choice box */
			hWndComboBox = CreateWindow(
				WC_COMBOBOX,
				TEXT(""),
				CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_VISIBLE,
				265,			// x pos
				85,			// y pos
				40,		// width
				200,		// height
				hWnd,		//parent
				(HMENU)5,
				hInst,
				NULL
			);
			
			/* Check for any window creation failures */
			if (!hWndComboBox || !imageBox || !fileBox)
			{
				MessageBox(NULL, _T("Child window creation failed!"), _T("Graphics Drawing Error"), NULL);
				return 1;
			}

			/* Populate the bit combo box with selections*/
			TCHAR BitBox[8][3] = { TEXT("1"), TEXT("2"), TEXT("3"), TEXT("4"), TEXT("5"), TEXT("6"), TEXT("7"), TEXT("8") };
			for (int k = 0; k <= 7; k++) { SendMessage(hWndComboBox, CB_ADDSTRING, 0, (LPARAM)BitBox[k]); }
			SendMessage(hWndComboBox, CB_SETCURSEL, (WPARAM)0, (LPARAM)0); // Set initial item in list

			break;
		}
		/* Repaint the window when necessary */
		case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			EndPaint(hWnd, &ps);
			break;
		}
		case WM_COMMAND:
		{
			if (LOWORD(wParam) == 1)
			{
				GetFilePath(imageBox);
			}
			else if (LOWORD(wParam) == 2)
			{
				GetFilePath(fileBox);
			}
			else if (LOWORD(wParam) == 3)
			{
				int bitChoice = SendMessage(hWndComboBox, CB_GETCURSEL, 0, 0); // Get choice from combobox
				EncodeImage(bitChoice + 1);
			}
			else if (LOWORD(wParam) == 4)
			{
				int bitChoice = SendMessage(hWndComboBox, CB_GETCURSEL, 0, 0); // Get choice from combobox
				DecodeImage(bitChoice + 1);
			}
			break;
		}
		case WM_CTLCOLORSTATIC:
			if (GetDlgItem(hWnd, 8) == (HWND)lParam) // Color the bit label because Windows is dumb
			{
				SetBkMode((HDC)wParam, TRANSPARENT);
				return (BOOL)GetStockObject(HOLLOW_BRUSH);
			}
			break;
		case WM_DESTROY:
		{
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
* Populates the given text box with the file path when called
*/
void GetFilePath(HWND textbox)
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

					// Load up the filename
					if (SUCCEEDED(hr))
					{
						SetWindowText(textbox, pszFilePath);
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

/* 
* Encodes the text file into the image
*/
void EncodeImage(int bits)
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
						int x, y, textLength;
						long s2;
						TCHAR imgInput[300];
						TCHAR textFileInput[300];
						GetWindowText(imageBox, imgInput, 300);
						GetWindowText(fileBox, textFileInput, 300);
						BYTE* a = LoadBMP(&x, &y, imgInput);
						BYTE* b = ConvertBMPToRGBBuffer(a, x, y);
						BYTE * textArray = LoadTextFile(&textLength, textFileInput);
						SetPixelBits(b, x, y, textArray, textLength, bits);
						BYTE* c = ConvertRGBToBMPBuffer(b, x, y, &s2);
						SaveBMP(c, x, y, s2, pszFilePath);
						delete[] a;
						delete[] b;
						delete[] c;
						delete[] textArray;

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

/*
* Decodes text from a given image file
*/
void DecodeImage(int bits)
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
						int x, y;
						unsigned long long textLength;
						TCHAR imgInput[300];
						GetWindowText(imageBox, imgInput, 300);
						BYTE* a = LoadBMP(&x, &y, imgInput);
						BYTE* b = ConvertBMPToRGBBuffer(a, x, y);
						BYTE * textArray = ReadPixelBits(b, x, y, &textLength, bits);
						SaveTextFile(textArray, textLength, pszFilePath);
						delete[] a;
						delete[] b;
						delete[] textArray;
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

/*
* Sets the pixel bits in the image to the given text array, sets final character to NULL
*/
void SetPixelBits(BYTE * imgBuffer, int imgWidth, int imgHeight, BYTE * textArray, int textLength, int bits)
{
	int imgArraySize = imgWidth * imgHeight * 3;
	bool finished = false;
	int bitCounter = 0;
	int textArrayPlace = 0;
	char currentChar;
	currentChar = textArray[textArrayPlace];
	for (int i = 0; i < imgArraySize; i++)
	{
		for (int j = 0; j < bits; j++)
		{
			imgBuffer[i] ^= (-((currentChar >> 0) & 1) ^ imgBuffer[i]) & (1 << j);

			bitCounter++;

			if (bitCounter == 8)
			{
				if (finished)
					return;

				textArrayPlace++;
				bitCounter = 0;

				if (textArrayPlace < textLength)
					currentChar = textArray[textArrayPlace];
				else
				{
					finished = true;
					currentChar = NULL;
					bitCounter = 0;
				}
			}
			else
			{
				currentChar = currentChar >> 1;
			}
		}
	}
}

/*
* Reads the pixels in the image for text data, stops at NULL char
*/
BYTE * ReadPixelBits(BYTE * imgBuffer, int imgWidth, int imgHeight, unsigned long long * textLength, int bits)
{
	int imgArraySize = imgWidth * imgHeight * 3;
	int bitCounter = 0;
	int textArrayPlace = 0;
	char currentChar = 0;
	BYTE* buffer = new BYTE[imgArraySize]; // Text buffer of maximum possible size
	for (int i = 0; i < imgArraySize; i++)
	{
		for (int j = 0; j < bits; j++)
		{
			currentChar ^= (-((imgBuffer[i] >> j) & 1) ^ currentChar) & (1 << 7);

			bitCounter++;

			if (bitCounter == 8)
			{
				if (currentChar == NULL)
				{
					*textLength = textArrayPlace;
					return buffer;
				}

				buffer[textArrayPlace] = currentChar;
				textArrayPlace++;
				bitCounter = 0;
			}
			else
			{
				currentChar = currentChar >> 1;
			}
		}
	}
	*textLength = textArrayPlace;
	return buffer;
}

/*
* Load a valid BMP from a file
*/
BYTE* LoadBMP(int* width, int* height, LPCTSTR bmpfile)
{
	BITMAPFILEHEADER bmpheader;
	BITMAPINFOHEADER bmpinfo;
	DWORD bytesRead;

	HANDLE file = CreateFile(bmpfile, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (file == NULL)
		return NULL; // coudn't open file


	// read file header
	if (ReadFile(file, &bmpheader, sizeof(BITMAPFILEHEADER), &bytesRead, NULL) == false)
	{
		CloseHandle(file);
		return NULL;
	}

	//read bitmap info
	if (ReadFile(file, &bmpinfo, sizeof(BITMAPINFOHEADER), &bytesRead, NULL) == false)
	{
		CloseHandle(file);
		return NULL;
	}

	// check if file is actually a bmp
	if (bmpheader.bfType != 'MB')
	{
		CloseHandle(file);
		return NULL;
	}

	// get image measurements
	*width = bmpinfo.biWidth;
	*height = abs(bmpinfo.biHeight);

	// check if bmp is uncompressed and 24 bits
	if (bmpinfo.biCompression != BI_RGB || bmpinfo.biBitCount != 24)
	{
		CloseHandle(file);
		return NULL;
	}

	// create buffer to hold data
	long size = bmpheader.bfSize - bmpheader.bfOffBits;
	BYTE* buffer = new BYTE[size];
	// move file pointer to start of bitmap data
	SetFilePointer(file, bmpheader.bfOffBits, NULL, FILE_BEGIN);
	// read bmp data
	if (ReadFile(file, buffer, size, &bytesRead, NULL) == false)
	{
		delete[] buffer;
		CloseHandle(file);
		return NULL;
	}

	CloseHandle(file);
	return buffer;
}

/*
* Format and save the BMP to a file
*/
bool SaveBMP(BYTE* buffer, int width, int height, long paddedsize, LPCTSTR bmpfile)
{
	BITMAPFILEHEADER bmfh;
	BITMAPINFOHEADER info;
	memset(&bmfh, 0, sizeof(BITMAPFILEHEADER));
	memset(&info, 0, sizeof(BITMAPINFOHEADER));

	// fill the fileheader with data
	bmfh.bfType = 0x4d42;       // 0x4d42 = 'BM'
	bmfh.bfReserved1 = 0;
	bmfh.bfReserved2 = 0;
	bmfh.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + paddedsize;
	bmfh.bfOffBits = 0x36;		// number of bytes to start of bitmap bits

	// fill the infoheader
	info.biSize = sizeof(BITMAPINFOHEADER);
	info.biWidth = width;
	info.biHeight = height;
	info.biPlanes = 1;			// we only have one bitplane
	info.biBitCount = 24;		// RGB mode is 24 bits
	info.biCompression = BI_RGB;
	info.biSizeImage = 0;		// can be 0 for 24 bit images
	info.biXPelsPerMeter = 0x0ec4;     // paint and PSP use this values
	info.biYPelsPerMeter = 0x0ec4;
	info.biClrUsed = 0;			// we are in RGB mode and have no palette
	info.biClrImportant = 0;    // all colors are important

	// open the file to write to
	HANDLE file = CreateFile(bmpfile, GENERIC_WRITE, FILE_SHARE_READ,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == NULL)
	{
		CloseHandle(file);
		return false;
	}

	// write file header
	unsigned long bwritten;
	if (WriteFile(file, &bmfh, sizeof(BITMAPFILEHEADER), &bwritten, NULL) == false)
	{
		CloseHandle(file);
		return false;
	}
	// write infoheader
	if (WriteFile(file, &info, sizeof(BITMAPINFOHEADER), &bwritten, NULL) == false)
	{
		CloseHandle(file);
		return false;
	}
	// write image data
	if (WriteFile(file, buffer, paddedsize, &bwritten, NULL) == false)
	{
		CloseHandle(file);
		return false;
	}

	CloseHandle(file);
	return true;
}

/*
* Convert the BMP format to a simpler RGB format
*/
BYTE* ConvertBMPToRGBBuffer(BYTE* buffer, int width, int height)
{
	if ((buffer == NULL) || (width == 0) || (height == 0))
		return NULL;

	// find the number of padding bytes
	int padding = 0;
	int scanlinebytes = width * 3;

	while ((scanlinebytes + padding) % 4 != 0)     // DWORD = 4 bytes
		padding++;

	int psw = scanlinebytes + padding; // get the padded scanline width
	BYTE* newbuf = new BYTE[width*height * 3];

	// swap R and B and flip picture around because BMPs are weird
	long bufpos = 0;
	long newpos = 0;
	for (int y = 0; y < height; y++)
		for (int x = 0; x < 3 * width; x += 3)
		{
			newpos = y * 3 * width + x;
			bufpos = (height - y - 1) * psw + x;

			newbuf[newpos] = buffer[bufpos + 2];
			newbuf[newpos + 1] = buffer[bufpos + 1];
			newbuf[newpos + 2] = buffer[bufpos];
		}

	return newbuf;
}

/*
* Convert the RGB array back to the format used by bitmaps
*/
BYTE* ConvertRGBToBMPBuffer(BYTE* buffer, int width, int height, long* newsize)
{
	if ((buffer == NULL) || (width == 0) || (height == 0))
		return NULL;

	// find the number of padding bytes
	int padding = 0;
	int scanlinebytes = width * 3;

	while ((scanlinebytes + padding) % 4 != 0)     // DWORD = 4 bytes
		padding++;

	int psw = scanlinebytes + padding; // get padded scanline width
	*newsize = height * psw;
	BYTE* newbuf = new BYTE[*newsize];

	// fill buffer with zero bytes to save work later
	memset(newbuf, 0, *newsize);

	// swap R and B and flip picture around because BMPs are weird
	long bufpos = 0;
	long newpos = 0;
	for (int y = 0; y < height; y++)
		for (int x = 0; x < 3 * width; x += 3)
		{
			bufpos = y * 3 * width + x;				// position in original buffer
			newpos = (height - y - 1) * psw + x;	// position in padded buffer

			newbuf[newpos] = buffer[bufpos + 2];		// swap r and b
			newbuf[newpos + 1] = buffer[bufpos + 1];	// not g
			newbuf[newpos + 2] = buffer[bufpos];		// swap b and r
		}

	return newbuf;
}

/*
* Load a text file and put it in an array
*/
BYTE * LoadTextFile(int * length, LPCTSTR textFile)
{
	DWORD bytesRead;
	HANDLE file = CreateFile(textFile, GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (file == NULL)
		return NULL; // coudn't open file

	*length = GetFileSize(file, NULL);
	BYTE* buffer = new BYTE[*length];

	if (ReadFile(file, buffer, *length, &bytesRead, NULL) == false)
	{
		delete[] buffer;
		CloseHandle(file);
		return NULL;
	}

	// success, close file and return buffer
	CloseHandle(file);
	return buffer;
}

/*
* Save a text file to disk
*/
bool SaveTextFile(BYTE* Buffer, unsigned long long length, LPCTSTR textFile)
{
	unsigned long bwritten;
	HANDLE file = CreateFile(textFile, GENERIC_WRITE, FILE_SHARE_READ,
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (file == NULL)
	{
		CloseHandle(file);
		return false;
	}

	// write text data
	if (WriteFile(file, Buffer, length, &bwritten, NULL) == false)
	{
		CloseHandle(file);
		return false;
	}

	CloseHandle(file);
	return true;
}