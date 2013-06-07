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
#include "font.h"
#include "gyazolib.h"
#include "resource.h"
#include "stringconstants.h"
#include "util.h"
using namespace Gyazo;

const Gyazo::Size cursorWinOffset(5, 5);
const Gyazo::Size clipWinTextBorder(5, 5);

// Globals
HINSTANCE hInstance;				// Application instance
HWND hClipWnd;
HWND hCursorWnd;
Gyazo::Rect cursorWinRect;			// mouse coordinates window rect
Gyazo::Size cursorTextSize;			// mouse coordinates text size
Gyazo::Size cursorPos;				// mouse cursor position

Gyazo::Size screenOffset;			// virtual screen offset
Gyazo::Size screenSize;				// virtual screen size

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
			GetTempFileName(tmpDir, GYAZO_PREFIX, 0, tmpFile);

			if (ConvertPng(tmpFile, fileArg))
			{
				//Upload
				UploadFile(tmpFile);
			}
			else
			{
				// Can not be converted into PNG
				ErrorMessage(ERROR_CONVERT_IMAGE);
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
	ATOM result;

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

	result = RegisterClass(&wc);
	if (result == 0)
	{
		return 0;
	}

	// Clip window
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

	result = RegisterClass(&wc);
	if (result == 0)
	{
		return 0;
	}

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

	result = RegisterClass(&wc);
	if (result == 0)
	{
		return 0;
	}

	return result;
}

