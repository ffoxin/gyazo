/*
* gyazo.cpp
* 
* Entry point of gyazowin application
* 
*/

// System headers
#include <windows.h>
#include <gdiplus.h>
using namespace Gdiplus;
#include <ShlObj.h>
#include <Shlwapi.h>
#include <tchar.h>
#include <WinInet.h>

// STL headers
#include <fstream>
#include <string>
#include <sstream>

// Project headers
#include "gdiinit.h"
#include "resource.h"
#include "util.h"
using namespace Gyazo;

// Globals
HINSTANCE hInstance;						// Application instance
LPCTSTR szTitle			= _T("Gyazo");		// Text in the title bar
LPCTSTR szWindowClass	= _T("GYAZOWIN");	// Main window class name
LPCTSTR szWindowClassL	= _T("GYAZOWINL");	// Layer window class name
HWND hLayerWnd;

int screenOffsetX, screenOffsetY;	// virtual screen offset

// Declarations
ATOM				RegisterGyazoClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	LayerWndProc(HWND, UINT, WPARAM, LPARAM);

int					GetEncoderClsid(LPCWSTR format, CLSID* pClsid);

bool				IsPng(LPCTSTR fileName);
void				DrawRubberband(HDC hdc, LPRECT newRect, bool erase);
void				ExecUrl(LPCTSTR url);
void				SetClipBoardText(LPCTSTR str);
bool				ImageToPng(Image* image, LPCTSTR fileName);
bool				ConvertPng(LPCTSTR destFile, LPCTSTR srcFile);
bool				SavePng(LPCTSTR fileName, HBITMAP hBmp);
bool				UploadFile(HWND hwnd, LPCTSTR fileName);
tstring				GetId();
bool				SaveId(LPCTSTR str);
int					ErrorMessage(HWND hwnd, LPCTSTR lpText);

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
			UploadFile(NULL, fileArg);
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
				UploadFile(NULL, tmpFile);
			}
			else
			{
				// Can not be converted into PNG
				ErrorMessage(NULL, _T("Cannot convert this image"));
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

// Look at the header whether PNG image check (once)
bool IsPng(LPCTSTR fileName)
{
	unsigned char pngHead[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
	unsigned char readHead[8];

	FILE *fp = NULL;

	if (_tfopen_s(&fp, fileName, _T("rb")) != 0
		|| fread(readHead, 1, 8, fp) != 8)
	{
		// Can not read the file
		return false;
	}
	fclose(fp);

	// compare
	if (memcmp(pngHead, readHead, sizeof(pngHead)) != 0)
	{
		return false;
	}

	return true;
}

// To register a window class
ATOM RegisterGyazoClass(HINSTANCE hInstance)
{
	WNDCLASS wc;

	// Main window
	wc.style			= 0;							// Do not send a WM_PAINT
	wc.lpfnWndProc		= WndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor			= LoadCursor(NULL, IDC_CROSS);	// + Cursor
	wc.hbrBackground	= 0;							// Background
	wc.lpszMenuName		= 0;
	wc.lpszClassName	= szWindowClass;

	RegisterClass(&wc);

	// Layer window
	wc.style			= CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc		= LayerWndProc;
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= hInstance;
	wc.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor			= LoadCursor(NULL, IDC_CROSS);	// + Cursor
	wc.hbrBackground	= (HBRUSH) GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName		= 0;
	wc.lpszClassName	= szWindowClassL;

	return RegisterClass(&wc);
}

// (I covered with a window full screen) initialization of instance
BOOL InitInstance(HINSTANCE hInst, int nCmdShow)
{
	HWND hWnd;
	hInstance = hInst; // I will store instances processing in a global variable .

	// It covers the entire virtual screen
	int screenOffsetX = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int screenOffsetY = GetSystemMetrics(SM_YVIRTUALSCREEN);
	int screenWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	// I remember the offset value of x, y
	screenOffsetX = screenOffsetX;
	screenOffsetY = screenOffsetY;

	// Completely ni shi taウィthrough clothウをmake ru nn
	hWnd = CreateWindowEx(
		WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST
#if (_WIN32_WINNT >= 0x0500)
		| WS_EX_NOACTIVATE
#endif
		,
		szWindowClass, NULL, WS_POPUP,
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
	hLayerWnd = CreateWindowEx(
		WS_EX_TOOLWINDOW
#if (_WIN32_WINNT >= 0x0500)
		| WS_EX_LAYERED | WS_EX_NOACTIVATE
#endif
		,
		szWindowClassL, NULL, WS_POPUP,
		100, 100, 300, 300,
		hWnd, NULL, hInst, NULL);

	SetLayeredWindowAttributes(hLayerWnd, RGB(255, 0, 0), 100, LWA_COLORKEY|LWA_ALPHA);

	return TRUE;
}

// I get the CLSID of the Encoder corresponding to the specified format
// Cited from MSDN Library: Retrieving the Class Identifier for an Encoder
int GetEncoderClsid(LPCWSTR format, CLSID* pClsid)
{
	unsigned num = 0;		// number of image encoders
	unsigned size = 0;		// size of the image encoder array in bytes

	GetImageEncodersSize(&num, &size);
	if (size == 0)
	{
		return -1;
	}

	//ImageCodecInfo* pImageCodecInfo = new ImageCodecInfo[num];
	ImageCodecInfo* pImageCodecInfo = reinterpret_cast<ImageCodecInfo*>(new char[size]);

	GetImageEncoders(num, size, pImageCodecInfo);

	for (unsigned i = 0; i < num; ++i)
	{
		if (wcscmp(pImageCodecInfo[i].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[i].Clsid;
			delete pImageCodecInfo;
			return i;
		}
	}

	delete[] pImageCodecInfo;
	return -1;
}

// Drawing rubber band
void DrawRubberband(HDC hdc, LPRECT newRect, bool erase)
{
	static bool firstDraw = true;	// First does not perform to erase the previous band
	static RECT clipRect;			// Band that was drawn last

	if (firstDraw)
	{
		// View layer window
		ShowWindow(hLayerWnd, SW_SHOW);
		UpdateWindow(hLayerWnd);

		firstDraw = false;
	}

	if (erase)
	{
		// I hide the layer window
		ShowWindow(hLayerWnd, SW_HIDE);
	}

	// Check coordinate
	clipRect = *newRect;
	if (clipRect.right < clipRect.left)
	{
		swap(clipRect.right, clipRect.left);
	}
	if (clipRect.bottom < clipRect.top)
	{
		swap(clipRect.bottom, clipRect.top);
	}

	MoveWindow(hLayerWnd, 
		clipRect.left, 
		clipRect.top, 
		clipRect.right - clipRect.left + 1, 
		clipRect.bottom - clipRect.top + 1, 
		TRUE);
}

// Save Image class data to file
bool ImageToPng(Image* image, LPCTSTR fileName)
{
	bool result = false;

	if (image->GetLastStatus() == 0)
	{
		CLSID clsidEncoder;
		if (GetEncoderClsid(L"image/png", &clsidEncoder) != -1)
		{
			if (image->Save(fileName, &clsidEncoder, 0) == 0)
			{
				result = true;
			}
		}
	}

	return result;
}

// Convert to PNG format
bool ConvertPng(LPCTSTR destFile, LPCTSTR srcFile)
{
	// Initialization GDI+
	const GdiPlusInit& gpi = GdiPlusInit();

	Image* image = new Image(srcFile, 0);

	bool result = ImageToPng(image, destFile);

	// Clean up
	delete image;

	return result;
}

// PNG formatでsave (GDI + Use)
bool SavePng(LPCTSTR fileName, HBITMAP hBmp)
{
	// Initialization GDI+
	const GdiPlusInit& gpi = GdiPlusInit();

	Bitmap* bitmap = new Bitmap(hBmp, NULL);

	bool result = ImageToPng(bitmap, fileName);

	// Clean up
	delete bitmap;

	return result;
}

// Layer window procedure
LRESULT CALLBACK LayerWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_ERASEBKGND:
		{
			RECT winRect;
			GetWindowRect(hWnd, &winRect);
			SIZE clipRect = { winRect.right - winRect.left, winRect.bottom - winRect.top };

			HDC hdc = GetDC(hWnd);
			HBRUSH hBrush = CreateSolidBrush(RGB(100, 100, 100));
			SelectObject(hdc, hBrush);
			HPEN hPen = CreatePen(PS_DASH, 1, RGB(255, 255, 255));
			SelectObject(hdc, hPen);
			Rectangle(hdc, 0, 0, clipRect.cx, clipRect.cy);

			TEXTMETRIC tm;
			GetTextMetrics(hdc, &tm);

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

			SIZE textSize, textPos;
			TCHAR sText[100];
			size_t nText;
			const int border = 5;

			// Draw top left coordinates (top left corner)
			_stprintf_s(sText, _T("%d:%d"), winRect.left, winRect.top);
			nText = _tcslen(sText);
			textPos.cx = border;
			textPos.cy = border;

			SetTextColor(hdc, RGB(0, 0, 0));
			TextOut(hdc, textPos.cx + 1, textPos.cy + 1, sText, nText);
			SetTextColor(hdc, RGB(255, 255, 255));
			TextOut(hdc, textPos.cx, textPos.cy, sText, nText);

			// Draw bottom right coordinates (bottom right corner)
			_stprintf_s(sText, _T("%d:%d"), winRect.right, winRect.bottom);
			nText = _tcslen(sText);
			GetTextExtentPoint(hdc, sText, nText, &textSize);
			textPos.cx = clipRect.cx - textSize.cx - border;
			textPos.cy = clipRect.cy - textSize.cy - border;

			SetTextColor(hdc, RGB(0, 0, 0));
			TextOut(hdc, textPos.cx + 1, textPos.cy + 1, sText, nText);
			SetTextColor(hdc, RGB(255, 255, 255));
			TextOut(hdc, textPos.cx, textPos.cy, sText, nText);

			// Draw crop size (center)
			_stprintf_s(sText, _T("%d:%d"), clipRect.cx, clipRect.cy);
			nText = _tcslen(sText);
			GetTextExtentPoint(hdc, sText, nText, &textSize);
			textPos.cx = (clipRect.cx - textSize.cx) / 2;
			textPos.cy = (clipRect.cy - textSize.cy) / 2;

			SetTextColor(hdc, RGB(0, 0, 0));
			TextOut(hdc, textPos.cx + 1, textPos.cy + 1, sText, nText);
			SetTextColor(hdc, RGB(255, 255, 255));
			TextOut(hdc, textPos.cx, textPos.cy, sText, nText);

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
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;

	static bool isClip		= false;
	static RECT clipRect	= {};

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
		if (isClip)
		{
			// Set the new coordinate
			clipRect.right	= LOWORD(lParam) + screenOffsetX;
			clipRect.bottom	= HIWORD(lParam) + screenOffsetY;

			hdc = GetDC(NULL);
			DrawRubberband(hdc, &clipRect, false);

			ReleaseDC(NULL, hdc);
		}
		break;


	case WM_LBUTTONDOWN:
		// Clip start
		isClip = true;

		// Set the initial position
		clipRect.left	= LOWORD(lParam) + screenOffsetX;
		clipRect.top	= HIWORD(lParam) + screenOffsetY;

		// Capture the mouse
		SetCapture(hWnd);
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
			DrawRubberband(hdc, &clipRect, true);

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
				UploadFile(hWnd, tmpFile);
			}
			else
			{
				// PNG save failed
				ErrorMessage(hWnd, _T("Cannot save png image"));
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

// Copy text to clipboard
void SetClipBoardText(LPCTSTR str)
{
	HGLOBAL	hText;
	LPTSTR	pText;
	size_t	slen;

	slen = _tcslen(str) + 1; // NULL

	hText = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, slen * sizeof(TCHAR));

	pText = (LPTSTR) GlobalLock(hText);
	_tcsncpy_s(pText, slen, str, slen);
	GlobalUnlock(hText);

	// I open the clipboard
	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(
#ifdef UNICODE
		CF_UNICODETEXT
#else
		CF_TEXT
#endif
		, 
		hText);
	CloseClipboard();

	// Liberation
	GlobalFree(hText);
}

// I open a browser (char *) URL that is specified
void ExecUrl(LPCTSTR url)
{
	// Run the open command
	SHELLEXECUTEINFO lsw = {};
	lsw.cbSize = sizeof(SHELLEXECUTEINFO);
	lsw.lpVerb = _T("open");
	lsw.lpFile = url;

	ShellExecuteEx(&lsw);
}

template<size_t SzDir, size_t SzFile>
void GetIdPaths(TCHAR (& idDir)[SzDir], TCHAR (& idFile)[SzFile])
{
	SHGetSpecialFolderPath(NULL, idDir, CSIDL_APPDATA, FALSE);

	_tcscat_s(idDir, _T("\\Gyazo"));
	_tcscpy_s(idFile, idDir);
	_tcscat_s(idFile, _T("\\id.txt"));
}

// I generate load the ID
tstring GetId()
{
	TCHAR idFile[MAX_PATH], idDir[MAX_PATH];

	GetIdPaths(idDir, idFile);

	LPCTSTR idOldFile = _T("id.txt");

	// load ID from the file
	tifstream ifs;
	tstring idStr;

	ifs.open(idFile);
	if (!ifs.fail())
	{
		// read the ID
		ifs >> idStr;
		ifs.close();
	}
	else
	{
		ifs.close();
		ifs.open(idOldFile);
		if (!ifs.fail())
		{
			// (Compatibility with older versions) to read the ID from the same directory
			ifs >> idStr;
			ifs.close();
		}
	}

	return idStr;
}

// Save ID
bool SaveId(LPCTSTR str)
{
	TCHAR idFile[MAX_PATH], idDir[MAX_PATH];

	GetIdPaths(idDir, idFile);

	// I want to save the ID
	CreateDirectory(idDir, NULL);
	tofstream ofs;
	ofs.open(idFile);
	if (!ofs.fail())
	{
		ofs << str;
		ofs.close();

		// Delete the old configuration file
		LPCTSTR idOldFile = _T("id.txt");
		if (PathFileExists(idOldFile))
		{
			DeleteFile(idOldFile);
		}
	}
	else
	{
		return true;
	}

	return true;
}

// I want to upload a PNG file .
bool UploadFile(HWND hwnd, LPCTSTR fileName)
{
	LPCTSTR UPLOAD_SERVER	= _T("gyazo.com");
	LPCTSTR UPLOAD_PATH		= _T("/upload.cgi");
	LPCTSTR	SHARE_PATH		= _T("http://ffoxin.github.io/gyazo.htm?id=");

	const char*	sBoundary = "----BOUNDARYBOUNDARY----";		// boundary
	const char	sCrLf[] = { 0xd, 0xa, 0x0 };				// Diverted (CR + LF)
	LPCTSTR szHeader = 
		_T("Content-type: multipart/form-data; boundary=----BOUNDARYBOUNDARY----");

	std::ostringstream buf;	// Outgoing messages

	// Get an ID
	tstring tidStr = GetId();
	std::string idStr(tidStr.begin(), tidStr.end());

	// Configuring Message
	// -- "id" part
	buf << "--";
	buf << sBoundary;
	buf << sCrLf;
	buf << "content-disposition: form-data; name=\"id\"";
	buf << sCrLf;
	buf << sCrLf;
	buf << idStr;
	buf << sCrLf;

	// - " ImageData " part
	buf << "--";
	buf << sBoundary;
	buf << sCrLf;
	buf << "content-disposition: form-data; name=\"imagedata\"; filename=\"gyazo.com\"";
	buf << sCrLf;
	//buf << "Content-type: image/png";	// 一応
	//buf << sCrLf;
	buf << sCrLf;

	// Read a PNG file
	std::ifstream png;
	png.open(fileName, std::ios::binary);
	if (png.fail())
	{
		png.close();
		ErrorMessage(hwnd, _T("PNG open failed"));
		return false;
	}
	buf << png.rdbuf();		// read all & append to buffer
	png.close();

	// Last
	buf << sCrLf;
	buf << "--";
	buf << sBoundary;
	buf << "--";
	buf << sCrLf;

	// Message completion
	std::string msg(buf.str());

	// WinInet preparation (proxy required Full setをはuse)
	HINTERNET hSession = InternetOpen(
		szTitle, 
		INTERNET_OPEN_TYPE_PRECONFIG, 
		NULL, 
		NULL, 
		0);
	if (hSession == NULL)
	{
		ErrorMessage(hwnd, _T("Cannot configure wininet"));
		return false;
	}

	// Access point
	HINTERNET hConnection = InternetConnect(hSession, 
		UPLOAD_SERVER, INTERNET_DEFAULT_HTTP_PORT,
		NULL, NULL, INTERNET_SERVICE_HTTP, 0, NULL);
	if (hSession == NULL)
	{
		ErrorMessage(hwnd, _T("Cannot initiate connection"));
		return FALSE;
	}

	// Full set requirements before
	HINTERNET hRequest = HttpOpenRequest(hConnection,
		_T("POST"), UPLOAD_PATH, NULL,
		NULL, NULL, INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_RELOAD, NULL);
	if (hRequest == NULL)
	{
		ErrorMessage(hwnd, _T("Cannot compose post request"));
		return false;
	}

	// User Agentを 指定
	LPCTSTR userAgent = _T("User-Agent: Gyazowin/1.0\r\n");
	BOOL bResult = HttpAddRequestHeaders(
		hRequest, userAgent, _tcslen(userAgent), 
		HTTP_ADDREQ_FLAG_ADD | HTTP_ADDREQ_FLAG_REPLACE);
	if (bResult == FALSE)
	{
		ErrorMessage(hwnd, _T("Cannot set user agent"));
		return false;
	}

	// Requirementsをmessenger
	bResult = HttpSendRequest(hRequest,
		szHeader,
		lstrlen(szHeader),
		(LPVOID)msg.c_str(),
		(DWORD) msg.length());
	if (bResult == TRUE)
	{
		// Success requiresは

		DWORD resLen = 8;
		TCHAR resCode[8];

		// state codeを 取得
		HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE, resCode, &resLen, 0);
		if (_ttoi(resCode) != 200)
		{
			// upload 失敗 (status error)
			ErrorMessage(hwnd, _T("Failed to upload (unexpected result code, under maintainance?)"));
		}
		else
		{
			// Upload succeeded

			// Get new id
			DWORD idLen = 100;
			TCHAR idUrl[100];

			memset(idUrl, 0, idLen * sizeof(TCHAR));	
			_tcscpy_s(idUrl, _T("X-Gyazo-Id"));

			HttpQueryInfo(hRequest, HTTP_QUERY_CUSTOM, idUrl, &idLen, 0);
			if (GetLastError() != ERROR_HTTP_HEADER_NOT_FOUND 
				&& idLen != 0)
			{
				// Save new id
				SaveId(idUrl);
			}

			// Read URL results
			DWORD len;
			char resbuf[1024];
			std::string result;

			// Never so long , but once well
			while (InternetReadFile(hRequest, (LPVOID) resbuf, 1024, &len) 
				&& len != 0)
			{
				result.append(resbuf, len);
			}

			std::string id = result.substr(result.find_last_of("/") + 1);

			tostringstream urlStream;
			urlStream << SHARE_PATH << id.c_str();

			const tstring sUrl = urlStream.str();
			LPCTSTR tUrl = sUrl.c_str();

			// Copy the URL to the clipboard
			SetClipBoardText(tUrl);

			// Launch an URL
			ExecUrl(tUrl); 

			return true;
		}
	}
	else
	{
		// Upload failed
		ErrorMessage(hwnd, _T("Failed to upload"));
		return false;
	}

	return false;
}

int ErrorMessage(HWND hwnd, LPCTSTR lpText)
{
	return MessageBox(hwnd, lpText,	szTitle, MB_ICONERROR | MB_OK);
}
