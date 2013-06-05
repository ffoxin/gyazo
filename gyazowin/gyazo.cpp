/*
* gyazo.cpp
* 
* Entry point of gyazowin application
* 
*/

// System headers
#include <windows.h>
#include <Shlwapi.h>
#include <tchar.h>

// Project headers
#include "gyazolib.h"
#include "resource.h"
#include "util.h"
using namespace Gyazo;

// Constants
LPCTSTR szTitle				= _T("Gyazo");		// Text in the title bar
LPCTSTR szWindowMainClass	= _T("GYAZOWINM");	// Main window class name
LPCTSTR szWindowLayerClass	= _T("GYAZOWINL");	// Layer window class name
LPCTSTR szWindowCursorClass	= _T("GYAZOWINC");	// Cursor window class name

// Globals
HINSTANCE hInstance;				// Application instance
HWND hClipWnd;
HWND hCursorWnd;
Gyazo::Size coordSize;
Gyazo::Rect cursorRect;
Gyazo::Size cursorPos;

int screenOffsetX, screenOffsetY;	// virtual screen offset
int screenWidth, screenHeight;		// virtual screen size

// Declarations
ATOM				RegisterGyazoClass(HINSTANCE);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProcMain(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	WndProcClip(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	WndProcCursor(HWND, UINT, WPARAM, LPARAM);


void				DrawRubberband(HDC hdc, LPRECT newRect);
void				DrawCoordinates(HDC hdc, LPRECT newRect);
int					DrawLabel(HDC hdc, Gyazo::Size textPos, LPCTSTR sText, int nText);

// Entry point
int APIENTRY _tWinMain(HINSTANCE hInstance,
					   HINSTANCE hPrevInstance,
					   LPTSTR lpCmdLine,
					   int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	MSG msg;

	// Get app directory
	TCHAR szThisPath[MAX_PATH];
	GetModuleFileName(NULL, szThisPath, MAX_PATH);
	PathRemoveFileSpec(szThisPath);

	// Set the current directory to app directory
	SetCurrentDirectory(szThisPath);

	// Upload file if it specified as an argument
	if (__argc == 2)
	{
		// Exit by file upload
		LPCTSTR fileArg = __targv[1];
		if (IsPng(fileArg))
		{
			// PNG to upload directly
			UploadFile(fileArg);
		}
		else
		{
			// Convert to PNG format
			TCHAR tmpDir[MAX_PATH], tmpFile[MAX_PATH];
			GetTempPath(MAX_PATH, tmpDir);
			GetTempFileName(tmpDir, _T("gya"), 0, tmpFile);

			if (ConvertPng(tmpFile, fileArg))
			{
				//Upload
				UploadFile(tmpFile);
			}
			else
			{
				// Can not be converted into PNG
				ErrorMessage(_T("Cannot convert this image"));
			}
			DeleteFile(tmpFile);
		}
		return TRUE;
	}

	// To register a window class
	RegisterGyazoClass(hInstance);

	// I run the application initialization :
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	// Main message loop :
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}

// To register a window class
ATOM RegisterGyazoClass(HINSTANCE hInstance)
{
	WNDCLASS wc;

	// Main window
	wc.style			= 0;							// Do not send a WM_PAINT
	wc.lpfnWndProc		= WndProcMain;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor			= LoadCursor(NULL, IDC_CROSS);	// + Cursor
	wc.hbrBackground	= 0;							// Background
	wc.lpszMenuName		= 0;
	wc.lpszClassName	= szWindowMainClass;

	RegisterClass(&wc);

	// Layer window
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= WndProcClip;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor			= LoadCursor(NULL, IDC_CROSS);	// + Cursor
	wc.hbrBackground	= (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName		= 0;
	wc.lpszClassName	= szWindowLayerClass;

	RegisterClass(&wc);

	// Cursor window
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= WndProcCursor;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor			= LoadCursor(NULL, IDC_CROSS);	// + Cursor
	wc.hbrBackground	= (HBRUSH) GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName		= 0;
	wc.lpszClassName	= szWindowCursorClass;

	return RegisterClass(&wc);
}

// (I covered with a window full screen) initialization of instance
BOOL InitInstance(HINSTANCE hInst, int nCmdShow)
{
	HWND hWnd;
	hInstance = hInst; // I will store instances processing in a global variable .

	// It covers the entire virtual screen
	screenOffsetX = GetSystemMetrics(SM_XVIRTUALSCREEN);
	screenOffsetY = GetSystemMetrics(SM_YVIRTUALSCREEN);
	screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	// Completely ni shi taウィthrough clothウをmake ru nn
	hWnd = CreateWindowEx(
		WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST
#if (_WIN32_WINNT >= 0x0500)
		| WS_EX_NOACTIVATE
#endif
		,
		szWindowMainClass, NULL, WS_POPUP,
		0, 0, 0, 0,
		NULL, NULL, hInst, NULL);

	// Did not make ... ?
	if (!hWnd)
	{
		return FALSE;
	}

	// I cover the entire screen
	MoveWindow(hWnd, screenOffsetX, screenOffsetY, screenWidth, screenHeight, FALSE);

	// (I troubled and is combed SW_MAXIMIZE) ignore the nCmdShow
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);

	// ESC key detection timer
	SetTimer(hWnd, 1, 100, NULL);

	// You can layer window
	hClipWnd = CreateWindowEx(
		WS_EX_TOOLWINDOW
#if (_WIN32_WINNT >= 0x0500)
		| WS_EX_LAYERED | WS_EX_NOACTIVATE
#endif
		,
		szWindowLayerClass, NULL, WS_POPUP,
		100, 100, 300, 300,
		hWnd, NULL, hInst, NULL);

	SetLayeredWindowAttributes(hClipWnd, RGB(255, 0, 0), 100, LWA_COLORKEY | LWA_ALPHA);

	hCursorWnd = CreateWindowEx(
		WS_EX_TOOLWINDOW
#if (_WIN32_WINNT >= 0x0500)
		| WS_EX_LAYERED | WS_EX_NOACTIVATE
#endif
		,
		szWindowCursorClass, NULL, WS_POPUP,
		100, 100, 300, 300,
		hWnd, NULL, hInst, NULL);

	SetLayeredWindowAttributes(hCursorWnd, RGB(255, 0, 0), 100, LWA_COLORKEY | LWA_ALPHA);

	return TRUE;
}

// Drawing rubber band
void DrawRubberband(HDC hdc, LPRECT newRect)
{
	static bool firstDraw = true;	// First does not perform to erase the previous band

	if (firstDraw)
	{
		// View clip window
		ShowWindow(hClipWnd, SW_SHOW);
		UpdateWindow(hClipWnd);

		firstDraw = false;
	}

	if (newRect == NULL)
	{
		// Hide clip window
		ShowWindow(hClipWnd, SW_HIDE);
		return;
	}

	// Check coordinate
	Gyazo::Rect clipRect = *newRect;
	if (clipRect.right < clipRect.left)
	{
		swap(clipRect.right, clipRect.left);
	}
	if (clipRect.bottom < clipRect.top)
	{
		swap(clipRect.bottom, clipRect.top);
	}

	MoveWindow(hClipWnd, 
		clipRect.left, 
		clipRect.top, 
		clipRect.right - clipRect.left + 1, 
		clipRect.bottom - clipRect.top + 1, 
		TRUE);
}

// Drawing rubber band
void DrawCoordinates(HDC hdc, LPRECT newRect)
{
	static bool firstDraw = true;	// First does not perform to erase the previous band

	if (firstDraw)
	{
		// View coordinate window
		ShowWindow(hCursorWnd, SW_SHOW);
		UpdateWindow(hCursorWnd);

		firstDraw = false;
	}

	if (newRect == NULL)
	{
		// Hide the coordinate window
		ShowWindow(hCursorWnd, SW_HIDE);
		return;
	}
	
	MoveWindow(hCursorWnd, 
		newRect->left, 
		newRect->top, 
		newRect->right + 1, 
		newRect->bottom + 1, 
		TRUE);
}

int DrawLabel(HDC hdc, Gyazo::Size textPos, LPCTSTR sText, int nText)
{
	int result;

	SetTextColor(hdc, RGB(0, 0, 0));
	result = TextOut(hdc, textPos.cx + 1, textPos.cy + 1, sText, nText);
	if (result == 0)
	{
		return result;
	}

	SetTextColor(hdc, RGB(255, 255, 255));
	result = TextOut(hdc, textPos.cx, textPos.cy, sText, nText);
	if (result == 0)
	{
		return result;
	}

	return result;
}

// Layer window procedure
LRESULT CALLBACK WndProcClip(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_ERASEBKGND:
		{
			Gyazo::Rect winRect;
			GetWindowRect(hWnd, winRect);
			Gyazo::Size clipRect(winRect.right - winRect.left, winRect.bottom - winRect.top);

			HDC hdc = GetDC(hWnd);
			HBRUSH hBrush = CreateSolidBrush(RGB(100, 100, 100));
			SelectObject(hdc, hBrush);
			HPEN hPen = CreatePen(PS_DASH, 1, RGB(255, 255, 255));
			SelectObject(hdc, hPen);
			Rectangle(hdc, 0, 0, clipRect.cx, clipRect.cy);

			// The output size of the rectangle
			int fontHeight = MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72);
			HFONT hFont = CreateFont(
				(-1) * fontHeight,	// Font height
				0,					// Text parcels
				0,					// Angle of text
				0,					// Angle of the x-axis and the baseline
				FW_REGULAR,			// The font weight (thickness)
				FALSE,				// Italic
				FALSE,				// Underline
				FALSE,				// Strike through
				ANSI_CHARSET,		// Character set
				OUT_DEFAULT_PRECIS,	// Output Accuracy
				CLIP_DEFAULT_PRECIS,// Clipping accuracy
				PROOF_QUALITY,		// Output Quality
				FIXED_PITCH | FF_MODERN, // Family pitch
				_T("Tahoma")		// Face name
				);

			SelectObject(hdc, hFont);

			SetBkMode(hdc, TRANSPARENT);

			const int border = 5;

			Gyazo::Size textSize, textPos;
			TCHAR sText[100];
			size_t nText;

			// Draw top left coordinates (top left corner)
			_stprintf_s(sText, _T("%d:%d"), winRect.left, winRect.top);
			nText = _tcslen(sText);
			textPos.Set(border, border);

			DrawLabel(hdc, textPos, sText, nText);

			// Draw bottom right coordinates (bottom right corner)
			_stprintf_s(sText, _T("%d:%d"), winRect.right, winRect.bottom);
			nText = _tcslen(sText);
			GetTextExtentPoint(hdc, sText, nText, textSize);
			textPos.Set(clipRect.cx - textSize.cx - border, 
				clipRect.cy - textSize.cy - border);

			DrawLabel(hdc, textPos, sText, nText);

			// Draw crop size (center)
			_stprintf_s(sText, _T("%d:%d"), clipRect.cx, clipRect.cy);
			nText = _tcslen(sText);
			GetTextExtentPoint(hdc, sText, nText, textSize);
			textPos.Set((clipRect.cx - textSize.cx) / 2, 
				(clipRect.cy - textSize.cy) / 2);

			DrawLabel(hdc, textPos, sText, nText);

			// Release resources
			DeleteObject(hPen);
			DeleteObject(hBrush);
			DeleteObject(hFont);
			ReleaseDC(hWnd, hdc);

			return TRUE;
		}
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return FALSE;
}

// Cursor window procedure
LRESULT CALLBACK WndProcCursor(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_ERASEBKGND:
		{
			HDC hdc = GetDC(hWnd);
			HBRUSH hBrush = CreateSolidBrush(RGB(100, 100, 100));
			SelectObject(hdc, hBrush);
			HPEN hPen = CreatePen(PS_DASH, 1, RGB(255, 255, 255));
			SelectObject(hdc, hPen);
			Rectangle(hdc, 0, 0, coordSize.cx + 4, coordSize.cy);

			// The output size of the rectangle
			int fontHeight = MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72);
			HFONT hFont = CreateFont(
				(-1) * fontHeight,	// Font height
				0,					// Text parcels
				0,					// Angle of text
				0,					// Angle of the x-axis and the baseline
				FW_REGULAR,			// The font weight (thickness)
				FALSE,				// Italic
				FALSE,				// Underline
				FALSE,				// Strike through
				ANSI_CHARSET,		// Character set
				OUT_DEFAULT_PRECIS,	// Output Accuracy
				CLIP_DEFAULT_PRECIS,// Clipping accuracy
				PROOF_QUALITY,		// Output Quality
				FIXED_PITCH | FF_MODERN, // Family pitch
				_T("Tahoma")		// Face name
				);

			SelectObject(hdc, hFont);

			SetBkMode(hdc, TRANSPARENT);

			Gyazo::Size textPos;
			TCHAR sText[100];

			// Draw mouse coordinates
			_stprintf_s(sText, _T("%d:%d"), cursorPos.cx, cursorPos.cy);
			size_t nText = _tcslen(sText);
			GetTextExtentPoint(hdc, sText, nText, coordSize);
			textPos.Set(2, 0);

			DrawLabel(hdc, textPos, sText, nText);

			// Release resources
			DeleteObject(hPen);
			DeleteObject(hBrush);
			DeleteObject(hFont);
			ReleaseDC(hWnd, hdc);

			return TRUE;
		}
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return FALSE;
}