// (I covered with a window full screen) initialization of instance
BOOL InitInstance(HINSTANCE hInst, int nCmdShow)
{
	HWND hWnd;
	hInstance = hInst; // I will store instances processing in a global variable .

	// It covers the entire virtual screen
	screenOffset.Set(GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN));
	screenSize.Set(GetSystemMetrics(SM_CXVIRTUALSCREEN), GetSystemMetrics(SM_CYVIRTUALSCREEN));

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
	MoveWindow(hWnd, screenOffset.cx, screenOffset.cy, screenSize.cx, screenSize.cy, FALSE);

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
		0, 0, 0, 0,
		hWnd, NULL, hInst, NULL);

	SetLayeredWindowAttributes(hClipWnd, RGB(255, 0, 0), 100, LWA_COLORKEY | LWA_ALPHA);

	hCursorWnd = CreateWindowEx(
		WS_EX_TOOLWINDOW
#if (_WIN32_WINNT >= 0x0500)
		| WS_EX_LAYERED | WS_EX_NOACTIVATE
#endif
		,
		szWindowCursorClass, NULL, WS_POPUP,
		0, 0, 0, 0,
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
	
	Gyazo::Rect coordRect = *newRect;
	if (coordRect.left + coordRect.right > screenSize.cx)
	{
		coordRect.left = screenSize.cx - coordRect.right;
	}
	if (coordRect.top + coordRect.bottom > screenSize.cy)
	{
		coordRect.top = screenSize.cy - coordRect.bottom;
	}

	MoveWindow(hCursorWnd, 
		coordRect.left, 
		coordRect.top, 
		coordRect.right, 
		coordRect.bottom, 
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
			SelectObject(hdc, Gyazo::Font::GetFont(fontHeight));

			SetBkMode(hdc, TRANSPARENT);

			Gyazo::Size textSize, textPos;
			TCHAR sText[100];
			size_t nText;

			// Draw top left coordinates (top left corner)
			_stprintf_s(sText, GYAZO_POINT_FORMAT, winRect.left, winRect.top);
			nText = _tcslen(sText);
			textPos.Set(clipWinTextBorder.cx, clipWinTextBorder.cy);

			DrawLabel(hdc, textPos, sText, nText);

			// Draw bottom right coordinates (bottom right corner)
			_stprintf_s(sText, GYAZO_POINT_FORMAT, winRect.right, winRect.bottom);
			nText = _tcslen(sText);
			GetTextExtentPoint(hdc, sText, nText, textSize);
			textPos.Set(clipRect.cx - textSize.cx - clipWinTextBorder.cx, 
				clipRect.cy - textSize.cy - clipWinTextBorder.cy);

			DrawLabel(hdc, textPos, sText, nText);

			// Draw crop size (center)
			_stprintf_s(sText, GYAZO_POINT_FORMAT, clipRect.cx, clipRect.cy);
			nText = _tcslen(sText);
			GetTextExtentPoint(hdc, sText, nText, textSize);
			textPos.Set((clipRect.cx - textSize.cx) / 2, 
				(clipRect.cy - textSize.cy) / 2);

			DrawLabel(hdc, textPos, sText, nText);

			// Release resources
			DeleteObject(hPen);
			DeleteObject(hBrush);
			Gyazo::Font::Release();
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
			Rectangle(hdc, 0, 0, cursorTextSize.cx + 4, cursorTextSize.cy);

			// The output size of the rectangle
			int fontHeight = MulDiv(8, GetDeviceCaps(hdc, LOGPIXELSY), 72);
			SelectObject(hdc, Gyazo::Font::GetFont(fontHeight));

			SetBkMode(hdc, TRANSPARENT);

			Gyazo::Size textPos;
			TCHAR sText[100];

			// Draw mouse coordinates
			_stprintf_s(sText, GYAZO_POINT_FORMAT, cursorPos.cx, cursorPos.cy);
			size_t nText = _tcslen(sText);
			GetTextExtentPoint(hdc, sText, nText, cursorTextSize);
			textPos.Set(2, 0);

			DrawLabel(hdc, textPos, sText, nText);

			// Release resources
			DeleteObject(hPen);
			DeleteObject(hBrush);
			Gyazo::Font::Release();
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
			cursorPos.Set(
				LOWORD(lParam) + screenOffset.cx, 
				HIWORD(lParam) + screenOffset.cy);

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
				cursorWinRect.Set(
					cursorPos.cx + cursorWinOffset.cx, 
					cursorPos.cy + cursorWinOffset.cy, 
					cursorTextSize.cx + 4, 
					cursorTextSize.cy);

				DrawCoordinates(hdc, cursorWinRect);
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
			clipRect.left	= LOWORD(lParam) + screenOffset.cx;
			clipRect.top	= HIWORD(lParam) + screenOffset.cy;

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
			clipRect.right	= LOWORD(lParam) + screenOffset.cx;
			clipRect.bottom	= HIWORD(lParam) + screenOffset.cy;

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
			Gyazo::Size imageSize(
				clipRect.right - clipRect.left + 1, 
				clipRect.bottom - clipRect.top + 1);

			if (imageSize.cx == 0 || imageSize.cy == 0)
			{
				// It is not already in the image , it is not nothing
				ReleaseDC(NULL, hdc);
				DestroyWindow(hWnd);
				break;
			}

			// Create a bitmap buffer
			HBITMAP	newBMP	= CreateCompatibleBitmap(hdc, imageSize.cx, imageSize.cy);
			HDC		newDC	= CreateCompatibleDC(hdc);

			// Associated
			SelectObject(newDC, newBMP);

			// Portraitをobtain
			BitBlt(newDC, 0, 0, imageSize.cx, imageSize.cy, 
				hdc, clipRect.left, clipRect.top, SRCCOPY);

			// I hide the window !
			ShowWindow(hWnd, SW_HIDE);

			// The determination of the temporary file name
			TCHAR tmpDir[MAX_PATH], tmpFile[MAX_PATH];
			GetTempPath(MAX_PATH, tmpDir);
			GetTempFileName(tmpDir, GYAZO_PREFIX, 0, tmpFile);

			if (SavePng(tmpFile, newBMP))
			{
				UploadFile(tmpFile);
			}
			else
			{
				// PNG save failed
				ErrorMessage(ERROR_CONVERT_IMAGE);
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