// Window procedure
LRESULT CALLBACK WndProcMain(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;

	static bool isClip = false;
	static Gyazo::Rect clipRect;

	switch (message)
	{
	case WM_RBUTTONDOWN:
		// Cancellation
		DestroyWindow(hWnd);
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;

	case WM_TIMER:
		// Detection of the ESC key is pressed
		if (GetKeyState(VK_ESCAPE) & 0x8000)
		{
			DestroyWindow(hWnd);
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;

	case WM_MOUSEMOVE:
		{
			cursorPos.Set(LOWORD(lParam) + screenOffsetX, 
				HIWORD(lParam) + screenOffsetY);

			hdc = GetDC(NULL);

			if (isClip)
			{
				// Set the new coordinate
				clipRect.right	= cursorPos.cx;
				clipRect.bottom	= cursorPos.cy;

				DrawRubberband(hdc, clipRect);
			}
			else
			{
				const Gyazo::Size offset(5, 5);

				cursorRect.Set(cursorPos.cx + offset.cx, 
					cursorPos.cy + offset.cy, 
					coordSize.cx + 4, 
					coordSize.cy);

				DrawCoordinates(hdc, cursorRect);
				SendMessage(hCursorWnd, WM_ERASEBKGND, NULL, NULL);
			}

			ReleaseDC(NULL, hdc);

			SetCapture(hWnd);
		}
		break;


	case WM_LBUTTONDOWN:
		{
			// Clip start
			isClip = true;

			ReleaseCapture();

			// hide windows with mouse coordinates
			HDC hdc = GetDC(NULL);
			DrawCoordinates(hdc, NULL);
			ReleaseDC(NULL, hdc);

			// Set the initial position
			clipRect.left	= LOWORD(lParam) + screenOffsetX;
			clipRect.top	= HIWORD(lParam) + screenOffsetY;

			// Capture the mouse
			SetCapture(hWnd);
		}
		break;

	case WM_LBUTTONUP:
		{
			// Clip end
			isClip = false;

			// And release the mouse capture
			ReleaseCapture();

			// Set the new coordinate
			clipRect.right	= LOWORD(lParam) + screenOffsetX;
			clipRect.bottom	= HIWORD(lParam) + screenOffsetY;

			// Screen ni direct drawing,っte -shaped
			HDC hdc = GetDC(NULL);

			// I turn off the line
			DrawRubberband(hdc, NULL);

			// Check coordinate
			if (clipRect.right < clipRect.left)
			{
				swap(clipRect.right, clipRect.left);
			}
			if (clipRect.bottom < clipRect.top)
			{
				swap(clipRect.bottom, clipRect.top);
			}

			// Image capture
			int iWidth	= clipRect.right - clipRect.left + 1;
			int iHeight	= clipRect.bottom - clipRect.top + 1;

			if (iWidth == 0 || iHeight == 0)
			{
				// It is not already in the image , it is not nothing
				ReleaseDC(NULL, hdc);
				DestroyWindow(hWnd);
				break;
			}

			// Create a bitmap buffer
			HBITMAP	newBMP	= CreateCompatibleBitmap(hdc, iWidth, iHeight);
			HDC		newDC	= CreateCompatibleDC(hdc);

			// Associated
			SelectObject(newDC, newBMP);

			// Portraitをobtain
			BitBlt(newDC, 0, 0, iWidth, iHeight, 
				hdc, clipRect.left, clipRect.top, SRCCOPY);

			// I hide the window !
			ShowWindow(hWnd, SW_HIDE);

			// The determination of the temporary file name
			TCHAR tmpDir[MAX_PATH], tmpFile[MAX_PATH];
			GetTempPath(MAX_PATH, tmpDir);
			GetTempFileName(tmpDir, _T("gya"), 0, tmpFile);

			if (SavePng(tmpFile, newBMP))
			{
				UploadFile(tmpFile);
			}
			else
			{
				// PNG save failed
				ErrorMessage(_T("Cannot save png image"));
			}

			// Clean up
			DeleteFile(tmpFile);

			DeleteDC(newDC);
			DeleteObject(newBMP);

			ReleaseDC(NULL, hdc);
			DestroyWindow(hWnd);
			PostQuitMessage(0);
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

